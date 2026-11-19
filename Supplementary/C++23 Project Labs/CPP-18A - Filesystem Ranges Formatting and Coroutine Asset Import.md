### CPP-18A — A file does not become an engine asset just because we opened it

We have reached a point where the system described in earlier chapters can create a typed texture or triangle mesh from a **span of bytes**, and the generational registry can give that owned asset a handle that cannot accidentally resolve to a later occupant. These are useful foundations, but they have also hidden a large part of the work that real engines must do. Where did the bytes come from? How did the loader decide which file it was allowed to open? Who owns the directory and the manifest while asynchronous work is pending? When did the temporary view become an owning object? Can one request be cancelled without corrupting the already published assets? And can a user diagnose a failed load without asking the debugger to inspect a dangling string?

An emulator ROM file, a glTF scene, a KTX texture, a Maveb Gaussian capture, a Kairo geometry cache and a Kyoto collision trajectory have different formats and downstream semantics, but **all of them begin as external, potentially malformed inputs**. The correct common lesson is not another emulator feature. It is a bounded, reproducible, ownership-conscious asset ingestion service written in modern C++23, demonstrated against the actual texture/mesh decoders and registry the reader has already implemented.

This is **Pass 18, CPP-18A**, and it answers the concrete coverage gap identified in Pass 17. The first 39 derived reading chapters and the sixteen prior supplements did not show a worked use of `std::filesystem`, `std::ranges`/`std::views`, `std::format`/`std::print` or the coroutine operators `co_yield`/`co_await` under their standard spellings. The absence of those spellings was an evidence-based *reading-corpus gap*, not proof that every external project lacked them. We now teach those language/library facilities in a single executable and explicitly relate them to the existing owned asset model. This lesson is a **new additive section in Chapter 06**, after the original typed/import and generational-handle material. It does not remove, regenerate or shorten the huge prior manuscript.

The main files are `tools/labs/foundations_course/modern_asset_ingestion.hpp` and `tools/labs/modern_asset_ingestion_cpp23_lab.cpp`. The header owns the filesystem root, bounded binary reader, explicit path/error rules, safe lazy range processing, a move-only coroutine generator, a separate manually resumed `co_await` coroutine, a `std::jthread` worker and feature-gated type-safe diagnostics. The executable creates real temporary directories and binary files; it tests path traversal, file size, symlinks where the OS permits, owned coroutine results, resumption, cancellation, duplicate manifests and eventual import through the **existing** `allmight::nes::assets::GenerationalAssetRegistry`. The texture/mesh fixture formats are the actual educational `TXR4`/`MSH1` schemas from Pass 05, not a pretend PNG/glTF reader or a copied mock of the original registry.

We first run into a problem that looks unimportant until the filesystem or a callback is controlled by someone else.

#### A path is a request, not proof that a file belongs to your engine

Suppose a Kairo editor receives the manifest entry `../private/secret-key.bin`. It would be unsafe to concatenate the selected asset directory with that relative string and assume the resulting pathname remains inside the project. The attacker may climb above the directory with `..`; the pathname may be absolute; a component may be a symbolic link; a directory can be replaced by a symlink after the program checks it; and different platforms parse separators, drive names, Unicode and case differently. A file named `stone.msh1` tells us nothing about whether its contents are a legitimate mesh.

A filesystem interface should therefore begin with a **trusted root identity** and a *typed relative asset request*, not a final absolute string supplied by the untrusted caller. In our example the root is an existing directory passed to the `Root` constructor. The constructor rejects a missing root or a direct symlink and resolves the directory to a canonical path; each subsequent asset request is independently checked.

A path \(p\) that claims to be relative is rejected when \(p\) is empty, absolute, drive-rooted or contains a component exactly equal to `.` or `..`. The intended destination is then formed by joining the trusted root \(R\) with the request \(p\). The loader checks each path component with `std::filesystem::symlink_status` and rejects encountered symbolic links. It canonicalizes the existing result and requires the canonical relative suffix to remain nonempty and free of ascent components. Finally, it requires a regular file before opening it.

Conceptually, the acceptance predicate is

\[
\operatorname{acceptPath}(R,p)
=
\operatorname{relativeNoAscent}(p)
\land\operatorname{noObservedSymlink}(R/p)
\land\operatorname{canonicalWithin}(R,R/p)
\land\operatorname{regularFile}(R/p).
\]

But that predicate is **not a secure, race-free OS sandbox**. Calls such as `symlink_status`, `canonical` and `std::ifstream` resolve the path separately. An adversary allowed to mutate the directory tree concurrently may swap a component between the check and the open. This is the time-of-check/time-of-use (TOCTOU) problem. The C++ standard filesystem library does not expose every kernel feature required to atomically constrain such traversal in a hostile directory. Native production implementations must use supported directory-handle-relative operations and no-follow/resolve-beneath semantics such as Linux `openat2` where appropriate, or the corresponding macOS/Windows facilities, then apply platform-specific tests.

Our loader consequently labels its path policy **best-effort trusted-project-root validation**, not a security boundary against a racing hostile writer. The negative tests prove that simple lexical traversal and stable symlinks are refused; they do not claim to prove the absence of a symlink race. This explicit limitation is a stronger engineering result than casually calling a sequence of filesystem checks “fully secure.”

This issue shows up in more than game assets. A scientific pipeline that reads benchmark files by seed can accidentally ingest another directory if user-controlled paths escape the corpus. A renderer extension that loads custom shaders needs to distinguish a project-scoped file from executable content in the user's home directory. Even a friendly local editor should avoid following unexpected symlinks silently when a project manifest has been imported from a third party.

#### Binary file I/O must have a hard upper bound before interpreting a header

A common beginner's implementation is to ask `std::filesystem::file_size`, allocate exactly that many bytes, then use `std::ifstream::read` to fill the vector. This is convenient for a small trusted file, but it makes a dangerous assumption about untrusted input: the file size is an appropriate allocation request. A malformed or deliberately huge input can cause enormous memory pressure before the program validates its format. The size may also change between the metadata query and actual reading.

The reference loader instead uses a fixed-size, stack-local temporary array of 4096 `char` values and reads from a binary `std::ifstream` one chunk at a time. It owns a `std::vector<std::uint8_t>` for the final raw asset. On each chunk it checks the remaining byte budget before appending any bytes. The maximum size is supplied when constructing `Root`. A file whose bytes would exceed the configured budget is rejected with `Issue::too_large`. It never advertises a successful asset result containing only an accidental prefix of the source file.

For a configured maximum \(M\), after reading chunks \(c_1,\ldots,c_k\) the invariant is

\[
0 \le \sum_{i=1}^{k}|c_i| \le M.
\]

The code checks each new \(c_{k+1}\) using `chunkCount <= M - bytes.size()` rather than `bytes.size() + chunkCount <= M`, because the latter addition could overflow before comparing. The bounded stream design also avoids trusting the filesystem metadata as an allocation oracle.

The output type is deliberately an **owning result**:

```cpp
struct OwnedAsset {
    std::string relative;
    AssetKind kind{};
    std::vector<std::uint8_t> bytes;
    std::uint64_t checksum{};
};
using Result = std::expected<OwnedAsset, Issue>;
```

There is no `std::span` pointing into the temporary 4096-byte array. The source filename is an owned `std::string` rather than a `std::string_view` into a temporary manifest. Both the pathname and the binary data stay alive after the file stream closes. The result's checksum is **FNV-1a**, a deterministic, inexpensive 64-bit fingerprint for fixtures and reproducibility comparisons; it is not a cryptographic content signature. For integrity against a malicious adversary use a cryptographic hash and a proper authentication/trust model, not FNV-1a.

An empty file may be a valid byte sequence at the filesystem level, but it is not thereby a valid texture or mesh. The file layer's job is to return a bounded sequence of bytes or a transport/path error. The **typed domain decoder** is responsible for deciding whether those bytes conform to `TXR4`, `MSH1`, iNES or another schema. This separation prevents every file reader from turning into a second, slightly inconsistent version of a texture or mesh parser.

#### Read errors and decode errors belong to different layers

A useful failure report identifies where the contract broke. A missing file, rejected relative path, symbolic link or unexpected stream error is an **I/O boundary** failure. A file containing the wrong magic bytes, a malicious vertex count, a triangle index outside the vertex array or a malformed image payload is a **domain-decoding** failure. A decoded mesh that the asset registry cannot publish because its capacity is exhausted is a **publication** failure. A GPU staging upload that cannot fit into the current frame arena is a **device-resource** failure. These must not all collapse into one Boolean.

For example, our real previous `Spec<TextureTag>::decode` expects the bytes to begin with `TXR4`, followed by little-endian width/height, reserved bytes equal to zero, then exactly \(4WH\) RGBA8 pixel bytes within a size limit. A file-layer result can pass all path and size checks yet fail that decoder because its width is zero or its payload is one byte short. The new Pass18 test hands owned `Result::bytes` to the original decoder by calling the existing `GenerationalAssetRegistry::import<TextureTag>`. It does not duplicate the decoder code. When the registry returns a typed `AssetHandle<TextureTag>`, a later lookup retrieves the published texture; erasing the asset invalidates the old generation, as previously derived.

The corresponding `MSH1` fixture contains exactly three quantized vertices and one triangle with three in-range indices. Its source bytes are checked by the same file service before the previous mesh decoder creates the owned vertex and index vectors. A successful file read is necessary but not sufficient; successful typed decoding is necessary but not sufficient for GPU upload; and a GPU handle is not a guarantee that the object remains referenced after deferred destruction.

A Maveb/Kairo production importer would eventually add real file-format adapters—glTF, KTX, PNG, compressed mesh containers, scene metadata—each with their own schema and security limits. The educational `TXR4` and `MSH1` formats are small on purpose: they let us prove the end-to-end ownership boundary without hiding it inside a giant third-party parser. We **do not** claim that the lab has implemented a glTF loader or imported a production Gaussian dataset merely because the same contract could support them.

#### The ranges library changes *how* a traversal is expressed, not who owns it

Earlier chapters introduced iterators, `std::vector`, sorting, filtering and projections separately. C++20's ranges facilities make it possible to express some of those operations with a stronger type contract. A range is an object whose beginning/end define a sequence; a view generally provides a cheap, potentially lazy *way of traversing* an underlying range. It is not automatically a deep copy or a lifetime-extending owner.

We use an owned `std::vector<Request>`, sort it by its `relative` string and filter intentionally disabled manifest entries:

```cpp
std::ranges::sort(requests, {}, &Request::relative);

for (const Request& request :
     requests | std::views::filter(
         [](const Request& r) { return r.enabled; })) {
    // The view is used right here; the vector remains owned and alive.
    // All file results are copied/moved into an owning vector.
}
```

The projection `&Request::relative` tells the ranges algorithm to compare the relative string member rather than require a custom ordering operator for the entire request struct. The view's predicate is not silently performing security validation: disabled entries are an explicit manifest feature. Every **enabled** path still goes through the complete filesystem validation. In particular, an empty enabled path must yield a failure, not quietly disappear from the pipeline.

A filter view of an lvalue vector commonly stores a reference-like view of that vector. If a function returns the view after destroying the vector, the returned view will dangle. Even when the view object has been moved correctly, its iterators can become invalid if the underlying vector reallocates or elements are erased. `std::ranges::borrowed_range` and `std::ranges::dangling` help express and diagnose certain iterator lifetime properties, but they are not universal borrow checkers.

The safe story in our implementation is that **the coroutine frame owns the vector of requests**, and its range view exists only while that same coroutine frame is executing. It is not returned to a caller and is not stored in another worker after the frame has been destroyed. The test deliberately clears the caller's original request vector after creating the coroutine. Because the parameter was passed by value, the frame still has its own manifest strings. The results own their own path strings and byte vectors.

A further subtlety is that the coroutine is **initially suspended**. When `make_batch` is called, its body does not immediately execute; parameter copies/moves and the promise/frame are created, and execution begins only when the caller resumes the handle. Passing `std::span<const Request>` into such a coroutine would be unsafe if the caller destroyed or resized the source vector before resumption. Passing an owning vector avoids that specific dangling view. The generator is move-only precisely because duplicating an owning coroutine handle would risk destroying the same frame twice.

#### A coroutine is not a thread, and co_yield is not a task scheduler

One of the most important modern C++ misunderstandings is to see `co_await` or `co_yield` and assume the runtime has started work on a thread pool. Standard coroutines define **suspension and resumption** of a function whose local state can survive between calls. Where that work executes is determined by the awaiter, executor and calling code. There is no universal standard thread pool silently launched by writing `co_await`.

The `Generator<T>` in this pass is a small explicitly owned coroutine wrapper. Its promise contains an `std::optional<T> current` for the most recently yielded item and an `std::exception_ptr` for failures. It initially suspends before the body, suspends at each yield, and suspends at the end so the owner can destroy the frame correctly. `Generator::next()` resumes the frame once, moves the current result into an owning optional, returns it to the caller, and checks the terminal promise exception state when the coroutine finishes.

The key arrangement looks like this:

```cpp
struct promise_type {
    std::optional<T> current;
    std::exception_ptr failure;

    std::suspend_always initial_suspend() const noexcept { return {}; }
    std::suspend_always final_suspend() const noexcept { return {}; }

    std::suspend_always yield_value(T value) noexcept {
        current.emplace(std::move(value));
        return {};
    }
    void return_void() const noexcept {}
    void unhandled_exception() noexcept {
        failure = std::current_exception();
    }
};
```

The caller owns a `std::coroutine_handle<promise_type>`. Its destructor calls `destroy()` exactly once if a handle remains. Move construction transfers the handle and clears the source. Copying is disabled. Those are not style preferences; they are the **resource ownership rules for a compiler-allocated coroutine frame**. A double destroy or resumption of a destroyed handle is undefined behavior.

Each `co_yield root.load(request)` places a fully owned `std::expected<OwnedAsset,Issue>` into the promise. The next caller resume consumes that result. If a decoder and registry later retain data, they must do so through their own owned types and handles; they may not retain a `std::span` to a temporary stream chunk or to a vector that the coroutine will overwrite on its next iteration.

The generator is **synchronous** unless a caller resumes it on a different thread. Its iteration naturally introduces backpressure: the caller cannot request the next file until it has handled the current yield, and the generator holds only its current result rather than returning a complete batch of raw file bytes at construction time. Our separate `WorkerBatch` collects returned results into an owning vector, but **enforces an aggregate byte budget** (default 8 MiB) before retaining each successful result. If a new asset would exceed the remaining budget, it emits `Issue::aggregate_budget` and stops accepting further results. A dedicated test proves this with two real files and a 24-byte batch limit. This bounds retained successful file bytes, though the number of error records and input manifest entries still needs an independent limit under hostile workloads. A production pipeline should add a bounded producer-consumer channel, explicit maximum job count, per-batch and in-flight allocation budgets, and responsive stop-aware blocking operations.

#### The difference between suspend_always and an actual asynchronous wait

The body of `make_batch` uses `co_yield`, which suspends after publishing a value. To make the semantics of `co_await` equally visible, the lab also includes a deliberately small `ManualTask` with a separate `publish_after_checkpoint` coroutine:

```cpp
ManualTask publish_after_checkpoint(
    std::shared_ptr<std::atomic<int>> value,
    std::stop_token stop,
    int result)
{
    co_await std::suspend_always{};

    // The caller controls when to resume; no hidden thread resumes us.
    if (!stop.stop_requested())
        value->store(result, std::memory_order_release);
}
```

The function initially suspends when called because its promise requests `initial_suspend = suspend_always`. The first explicit `resume()` begins the body and hits the `co_await suspend_always` checkpoint. The second resume continues past that point, checks the stop token and stores the result only if cancellation has not been requested. The tests assert this ordering and request cancellation between the two resumes.

This particular awaiter does not schedule a kernel, register an I/O completion or spin up a background worker. It is a pure manual suspension point, useful for understanding the lifetime and state machine. A real asynchronous awaiter would generally register a continuation with an executor or device callback and arrange for safe resumption when the awaited event becomes ready. That requires a more demanding contract: who owns the coroutine frame while suspended, whether a cancellation can race with a completion, whether the awaiter's captured resources remain valid, what synchronization makes results visible, and which thread is allowed to resume the frame.

Notice that the `publish_after_checkpoint` coroutine accepts the `shared_ptr` **by value**. Therefore the coroutine frame keeps the atomic result object alive even if the original caller drops its shared pointer before resuming. Passing a reference to a temporary shared pointer would not provide the same guarantee. The test uses a `weak_ptr` to prove the ownership relation: the result remains live while the suspended frame owns it, and no publication occurs if the stop source is requested after the first suspension.

Coroutine destruction has its own important edge cases. Destroying a suspended coroutine releases its live local/parameter objects, but destroying a coroutine while another thread is about to resume it would be unsafe without external synchronization. The teaching generator is explicitly single-resumer; it is not a concurrently resumable lock-free task. That constraint remains in force even though `Generator<T>` itself contains an owning handle and the payloads are owned.

#### std::jthread makes actual host concurrency explicit

When we want real concurrent file processing, we deliberately introduce a host thread. The `WorkerBatch` constructor owns a `std::shared_ptr<State>` containing a mutex and collected results, then creates a `std::jthread` that constructs and resumes the owned generator. The lambda captures the root and manifest by value. Only one thread resumes the coroutine, while `WorkerBatch::finish()` joins the worker before moving the collected results to the caller.

That design makes two separate lifetimes observable. The **coroutine frame** owns its request collection and one current yielded asset. The **worker's shared result state** owns all outcomes already consumed and transferred into its vector. When the worker ends, the coroutine frame is destroyed, but the completed result vector remains valid. A mutex protects state publication for any future consumer that might read concurrently, although the current `finish()` waits for the worker to terminate before returning the results. A simple mutex is easier to prove correct here than an unnecessary lock-free queue.

The stop token is cooperative. Calling `request_stop()` asks the worker to stop; it does not kill a thread in the middle of a C++ object mutation. The generator checks `stop_requested()` between requests, and a request already reading a file is allowed to finish that read. The worker stops consuming after an observed stop request. There is no false claim of kernel-cancelled or OS-I/O-interrupted execution. A more advanced service can use OS cancellation facilities where available, and must still reconcile partial file operations and native completion.

The tests cover a deterministic cancellation before the first request and cancellation after the `co_await` suspension point. Race-sensitive cancellation during an arbitrary file read is not claimed to be deterministic. A real service should use controlled test barriers or a fake I/O provider to force every cancellation interleaving, then test that published owned results remain valid and that a cancelled task does not mutate a later generation.

Crucially, `std::jthread` differs from raw `std::thread` in teardown: when joinable, its destructor requests stop and joins. That is a useful default for an owned worker lifecycle, but the owner still must avoid designs in which the worker waits forever on a resource that the destructor is trying to destroy while waiting for that same worker. Stop requests need responsive checkpoints or stop-aware blocking operations. Our simple bounded file loop does not guarantee prompt cancellation during a slow network filesystem read, so it is a teaching baseline, not a complete production task scheduler.

#### The transaction boundary moves from files to decoded assets to registry handles

Let \(B\) be the owned raw bytes read from the filesystem. Let \(D(B)\) be the type-specific decoder, such as the actual `Spec<TextureTag>::decode`. Let \(P(D(B))\) be the registry's publication operation. The desired invariant is:

\[
\text{published asset exists}
\quad\Longrightarrow\quad
\text{read completed}
\land\text{format validated}
\land\text{publication committed}.
\]

An I/O error must not produce a half-filled asset; a decoder error must not partially publish a mesh; a registry capacity error must not invalidate previously published handles. These are separate strong guarantees joined into one end-to-end contract.

The earlier `GenerationalAssetRegistry` implements a transactional preparation/publication scheme, with a typed source decoder, a prepared variant and a stable owner/generation handle. In the new integrated test, the filesystem returns an owned texture and triangle mesh byte vector. Each vector is passed as a temporary `std::span<const Byte>` to the existing registry import call. That span is borrowed **only for the duration of the synchronous decoder/import**; the registry prepares and owns the decoded asset before returning. After the call succeeds, the bytes returned by the filesystem can be destroyed without invalidating the published texture or mesh.

The reference interoperation in the test is:

```cpp
auto textureBytes = root.load({"assets/tex.txr4",
                               AssetKind::texture_rgba8});
auto meshBytes = root.load({"models/tri.msh1",
                            AssetKind::triangle_mesh});

GenerationalAssetRegistry registry(2);
auto textureHandle =
    registry.import<TextureTag>(textureBytes->bytes);
auto meshHandle =
    registry.import<MeshTag>(meshBytes->bytes);
```

The actual test checks both `std::expected` results before committing, then looks up the published texture and mesh and verifies the image dimensions, pixel count and triangle geometry. Erasing the texture must make the original texture handle stale. This is precisely where Book 1's first six chapter contracts connect to an input source and to a real asset store.

The test is conditional on the actual earlier shared registry header being on the compiler's include path; in the normal full Book01 CMake build it is part of the same public include interface and should be exercised. A **standalone local smoke build** that has only the new source files can still test filesystem, ranges, coroutine, cancellation and jthread behavior but clearly announces that the previous registry integration was not compiled. The full course's CMake/CTest run must be observed before claiming that earlier-to-later integration passed on the repository branch.

For a real Kairo engine resource, the next boundary is the GPU uploader. It should consume a stable typed asset and schedule an explicit transfer, not receive an arbitrarily mutable span to a temporary vector and hope the CPU doesn't overwrite it. The registry handle may remain alive while a GPU upload is outstanding; deleting or replacing that asset should trigger deferred resource retirement under the Pass 11–16 lifetime rules. The file system doesn't know about that lifetime, and it should not be tasked with solving it.

#### std::format and std::print are diagnostic APIs, not a reason to ignore toolchains

C++20's formatting library and C++23 printing facilities are important practical additions to a modern C++ curriculum. With `std::format`, the format string expresses the output structure while arguments remain typed; the implementation can check a literal format string against argument types where the supported standard/library requires it. With `std::print` and `std::println`, formatted output can be written more directly than chaining `operator<<` expressions.

The Pass18 loader builds its rejection message using the actual C++ formatting API when the standard library advertises it:

```cpp
#if defined(__cpp_lib_format) && __cpp_lib_format >= 201907L
return std::format("asset '{}' rejected with code {}",
                   file, static_cast<int>(issue));
#else
// A transparent fallback for standard libraries without <format>.
return "asset '" + std::string(file) +
       "' rejected with code " +
       std::to_string(static_cast<int>(issue));
#endif
```

The separate `announce_success()` uses `std::println` when `__cpp_lib_print` is available, otherwise it simply refrains from printing that additional decorative status. The feature-gated code is compiled with the real GCC/Clang standard libraries present in the authoring environment; some AppleClang/libc++ combinations may have different library coverage, and support must be verified by actually building the enabled path on that platform.

Formatting is not an input sanitizer. A filename may contain odd Unicode, terminal control characters or private information. Putting it inside a formatted log can still make a terminal confusing or disclose a sensitive path. For untrusted file names, an engine should escape control characters, keep structured error codes, and avoid logging secrets or entire asset contents. User-facing diagnostics should distinguish **what went wrong** from **where the internal filesystem happened to be mounted**.

A formatting API also does not determine the correct numerical precision or scientific meaning of a measured value. A Kyoto benchmark that prints “1.002x” needs to state the sample set, aggregation formula, baseline and confidence evidence. An XR latency that prints “9.1 ms” needs the source clocks and frame identities. Standard formatting makes diagnostics more readable; it does not make ungrounded metrics true.

#### C++ exceptions, expected errors and coroutine promise failures must be reconciled

We already distinguish expected input failure from programmer error and resource exhaustion. The new loader returns `std::expected<OwnedAsset,Issue>` for ordinary path and stream refusal. The coroutine's `promise_type::unhandled_exception()` captures unexpected exceptions in `std::exception_ptr`, which `Generator::next()` rethrows only after the coroutine reaches its terminal state. This prevents exceptions from silently disappearing when a coroutine resumes through nested library code.

We must not confuse this with a complete no-throw task executor. Allocation failures in `std::vector`, `std::string` or a coroutine frame can still propagate. An exception escaping a `std::jthread` function would call `std::terminate`; the revised `WorkerBatch` therefore encloses its worker loop in `try/catch(...)`, stores `std::exception_ptr` in shared state, joins, and rethrows that exception from `finish()` on the caller's thread. It also enforces an 8 MiB default aggregate byte cap (configurable and independently tested).

This is not a hard real-time worker or a transaction across all assets. A caller must explicitly handle partial results, source/data errors and a possible rethrown exception rather than reporting “all assets loaded.” The worker does not yet impose a separate input-manifest job-count limit, a bounded consumer handoff queue, deterministic cancellation of a blocked file read, or a race-free native filesystem sandbox. Those remain production-level follow-ups. A byte cap alone does not bound the memory used by an upstream manifest with a million paths or by numerous error records.

There is another subtle lifetime issue in asynchronous file loading: the **request list itself** can become stale even if copied safely. A caller may replace a Kairo scene or Maveb reconstruction while the worker is still decoding the original manifest. The worker's results then belong to a particular scene/asset revision; the publisher must check that revision before inserting them into the live registry. A pointer-safe task can still publish logically stale data if it doesn't carry revision identity.

This is why generation IDs and cancellation interact. When the user loads a new scene, request a stop on the old worker, but do not assume all old tasks stopped immediately. Tag results with the old revision and reject them at the publication boundary if they are no longer current. A late completed task should be allowed to clean up its own buffers; it should not mutate an unrelated new scene generation.

#### The negative cases are as important as the successful mesh

The executable creates a temporary directory with a synthetic 2×1 RGBA texture and a one-triangle mesh. It verifies the exact bytes and deterministic FNV-1a fingerprint after binary file I/O. It refuses `../outside.msh1`, an absolute `/etc/passwd`-shaped request, `./assets/...`, a directory ascent embedded in the pathname, an empty path, a nonexistent file and a directory used as a file. It also creates a symlink to an existing mesh when the host OS permits that operation and verifies that the link is rejected.

A separate cap test sets the maximum read size smaller than the file and requires an explicit `too_large` result. The coroutine test supplies two enabled requests out of order, ensures `std::ranges::sort` produces the expected order, and deliberately clears the original caller's request vector after the coroutine is created. The yielded texture and mesh bytes remain correct. After the asset is yielded, the test removes the original texture file and confirms that the owned result still contains its source bytes.

Additional tests request cancellation before the generator begins, show an explicit cancelled result, and verify a duplicate path does not get silently accepted twice. The `co_await` test requires two explicit resumes, drops the caller's shared pointer while the frame is suspended, then checks the published atomic value. Its cancellation variant stops between resumes and checks that no value was published. Finally, an actual `std::jthread` executes the batch and returns the owned results after joining.

The complete CMake course target includes the earlier generational registry; its test verifies the decoded 2×1 texture and three-vertex/three-index mesh, then proves that a removed texture handle cannot be resolved again. The local standalone compilation and sanitizer runs cover only the new files when the prior registry header isn't on the include path. Distinguishing those outcomes is part of the lesson's evidence protocol.

#### What the authoring environment actually proved

For this pass, the new standalone ingestion source was compiled and run locally under **GCC 14.2 C++23 Debug**, **Clang 17 C++23 Debug**, **Clang 17 AddressSanitizer/UndefinedBehaviorSanitizer**, and a **GCC 14.2 optimized Release** build. The runs exercised real temporary filesystem I/O, ranges, coroutine resumption, manual `co_await`, cooperative stop, host `std::jthread` and feature-gated formatting/printing. All four passed in the authoring container; the non-Apple authoring container has no direct view of the user's actual Maveb/Kairo local checkout.

Those results **do not** establish that all seventeen Book01 targets compile on the GitHub revision branch. The new interoperation against the existing registry header requires the full course include tree, and the native Metal and Vulkan probes still require hardware-specific checks. Source files for those prior projects are registered in CMake, but there is no observed full-source CTest or native GPU receipt from this turn. The new test is not a substitute for the final integrated build or the user's own device acceptance work.

The local filesystem tests also do not prove a race-free sandbox against a hostile concurrent directory writer. The code intentionally documents the remaining TOCTOU window; a secure production loader must use OS-specific atomic path-resolution primitives, platform-specific path/encoding policies and stronger fuzzing. This is especially important if the user eventually lets a plugin or internet-downloaded scene supply arbitrary manifest paths.

#### Rebuild this service in five projects without duplicating the algorithm

For **Kairo's scene editor**, replace the toy `TXR4`/`MSH1` adapters with actual asset formats, keep typed decode errors and generational publication, and add a scene revision so a cancelled import can't publish data into the wrong scene. Expose progress to the UI through a bounded queue delivered on the correct execution thread. Add failure injection where allocation, file I/O or GPU upload fails at each stage; use an owned result to determine whether a stage committed.

For **Maveb**, load a measured sequence of reconstruction assets, such as mesh chunks, Gaussian records or camera metadata, while the current frame continues rendering. Use the same ownership guarantees to keep staging buffers alive until their GPU consumption completes. A file read that returns successfully does not prove a GPU upload finished; a GPU upload completion does not prove a splat visualization is numerically correct. Attach source and scene-generation provenance to every published dataset.

For **Kyoto collision research**, use a typed manifest to describe material, regime, object count, seed, source file digest and expected exact-pair oracle. Lazy range traversal can help select subsets, but a filter view must not allow a stale benchmark dataset vector to disappear while a worker is still iterating it. A source change should create a new benchmark revision, not silently overwrite results under the same identity.

For **Vulkax scientific validation**, ensure input CSV/trajectory files are bounded, parsed with explicit units and independently validated before computing residuals. The same C++23 error model can be mirrored in Python's structured exception/result contract. Pair paths with content digests, experimental conditions and code revisions so a missing trial is reported rather than silently omitted from a geomean.

For **XR/foveated rendering**, treat a gaze sample as an input associated with a timestamp and frame generation, not merely a number to print. A stopped or late asset/shader preparation task must not publish an old foveation pattern into a newer eye pose. Cancellation, lifetime and revision checks protect logical correctness; separate device and display timeline measurements protect latency claims.

The important lesson is that filesystem validation, typed decoding, coroutine ownership, ranges and async publication are **one continuous resource transaction**. They should be learned while constructing a useful engine interface, not as four pages of unrelated syntax and not as a shallow reference list. The correct result is a program that can explain where every byte came from, which type promises its meaning, which owner protects its lifetime, where cancellation is checked, and what proof is required before the next system may safely use it.

#### Reconstruction checkpoints for the reader

First replace the fixed `TXR4`/`MSH1` fixture input with one carefully documented real format. Write a byte-layout diagram before the parser, enforce every extent calculation without integer overflow, and verify malformed corpus inputs including missing terminators, excessive counts, wrong endianness and truncated payloads. Decide how a decoder reports the difference between unsupported format versions and corruption.

Second change the loader into a truly bounded producer-consumer service. Limit not only bytes per file but also simultaneous open file descriptors, outstanding decoded bytes, queued results, retry budget and aggregate batch memory. Write a fake file provider that blocks at a controlled point so cancellation and scene-revision replacement can be tested deterministically. Ensure producer destruction, worker stop, coroutine-frame destruction and registry publication follow one proven ordering.

Third implement the native filesystem hardening appropriate to the target OS, with directory-relative handles, no-follow options and safe output publication. Explain why checking path strings alone is insufficient and why a symlink check before opening has a race. Add a controlled adversarial test that swaps a path component during repeated load attempts, while explicitly distinguishing a reliable POSIX/Linux proof from a test that happens not to reproduce the race on macOS.

Fourth connect loading to a real GPU staging command. Store the typed asset while the upload is pending, defer publication of the GPU-resident representation until the native command has completed and its result can be validated, and keep old generations alive while in-flight renderers borrow them. Reuse the frame-retirement contracts from Chapters 11–16 instead of inventing a new lifetime policy.

Fifth write a reproducible experiment report. Record source bytes/digest, asset-format version, compiler/standard library, sanitizer configuration, file sizes, decode time, publication time, optional native GPU upload and rendering times separately, failure count and exact output oracle. For a speed claim, compare equivalent workloads across multiple runs with correctness checks, not a single warm cache read.

**Acceptance boundary of Pass 18:** substantive narrative covering actual C++23 filesystem/ranges/format/print/coroutine semantics and common real-world pitfalls; an owned, locally compiled portable asset-ingestion implementation; a new CMake/Ctest lab designed to integrate the previous typed decoders and generational asset registry; explicit failure and cancellation tests; byte-preserving manuscript insertion staging. The entire seventeen-target course build, native devices and production filesystem/security race defenses remain independently verifiable work.

**Primary documentation referenced:** [C++ coroutines and coroutine frame lifetime](https://en.cppreference.com/w/cpp/language/coroutines), [C++ ranges and borrowed ranges](https://en.cppreference.com/w/cpp/ranges), [C++ formatting library](https://en.cppreference.com/w/cpp/utility/format), [C++23 printing](https://en.cppreference.com/w/cpp/io/print), [C++ filesystem status and symlink_status](https://en.cppreference.com/w/cpp/filesystem/status), [C++ standard feature test macros](https://en.cppreference.com/w/cpp/feature_test), and the actual previously committed Pass05 `typed_asset_pipeline.hpp` / Pass06 `generational_asset_registry.hpp` source. Relevant native OS directory traversal rules must be validated per platform rather than inferred from portable filesystem APIs.

**Continue the original Chapter 06.** The existing `From a living asset registry to reliable queries and transformations` section remains untouched and now has an explicit external-byte and asynchronous-manifest example that explains why range views and typed generational handles belong together. The next engineering pass should compile/repair the full seventeen-target course and harden worker exception containment and true bounded async publication, before a reader is told the full Book 1 is release ready.
