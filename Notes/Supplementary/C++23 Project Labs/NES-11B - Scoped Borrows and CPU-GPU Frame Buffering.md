### NES-11B — Borrow the frame, not the right to overwrite it

The CPU in our growing NES project can execute a known subset of 6502 instructions. The debugger can return an owned register snapshot, and the asset registry can turn a validated TXR4 source image into an owned typed texture with a generational handle. The work graph can order simulated host and GPU stages, and the memory reference model can distinguish initialization from visibility and lifetime from completion. None of those earlier guarantees, by itself, answers the question that an interactive renderer faces every frame: **may the CPU overwrite the bytes prepared for frame 0 while a device may still be reading them, or while an editor panel still holds a view into them?**

This is where C++ const qualification, borrowing, RAII, frame buffering, resource generations and actual completion evidence become one problem. They are not disparate language trivia. The chapter already contains detailed derivations of nested const safety and the intersection of borrow intervals, then a large explanation of GPU-generated work, frame rings, command buffers, Vulkan/Metal synchronization, presentation and queue retirement. We are not replacing that narrative. We attach an executable **NES-11B CPU reference model** at the boundary before “From generated GPU work to safe explicit execution,” making the qualification and lifetime proofs tangible before the retained mature GPU material continues.

Two new sources form the working laboratory. `tools/labs/nes_course/scoped_frame_upload_ring.hpp` defines a bounded, generational, move-only frame upload ring with ownership-preserving read and write leases. `tools/labs/nes_scoped_frame_upload_cpp23_lab.cpp` combines that ring with the actual pre-existing NROM loader, typed texture importer, generational registry and separately compiled CPU debugger, then verifies a synthetic three-frames-in-flight trace. The CMake target for the test compiles the same `cpu_debug_session.cpp` implementation used earlier. These are real C++23 host-side source files; they do **not** create a Vulkan or Metal device, submit a command buffer or read a real GPU completion event.

#### A const wrapper can still name mutable bytes

Begin with a straightforward statement: a C++ object can be const while an object reachable *through* it remains mutable. Consider a span that stores a pointer and a length. Making the span wrapper itself const prevents changing its pointer or length through that wrapper, but does not automatically change the element type from mutable to const.

```cpp
std::array<std::uint8_t, 4> payload{1, 2, 3, 4};

std::span<std::uint8_t> editable{payload};
const std::span<std::uint8_t> fixed_view{payload};
std::span<const std::uint8_t> read_only_elements{payload};

// Both paths below can modify the underlying element.
editable[0] = 10;
fixed_view[1] = 20;

// read_only_elements[2] = 30; // ill-formed: element is const.
```

The distinction is about **which level of the type** is qualified. `const std::span<Byte>` gives you a const view object whose element type is still `Byte`. `std::span<const Byte>` is a view whose element type does not permit modification through that path. The distinction is more than stylistic in an upload pipeline: a supposedly read-only observer must not silently retain a mutable element reference through which it can alter bytes that a queued GPU operation expects to remain stable.

Our laboratory contains compile-time assertions of these exact assignability rules. It also checks the existing chapter's nested pointer example: converting `int**` to `const int**` is not a permitted qualification conversion, because it could inject a pointer to a const object into a mutable `int*` slot. A safer qualification decomposition such as `const int* const*` prevents changing that intermediate pointer through the newly qualified access path.

A crucial limit remains: even `span<const Byte>` is a **read-only access path**, not a guarantee that no other alias can mutate the underlying bytes. Another thread holding a mutable alias could still attempt to change them, causing a data race if not synchronized. An upload ring therefore needs an *access protocol* that prevents mutation while the read-only view exists. Constness is part of the interface; temporal exclusion is another part of correctness.

#### Borrowing means an interval, not ownership of the object

The original chapter expresses the validity of a borrow as an intersection of the referent's live interval, its storage relationship and the period during which it still denotes the intended identity. That is the correct mental model for `std::span`, `std::string_view`, iterators, `std::reference_wrapper` and raw references. None of those types automatically keeps the referent alive.

Let $L_o$ be the set of instants in which the object is alive, $L_s$ the instants in which its storage remains appropriate for the accessor, $L_i$ the instants in which it still represents the intended logical identity, and $L_a$ the instants in which access is permitted by the synchronization/ownership protocol. For an ordinary borrowed view, the admissible access interval is constrained by

$$
L_{\text{access}}\subseteq L_o\cap L_s\cap L_i\cap L_a.
$$

The intersection is necessary but not sufficient: type qualification, bounds, strict-aliasing rules and any required memory visibility also apply. But the formula exposes why a valid pointer is not a lifetime proof. A non-null pointer may point to freed storage; the same allocation address may be occupied by a new generation; a vector reallocation may invalidate its old elements; or the object may be live while another participant has exclusive authority to mutate it.

Our Pass 06 asset registry illustrates identity without pinning. Its handle combines an owner ID, slot and generation; a lookup checks those fields before returning a reference to an asset. But that reference does not remain valid through arbitrary registry mutations or after the registry is destroyed. The caller must either bound the lookup's use to a safe interval or deliberately copy the needed payload into an independent owned value.

The new Pass 11 ring makes the borrow interval explicit by returning a move-only **ReadLease** rather than a naked pointer from the ring. The lease holds two things: shared ownership of the ring's heap-allocated control block, and a read pin on one specific slot. The shared ownership keeps the byte-storage owner alive even if the outer FrameUploadRing wrapper is moved or destroyed. The slot pin prevents that slot from being retired and recycled while the read lease exists.

The view itself is still a `std::span<const Byte>`. If the programmer copies that span into a global variable and destroys the lease, C++ cannot automatically revoke the copied span or prevent its future dereference. This is an explicit limitation. The correct use is to consume the view while the ReadLease lives, or invoke `copy_snapshot()` to create a separately owned vector whose lifetime is independent of the ring. A wrapper that appears to “solve all dangling references” merely because it uses `shared_ptr` would be overstating its guarantee.

#### Why lifetime ownership and identity must coexist

The ring contains three distinct identity concepts. The pool's **owner ID** identifies which ring issued a frame handle. The **slot index** selects one of a small fixed number of reusable storage locations. The **generation** distinguishes successive occupants of that slot. A handle is therefore the tuple

$$
H=(O,S,G).
$$

A lookup must verify all three components. If an old `H=(5,0,8)` is retained when slot zero is recycled into `(5,0,9)`, the old handle must fail even though the physical slot and likely the vector's capacity are unchanged. A handle from a different ring with the same slot index and generation must likewise fail because its owner namespace differs.

The pool owner is minted through a process-local atomic counter, as in our earlier registry experiment. That makes ordinary identity creation thread-safe at the counter level but does **not** make subsequent ring operations thread-safe. The ring's slot transitions, reader pins and payload vectors are deliberately single-threaded for this lab. Any real cross-thread use must establish locking or a proven atomic publication protocol; adding one atomic ID generator does not protect non-atomic slot fields.

The test forces the generation to its maximum 64-bit value using an explicit teaching-only constructor parameter. On retirement, that slot becomes permanently exhausted rather than incrementing and wrapping to zero or one. This avoids an ABA problem in which an ancient handle could match a later recycled slot after an arithmetic wrap. A production engine may choose a different quarantine or larger identity strategy, but it must provide a real no-alias policy appropriate to its intended lifetime.

FrameHandle is intentionally not a pointer to the payload. It is a validated logical name whose meaning depends on the ring's current state. This protects identity. The ReadLease then supplies a bounded access authority and keeps the referent's storage alive. Both mechanisms are necessary: a valid logical name is not automatically a safe live reference, and a live pointer is not automatically a reference to the correct logical frame.

#### Every frame slot has a finite state machine

A frame ring can be implemented as one growing byte buffer with offsets, multiple fixed buffers, or a transient arena allocator. For this teaching project we choose three fixed **logical slots**, each containing an owned host vector, a generation, a state and a completion requirement. The state machine is

```text
                    writer acquired
FREE --------------------------------> WRITING
  ^                                      |
  |                                      | cancel or abandoned writer
  |______________________________________|
  |
  |  all required queues completed
  |  AND active reader count == 0
  |
IN_FLIGHT <--------------------------- WRITING
                   successful submit

IN_FLIGHT --retire at max generation--> EXHAUSTED (never reused)
```

The diagram is intentionally simple; a C++ implementation must check errors before every state transition. `begin()` searches for a FREE slot and changes it to WRITING. The returned WriteLease is move-only: there cannot be two independent C++ owners each believing they have exclusive authority over the same draft. The lease's destructor cancels an unfinished write and restores the slot to FREE. That is a real RAII rollback contract; it does not depend on the caller remembering to invoke `cancel` along every exception or early-return path.

A WriteLease's `replace(bytes)` method receives a read-only view of source bytes. It first checks that the source is nonempty and within the configured byte budget. It then constructs an owned replacement vector **before** swapping that vector into the current draft. If allocation fails during construction, the original draft remains present and the failure may propagate as a C++ allocation exception. If the source is too large, a typed error is returned without modifying the draft. A successful replacement copies the data, so later changes to the source container do not retroactively edit the staged frame.

This is a concrete instance of the **prepare, then commit** transaction pattern we used in earlier chapters. The frame writer's draft is a private candidate; the submission step is the publication boundary. A half-prepared draft is never exposed as a submitted frame. If a writer is moved, its new C++ owner receives that candidate and the old writer becomes inert. When the moved-from writer is destroyed, it must not cancel the candidate that the destination now owns.

The `submit(required_stamp)` operation checks that a draft actually contains bytes and that its declared nonzero queue tickets are not already completed. It records the completion requirements and moves the slot to IN_FLIGHT, returning a generation-aware FrameHandle. After successful submission, the writer relinquishes its authority and must not modify the now-frozen payload. Our model does not itself submit any device work; the completion stamp is an **intended external requirement**, not a proven native event.

If every slot is IN_FLIGHT, `begin()` returns `no_free_slot`. It does **not** wrap its index modulo three and overwrite slot zero merely because the fourth frame begins. That is the bug we want the learner to recognize. A frame index is a bookkeeping choice; it is not evidence that the previous user's command buffer has completed.

#### Three frames in flight are not three swapchain images

The original chapter already explains a distinction that causes real graphics bugs. A **frame slot** groups CPU/GPU temporary resources with a particular submission lifetime. A **swapchain image index** identifies the presentation image acquired from the window system. Those indexes need not be equal; even when the swapchain has three images and the ring has three slots, the acquisition order can differ from the modulo frame sequence.

For example, frames might use slots $0,1,2,0$ while the presentation API returns image indexes $2,0,1,2$. The same numerical index, when it occurs, is a coincidence between two distinct namespaces. A command pool, per-frame uniform suballocation, host upload region and some reusable descriptors may belong to the frame slot. A swapchain image view and image-indexed presentation synchronization belong to the acquired image. Binding the wrong image because the frame slot happened to be zero can produce invalid presentation behavior even if the CPU ring's generation tracking is perfect.

Khronos's current [frames-in-flight tutorial](https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/03_Frames_in_flight.html) and [swapchain semaphore reuse guide](https://docs.vulkan.org/guide/latest/swapchain_semaphore_reuse.html) emphasize that frame and acquired-image lifetimes have different synchronization concerns. In a Vulkan implementation, the rendering command completion for one frame does not by itself establish that a present-wait semaphore can be reused independently of presentation. The exact safe pattern depends on the available swapchain synchronization mechanisms and extensions. Our model deliberately does not represent presentation semaphores, so its tests are **not** a swapchain certification.

A third identity is the output's **content version**: the camera and CPU snapshot used to render one frame can differ from the current scene by the time a readback completes. A delayed screenshot or screen-space picking result should carry enough frame/camera/source-generation provenance to interpret it correctly. A live memory address tells us none of that. The frame slot ID keeps storage safe; the content stamp tells us which simulated world state the bytes describe.

#### A single completion number is not a proof about three queues

For one strictly ordered GPU submission stream with an appropriate timeline event, a single monotonically increasing completion point can often control retirement of the work carried by that stream. But an engine may use independent transfer, compute and graphics queues, or otherwise need to track several completion domains. A scalar frame count cannot be compared across independent domains as if it were a universal GPU timestamp.

Our FrameUploadRing stores an explicit vector of completion requirements

$$
R=(r_t,r_c,r_g)
$$

for transfer, compute and graphics, and a vector of **observed** completed values

$$
D=(d_t,d_c,d_g).
$$

The required work is complete only if

$$
D\succeq R
\quad\Longleftrightarrow\quad
d_t\ge r_t\;\land\;d_c\ge r_c\;\land\;d_g\ge r_g.
$$

The comparison is componentwise, not lexicographic. The largest individual number has no special meaning across queues. For a frame requiring $R=(1,2,4)$, observed completion $D=(3,1,4)$ is **not sufficient** because compute has reached only one, even though transfer has reached three and graphics four. Another frame requiring $(2,0,5)$ remains pending if graphics has reached four. A third requiring $(3,6,0)$ remains pending while compute has reached one. The test constructs exactly these three cases.

The zero ticket means that the modeled submission makes no completion claim on that particular queue. The test then advances observations to $D=(3,6,5)$, satisfying all three required vectors. The frame slots are **potentially** retireable at that point, but only after separate CPU read pins are released. A completion observation for a queue must be monotonically nondecreasing; the ring rejects regression and preserves its previous observed stamp. If a real backend reports an impossible regression, overwriting the old value would make the bookkeeping less trustworthy.

This is **CPU-side retirement bookkeeping**. The lab does not verify that a native queue actually signaled the reported values. The caller supplies them in a deterministic fixture. A production backend must obtain each value from the proper native completion primitive and prove that every physical use of the affected allocation is covered. Cross-queue dependencies, ownership transfers and memory visibility are not established merely by storing three integers.

For Vulkan, queue submission and timeline-semaphore semantics must be mapped to the actual device and resource uses. For Metal, command-buffer completion, events and the applicable queue-generation rules determine when operations and resource lifetimes may be retired. In classic Metal, the documentation for retained command-buffer references distinguishes automatic retention from unretained-reference paths. In Metal 4, the command model has different reference-retention and synchronization responsibilities; the current [Metal 4 core API documentation](https://developer.apple.com/documentation/metal/understanding-the-metal-4-core-api) explicitly calls out resource retention. A C++ reference model that succeeds under arbitrary caller-provided completion stamps is not a substitute for those rules.

#### GPU completion is still not permission to violate an active CPU borrow

Imagine the device has finished reading a particular frame's upload bytes, but an editor-side CPU diagnostic tool is still displaying a view of those bytes. The GPU usage has ended, yet the CPU observer's borrowed span would become dangling if the ring destroyed or repurposed that slot immediately. The ring must therefore require *both* device completion and no CPU reader pins.

Our `retire(handle)` follows that logic. It validates owner, slot, generation and submitted state. It then checks that all declared queue completion stamps have been observed. Finally it checks that the slot's active read count is zero. If any condition fails, it returns a typed error without changing the slot's payload or generation. Only after every condition succeeds does it clear the old payload, increment the generation and return the slot to FREE. The test keeps one ReadLease alive after all three queues are marked complete, confirms that retirement fails with `readers_pinned`, then releases the lease and verifies that retirement becomes possible.

This establishes an important order of reasoning:

```text
Logical frame exists
    |
    v
CPU writer prepares bytes privately
    |
    v
Writer seals frame; GPU obligations recorded
    |
    +--------------------+
    |                    |
    v                    v
GPU use reaches          CPU reader holds
every required stamp     an immutable read lease
    |                    |
    +--------------------+
             |
       BOTH conditions
       must be satisfied
             |
             v
Old handle retired; generation incremented
             |
             v
Slot becomes reusable for next frame
```

A real engine could decide that only the GPU and no CPU diagnostic observer is permitted to keep a view after submission. Then the CPU read-pin count may remain zero and retirement can occur at the GPU completion point. Our lab deliberately supports CPU readers to make the lifetime relation testable and to avoid implying that completion of one processor automatically ends **every other** participant's access right.

This is the first connection to the *reader/writer lock* idea, but the current model is not a lock and does not allow concurrent thread-safe access. It is a state machine with single-threaded transitions. A future multi-threaded renderer needs a synchronization strategy for the ring's control block itself. A shared_ptr makes the allocation's *reference count* safe for concurrent ownership changes, not the contained vector, phase enum, completion stamp and pin count safe for arbitrary simultaneous mutation.

#### ReadLease lives after the wrapper dies, but its span does not outlive the lease

Consider the outer FrameUploadRing wrapper being destroyed while an editor component still owns a ReadLease. In an ordinary vector-based ring whose data lived directly inside the wrapper, the editor's span would become dangling immediately. In our model the wrapper holds a `std::shared_ptr<Control>`, and each active lease holds its own copy. The final control block and its slots are destroyed only after the final owning reference disappears.

This supports a clean and practical test: create a one-slot pool inside a nested scope, submit a three-byte payload, borrow the submitted frame into a ReadLease held in the outer scope, then destroy the inner pool wrapper. The read lease's `view()` can still return the original three bytes, because the referent's storage owner remains alive. Releasing that final read lease may destroy the control block, ending the borrow interval; any previously copied raw span must not be used after that point.

Why not return only a `shared_ptr<const vector<Byte>>` and call the problem solved? That is a legitimate alternative for immutable, independently owned snapshots. But it changes the recycling policy: a fresh immutable payload might be allocated on every frame, while reusing a ring's fixed storage would be delayed until every shared snapshot is released. Such a tradeoff can be appropriate for editor readbacks or data analytics, but an upload arena with a strict memory budget may prefer per-slot pins and a clear retirement operation. The right abstraction depends on whether readers need raw stable spans, independent owned values or access to a GPU-backed range with external completion obligations.

The present design illustrates two viable ways to escape the original registry lookup's lifetime restrictions. `copy_snapshot()` returns a true owned `std::vector<Byte>`, safe from later ring changes. A ReadLease returns a borrowed `span<const Byte>` while retaining the pool's control and preventing slot recycling. Both are stronger than returning a naked reference with no lifetime guard, but they have different costs: copying moves bytes into independent storage; pinning keeps existing storage occupied and may cause backpressure.

#### Real NROM state becomes a byte-stable frame payload

The project would be much weaker if the upload ring only stored invented integer arrays. We therefore construct the example from the preceding NES runtime. First we build a valid synthetic one-bank NROM cartridge whose reset vector points to `0x8000`. Its program executes `LDA #$7F` followed by `STA $02`. We mount it through the **same owned CpuBus** developed earlier, reset the actual separately compiled `CpuDebugSession`, and execute the two instructions. The resulting owned `CpuStateSnapshot` has accumulator `0x7F`, program counter `0x8004`, modeled total cycle count five, and two pending debugger trace entries.

Separately, the Pass 05 decoder imports a synthetic 2×1 RGBA8 TXR4 image into the Pass 06 `GenerationalAssetRegistry`. A typed lookup verifies the source asset and we deliberately copy its eight pixel bytes into an owned CPU vector. We do not stash a reference returned from that mutable registry and hope its storage never moves. The actual pixel sequence is

$$
[0x12,0x34,0x56,0xFF,\;0xAB,0xCD,0xEF,0x44].
$$

The new `make_overlay` function encodes a 16-byte header and those eight payload bytes into a 24-byte record. The header contains the four ASCII signature bytes `NESF`, a frame number, the accumulator, the little-endian program counter and an explicit eight-byte little-endian cycle counter. Crucially, it does **not** perform `memcpy` on the C++ register snapshot struct. A native struct may contain padding and ABI-dependent layout, and sharing its raw object representation with another device or a file format would quietly import unwanted platform assumptions.

```cpp
// Format contract, not a native C++ object-layout promise.
bytes.push_back('N');
bytes.push_back('E');
bytes.push_back('S');
bytes.push_back('F');
bytes.push_back(frame_index);
bytes.push_back(snapshot.a);
bytes.push_back(static_cast<Byte>(snapshot.pc & 0xffu));
bytes.push_back(static_cast<Byte>((snapshot.pc >> 8u) & 0xffu));

for (unsigned i = 0; i != 8; ++i)
    bytes.push_back(static_cast<Byte>(
        (snapshot.total_cycles >> (8u * i)) & 0xffu));

// Follow with the copied RGBA8 pixels.
bytes.insert(bytes.end(), pixels.begin(), pixels.end());
```

The test prepares three such overlays, one per frame slot, and immediately mutates the temporary source vector after staging. When a pinned reader later examines slot zero, the header still begins with `NESF`, contains the CPU state and the original first and last pixel bytes. This is direct evidence for the writer's deep-copy publication boundary. It is not evidence that those bytes have been uploaded to GPU VRAM; device visibility remains a separate responsibility from Pass 09.

The test then obtains an independently owned snapshot from a ReadLease. After all queue completions and pins allow slot zero to retire, a fourth frame reuses that slot and publishes the copied snapshot under a **new generation**. The original handle is rejected as stale. The owned snapshot remains valid throughout. At the end, the original typed texture is erased from its own asset registry and its earlier handle is invalidated independently of the frame ring. We have now demonstrated four distinct identities and intervals: asset registry identity, CPU snapshot ownership, frame slot identity and future GPU execution lifetime.

#### Why a valid C++ expression may still violate the frame's contract

A program might legally write `buffer[index] = value` and still violate the renderer's lifetime or concurrency rules. C++ type-checking establishes the expression's static legality under its declarations, not that the GPU has finished reading the same allocation. Conversely, a read-only span can prevent mutation through that path but cannot prove that the device has made its latest write visible to the host.

The correct questions for an upload buffer are therefore layered. Does a valid object own the storage? Is the byte range within bounds? Is the current accessor a read-only or mutable authority? Has the relevant writer completed its transaction? Are outstanding device accesses still pending? Have the necessary memory visibility operations occurred? Does the buffer still correspond to the expected generation? Is a CPU observer pinning the storage? A failure in any one condition invalidates the operation regardless of the other conditions.

This is the meaning of the original chapter's borrow-interval intersection when applied to real graphics engineering. Type qualifiers narrow the available operations. RAII expresses scoped authority. Generation checks express logical identity. Completion stamps express the modeled device-use horizon. The actual Vulkan/Metal memory model supplies producer/consumer ordering and visibility. None of these is a replacement for another.

The model makes the failure modes explicit: `oversized_payload`, `empty_payload`, `no_free_slot`, `no_writer`, `wrong_owner`, `invalid_slot`, `stale_handle`, `not_submitted`, `completion_pending`, `readers_pinned`, `completion_regressed` and `moved_from_pool`. The result types use `std::expected` so a caller must decide how to handle a failed operation rather than treating an arbitrary return value as a valid frame ID.

#### Backpressure is a user-visible policy, not an inconvenient exception

A three-slot ring means there can be only three distinct occupied frame slots at once. If three frames are submitted and no slot is both complete and retired, the fourth preparation request returns `no_free_slot`. This is intentional backpressure. A real editor might choose to wait for the oldest frame's actual completion, skip generating a new visual frame, allocate temporary overflow capacity within a budget, or apply a different frame pacing policy. The model does not silently select one for the user.

The number of slots is a throughput/latency/memory tradeoff. A larger ring can allow the CPU to prepare work farther ahead while the GPU is busy, reducing stalls at some workloads. But every additional in-flight frame may increase temporary memory consumption and increase the delay between user input and visible response. More buffering is therefore not automatically a better experience; an interactive XR application may prefer a strict low-latency policy over maximum queued throughput.

Also distinguish **frame slot buffering** from **algorithmic ping-pong**. A simulation using state arrays A and B reads the old state and writes the new state to prevent in-place read/write hazards. The arrays may swap roles once the algorithmic phase dependency is satisfied. A frame ring instead prevents an entirely new frame from overwriting bytes used by a prior submission. A production pipeline may need *both*—two ping-pong physics buffers inside a renderer that maintains three separate frame contexts. Counting all those resources as “double versus triple buffering” without specifying their purpose is a source of design confusion.

The fixed three-frame demonstration is not an instruction to set every renderer's `MAX_FRAMES_IN_FLIGHT` to three. The appropriate count depends on device scheduling, CPU/GPU overlap, present mode, swapchain images, workload latency, memory budget and the application's real responsiveness target. Our test proves only its declared state-machine invariants for three logical slots.

#### The cost of a pin is not the cost of a copy

A pinned ReadLease is cheap to acquire in our model: it copies a shared owner token and increments the slot's reader count. It does not copy the entire overlay. But that cheap read has a hidden *capacity* cost: a pending CPU observer may prevent the frame slot from retiring even after all device work completes. If a debugger keeps a borrow indefinitely, the ring can run out of free slots. Correctness holds, but liveness and throughput suffer.

By contrast, `copy_snapshot()` allocates a separate vector and copies the bytes. It costs time and memory proportional to the payload length, but the ring can release and reuse the slot independently once the copy exists. For tiny 24-byte teaching overlays, copying is often a reasonable simplification. For megabyte-scale texture uploads or GPU readbacks, repeatedly copying may be expensive and a pinned mapped range or managed snapshot object may be preferable.

Neither choice creates a native synchronization guarantee. A real GPU upload buffer may be persistently mapped or write-combined; the host may need to flush noncoherent memory before device access, keep allocations and descriptor bindings alive, or wait for a completion event before overwriting. The exact requirements vary by backend and memory type. The CPU ring's `std::vector<Byte>` is ordinary host storage, not a device allocation, and its completion stamps are bookkeeping supplied by the test. When porting, keep the pool's access and generation contracts but replace the simulated completion input with the actual backend's observed proof.

The ring also intentionally retains vector capacity after a successful retirement; `clear()` ends the logical payload size without promising that capacity is returned to the host allocator. This is useful for reuse, but it means `available()` reports *free logical slots*, not a count of freed physical bytes. An actual memory-budget report must distinguish capacity reserved, live payload size, alignment overhead, pinned in-flight storage and allocations ready for decommit.

#### Transactional submit and cancellation have separate meanings

The writer's destructor cancels an **unsubmitted** draft. It does not cancel commands already handed to an external GPU queue; after submission, the frame exists independently under its FrameHandle. This is why the successful submit operation resets the writer's authority but leaves the frame slot marked IN_FLIGHT.

There is no automatic “undo submit” in the model, because such an operation would be unsafe in a real asynchronous renderer without proving what work had been submitted and whether cancellation was supported. A true device-lost recovery path, failed queue submission or abandoned acquired swapchain image needs its own explicit state machine. Merely destroying the C++ writer object is not evidence that the GPU stopped reading the storage.

The test verifies that replacing a draft with an oversized candidate fails while preserving the earlier valid draft. It moves the writer and confirms that the moved-from writer can no longer write. It then destroys the remaining writer without submitting and checks that exactly one slot is available again. Finally it exercises submission refusal for an empty payload. Those are negative test cases of *authority transfer* rather than repetitive happy-path calls.

A deeper C++ lesson follows: C++ move construction transfers ownership of some resources according to the class contract, but the source object remains alive and must be left in a valid state. In our ring, the source of a moved WriteLease becomes inert; the new destination becomes the only scope that can submit or cancel the draft. A copied WriteLease would be dangerous because two scopes could both believe their destructors own the rollback. We forbid copying at compile time and test moves explicitly.

#### The pool's own move cannot invalidate active readers

The outer FrameUploadRing itself is move-only. The actual payload and slot table live in a shared heap control block, so moving the outer wrapper changes which C++ object holds the owner's shared pointer without relocating the stored vectors. A move carries the same owner namespace into the destination; existing FrameHandles continue to refer to the same control. The moved-from wrapper has no control block and must return a typed `moved_from_pool` error if asked for a new writer.

This differs from moving a raw vector in a system where clients have captured pointers to the vector object itself rather than its elements. The safety of a move depends on which physical object holds the authoritative state and what each client has borrowed. C++ references to the old wrapper do not automatically “follow” the move, but the explicit FrameHandle and ReadLease point into the shared control, not the address of that wrapper.

The test exercises this distinction. It moves a ring into a new C++ wrapper, verifies the owner ID is unchanged and the moved-from wrapper refuses use, then stages and retires a frame through the destination. Another test has a ReadLease outlive an entire inner-scoped pool wrapper and still successfully read its bytes. Those cases prove the declared lifetime topology under the lab's single-threaded assumptions; they do not justify unsynchronized access from multiple OS threads.

#### How this maps to a renderer without overclaiming correctness

To map the host model onto a real Vulkan backend, assign per-frame CPU upload regions, command allocations and completion records to their actual native resources. An upload must be prepared before device use, its host writes must satisfy any required coherency and availability conditions, recorded commands must reference live allocations and descriptors, and a slot must not be recycled until submissions using its bytes are complete. For multiple queues, establish the proper cross-queue dependencies and track all relevant completions. Keep swapchain image acquisition and presentation semaphore reuse separate from frame-slot completion. Use the exact API documentation and validation tools rather than assuming that the CPU reference model's three counter values are Vulkan semaphore handles.

For classic Metal command-buffer paths, inspect the reference-retention behavior of the specific command-buffer creation method and any unretained-reference policy. A completion handler is a signal about a command buffer reaching its documented completed state, not a generic proof of every unrelated command stream. For Metal 4, inspect its distinct command-queue, synchronization and resource-retention rules; the [Metal 4 documentation](https://developer.apple.com/documentation/metal/understanding-the-metal-4-core-api) explicitly calls attention to differences from classic command buffers. Shared physical DRAM on Apple silicon does not make unsynchronized reuse of an in-flight upload region valid.

The frame ring could also be adapted for a CPU readback pipeline, but the access direction reverses. A device produces bytes, then the CPU may read them only after the required completion and visibility/coherency conditions. A ReadLease that simply points at host memory is not enough if the device has not yet made its writes available. The ring's CPU read pin expresses *lifetime permission*, not *memory visibility*. Our Pass 09 memory-contract oracle should remain a separate, composable part of that proof.

A production renderer needs error handling beyond this introduction: device loss, failed submission after a writer has committed, out-of-date swapchain acquisition, allocation failure, presenter dependencies, per-resource queue requirements, dynamic resizing, transient aliasing, and mapping/unmapping of platform-native memory. The model does not solve those cases just because it uses modern C++23 and RAII. Naming the open contracts is more valuable than presenting a few successful synthetic tests as a complete graphics backend.

#### Reconstruct the invariants without reading the implementation

First write down the ring's allowed states and transitions. Explain who can modify a draft, why the writer is move-only and when the draft is published. Derive the invariants `free_slot => no active read pin`, `in_flight => no active writer`, and `retired_generation => stale old handle`. Show why the pool owner ID is necessary when two rings both have slot zero, generation one.

Then prove the retirement predicate for one slot:

$$
\mathrm{mayRetire}(H)
=
\mathrm{validGeneration}(H)
\;\land\;
\mathrm{phase}(H)=\mathrm{IN\_FLIGHT}
\;\land\;
D\succeq R(H)
\;\land\;
\mathrm{readerCount}(H)=0.
$$

The formula is a **CPU bookkeeping precondition**; in a real renderer the observed completion vector must itself be established by the native device and its synchronization semantics. Test the predicate with $R=(1,2,4)$ and $D=(3,1,4)$, then with $D=(3,6,5)$. Keep an active ReadLease for the second case and explain why one further condition still blocks retirement.

Next show why a top-level const `std::span<Byte>` and an element-const `std::span<const Byte>` grant different operations. Prove why the unsafe `int** -> const int**` conversion would allow a const object's address to pass through a mutable pointer. Then analyze why a view returned by `ReadLease::view()` must not survive the lease, even though the lease itself can survive destruction of the outer ring wrapper. Deliberately produce a deep-owned snapshot and explain how it escapes the borrow interval.

After that, reconstruct the 24-byte NESF payload from a CPU register snapshot and the eight RGBA8 texture bytes. Predict every header byte, including the little-endian program counter and cycle counter. Verify that the source image or temporary staging vector can be altered after `replace` without changing the submitted frame. Retire slot zero only when the completion and reader rules permit, submit the fourth frame with a new generation, then prove the old handle fails. Finally explain why its frame slot is not the same identity as an acquired presentation image.

For more demanding validation, design additional tests for concurrent access using a proper lock, completion callbacks that arrive out of order on independent queues, source-authority loss during a device upload, malformed native resource sizes, managed storage that must be invalidated before CPU readback, and a device-lost path. Do not run undefined behavior on purpose to “prove” that escaping a span is dangerous; use compile-time type checks, state-machine refusals and sanitizer-backed safe fixtures to study the boundary.

#### The next expansion is not another detached example

This section adds one move-only upload ring, one real NES/texture/CPU integration lab and its proof obligations. It does **not** reduce the original chapter's broad coverage of GPU work generation, indirect draws, scan-based compaction, queue synchronization, Metal and Vulkan resource semantics, buffering, atomics and GPU algorithms. We return to that original material immediately after the insertion, with the frame-slot model as an executable reference for its later native examples.

The next chapter, “Commands, Async Work, and Resource Lifetimes,” should follow the same system into queued command state, explicit submission failure, completion callbacks, cancellation, device-loss and multi-owner resource retirement. It should derive actual command lifetime obligations from the frame ring rather than simply treating a command buffer as a convenient C++ function queue.

**Reference material checked for the concrete claims above:** [C++ `std::span` element/view lifetime and invalidation](https://en.cppreference.com/w/cpp/container/span), [C++ span constructors and const elements](https://en.cppreference.com/w/cpp/container/span/span), [Khronos Vulkan frames in flight](https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/03_Frames_in_flight.html), [Khronos swapchain semaphore reuse](https://docs.vulkan.org/guide/latest/swapchain_semaphore_reuse.html), [Khronos synchronization examples](https://docs.vulkan.org/guide/latest/synchronization_examples.html), [Apple Metal command-buffer reference retention](https://developer.apple.com/documentation/metal/mtlcommandbuffer/retainedreferences), [Apple Metal CPU/GPU synchronization](https://developer.apple.com/documentation/metal/synchronizing-cpu-and-gpu-work), and [Apple Metal 4 core API](https://developer.apple.com/documentation/metal/understanding-the-metal-4-core-api). These documents establish language/API semantics; their content alone does not certify any unbuilt or unexecuted C++23 tests in this repository.
