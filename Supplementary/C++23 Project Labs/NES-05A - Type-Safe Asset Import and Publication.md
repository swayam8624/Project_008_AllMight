### NES-05A — One import transaction, three different asset meanings

The previous pages explain why a template can reuse an algorithm without erasing the types it works with. They also distinguish forwarding, traits, concept constraints, tag dispatch, compile-time policies, and virtual interfaces. Now the evolving emulator gives those ideas a system-level test: an asset loader receives files of three fundamentally different kinds. One file holds a legacy NES cartridge image; another holds an RGBA texture; a third holds a triangle mesh. All arrive as bytes, but they do not share a binary format, a meaningful unit of size, or a device-access contract. If our generic program treats the three as interchangeable byte arrays, it will erase the very invariants that the previous four chapters taught us to preserve.

We will write a single **C++23 import-and-publication operation** that keeps different source validation rules behind separate, statically selected type specifications. It will use the actual `NromCartridge` class we built in NES-01 and NES-02, with its existing parser, data ownership and mapper checks. The two new asset formats are deliberately small and documented as educational formats—not misleading substitutes for PNG, KTX, glTF, OBJ, or an actual Vulkan or Metal resource. The result is an executable architecture to which later engine importers can be attached without pretending those standards have already been implemented.

This is **NES-05A, the generic asset-loading milestone**, not a claim that we have reached the full NES-05 PPU renderer. The numbering records the teaching connection: the emulator is now one consumer of a reusable C++23 asset pipeline, while geometry and image representations become two other consumers. The cumulative source resides in `tools/labs/nes_course/typed_asset_pipeline.hpp`; the independent test executable is `tools/labs/typed_asset_pipeline_cpp23_lab.cpp`. They depend on the same NES-01 and NES-02 core headers already used by prior passes. No earlier section of the chapter is replaced.

#### Why a file's bytes cannot determine the application's type by themselves

An untrusted sequence such as `01 00 00 00` cannot tell us whether it means an integer, a four-channel pixel, a section of 6502 program ROM, a quantized vertex coordinate, or something else. A magic sequence may indicate the intended schema, but is not proof that the rest of the input obeys it. A loader has to know a *format contract* before decoding, and a consumer has to know a *domain contract* before using the decoded result.

Our experiment introduces three distinct tag types:

```cpp
struct CartridgeTag {};
struct TextureTag {};
struct MeshTag {};

template<class Tag>
struct Spec;  // No implementation for unknown asset kinds.
```

This declaration means that a generic importer can only be instantiated for types with a deliberately supplied specialization. It does not define a universal conversion from any arbitrary bytes into any arbitrary C++ object. `Spec<CartridgeTag>` names the NES cartridge type and calls the real `NromCartridge::from_image` factory. `Spec<TextureTag>` validates a small RGBA8 image format; `Spec<MeshTag>` validates a small triangle mesh format. The import algorithm sees the same abstract result—either a fully owned domain value or a typed source error—while the semantics of the bytes remain with the appropriate specialization.

A C++23 concept makes that architectural boundary visible to the compiler:

```cpp
template<class Tag>
concept Supported = requires(ByteView bytes) {
    typename Spec<Tag>::asset_type;
    { Spec<Tag>::decode(bytes) } ->
        std::same_as<
            std::expected<typename Spec<Tag>::asset_type, DecodeError>
        >;
};
```

The requirement has two parts. The first demands that the specification name a concrete domain type; the second demands that decoding produce exactly the expected result form. `Supported<CartridgeTag>`, `Supported<TextureTag>` and `Supported<MeshTag>` are true, but an arbitrary `UnsupportedAsset` has no specialization and cannot instantiate the importer. This is not merely a descriptive comment that a template “expects an importer.” It is a compile-time rejection rule. It does not, however, mathematically prove that a provided specialization has correct size checks; semantic correctness still needs a source review and tests.

Notice the deliberate separation between **static choice** and **runtime choice**. The caller knows the requested type at the call site and selects it using the template argument. Runtime fields such as image dimensions, mesh index counts and cartridge mapper IDs still require checking. Compile-time specialization can prevent calling a texture decoder as a cartridge decoder by accident, but it cannot make an untrusted file's length valid without examining that file.

#### The texture's first twelve bytes are a teaching contract, not a C++ struct

To make the validation concrete, our educational texture wire format begins with four ASCII bytes `TXR4`. The next two bytes encode width as an unsigned little-endian 16-bit integer, the next two encode height in the same way, and the next four bytes are reserved and must be zero for this version. The payload contains exactly four bytes per pixel, in RGBA channel order, with no row padding, no mip levels and no compressed blocks. A complete image is therefore expected to have

$$
L_{\text{texture}} = 12 + 4WH
$$

bytes, with positive integer width $W$ and height $H$.

If $W=2$ and $H=3$, there are six pixels, twenty-four payload bytes and thirty-six total file bytes. The decoder reads the two size fields from their declared offsets. It does not read the entire header into a native structure and assume that a particular ABI will pack or align the fields exactly as desired. If the dimensions or reserved bytes disagree with the contract, the input is rejected before any object is published.

The multiplication itself is part of the contract. In a general format with larger dimensions, calculating `4 * width * height` using unchecked `std::size_t` arithmetic can silently wrap and produce a plausible smaller size. Our code provides a checked multiply operation: before forming $ab$, it verifies that either $a=0$ or $b\le\mathrm{SIZE\_MAX}/a$. We then enforce a specific **one-megabyte maximum raw pixel payload** for the educational importer. This is an application resource budget; it is not a statement that real GPUs have a one-megabyte texture limit. The test accepts a 512-by-512 RGBA8 texture exactly at that boundary and rejects a 600-by-600 example without allocating its advertised payload.

The decoder rejects both truncated and trailing bytes. That is a chosen strict contract. A more extensible production format may explicitly support trailers, concatenated records or versioned optional sections, but those are new schema rules and must be specified before being accepted. The zero reserved bytes are therefore not silently treated as vendor extensions.

The decoded `TextureRgba8` contains width, height and an owned `std::vector<std::uint8_t>` of pixels. This is **CPU-owned decoded image data**, not a Metal texture or Vulkan image. A GPU upload has additional requirements: device format support, row pitch and alignment, transfer commands, staging lifetime, synchronization, completion and potentially color-space metadata. A working CPU decoder cannot certify that any of those downstream steps have occurred. Later books will add those policies where they are needed.

#### The mesh asks a different validation question: which vertices do the indices name?

Our educational mesh format begins with `MSH1` followed by a little-endian unsigned 16-bit vertex count, a little-endian unsigned 16-bit index count and four reserved zero bytes. The payload first stores every vertex as three little-endian signed 16-bit coordinates: X, Y and Z. Each stored coordinate represents a point on a fixed grid of 1/256 world unit. After the vertex array comes an array of unsigned 16-bit triangle indices.

This is a deliberately quantized representation. It introduces the same error-versus-memory trade-off taught in the earlier numerical material: a coordinate $q$ represents $q/256$ world units, and real positions not lying on the grid require a declared quantization rule upstream. We are not pretending this format can preserve arbitrary floating-point geometry without loss, or that six bytes per vertex already include normals, texture coordinates, skinning data, material groups or tangents.

For $V$ vertices and $I$ indices, the required file length is

$$
L_{\text{mesh}} = 12 + 6V + 2I.
$$

A valid triangle list must have at least three vertices, at least one triangle and an index count divisible by three. In this training format the count is also limited to 8,192 vertices and 24,576 indices so a malicious file cannot induce unbounded resource usage. Each index must be strictly less than $V$; checking only that the index array fits in the file is insufficient. A file may contain every advertised byte and still refer to vertex 3 when its vertices are numbered only 0, 1 and 2. Such an index would be **semantically invalid** despite having a structurally correct length.

Our test uses three vertices at the quantized positions (0,0,0), (256,0,0) and (0,-256,0), with indices (0,1,2). A byte-exact little-endian decoder recovers the signed coordinate -256 and all three indices. No native-struct reinterpretation is involved. The code uses a bit-preserving `std::bit_cast<std::int16_t>` from the validated exact-width unsigned word, rather than relying on an out-of-range unsigned-to-signed value conversion. The source and destination types are explicitly sized for the representation, and the file-format contract supplies the byte order.

This mesh representation can become an intermediate source of vertices for a future renderer, collision geometry or a reconstruction project such as Maveb. It does **not** yet solve triangle winding conventions, degeneracy, topology consistency, mesh repair, GPU vertex packing, normals or acceleration-structure construction. Those are substantive future topics; none is inferred from the mere presence of an index buffer.

#### The NES cartridge remains its own domain object

A third specialization wraps our existing `NromCartridge::from_image` operation. It uses the legacy iNES inspection path, the supported mapper-zero restrictions, the program-ROM size checks and the owned CHR-ROM/CHR-RAM behavior already developed in earlier chapters. We do not duplicate the iNES byte decoder inside the generic asset library. That preserves one authoritative implementation and avoids introducing two versions of the same format contract that might drift apart.

The generic importer translates the cartridge factory's typed failures into its own source-error vocabulary. A rejected mapper remains distinguishable from unsupported NROM geometry or a generally invalid image. This is a deliberate loss of some parser-specific detail in exchange for a shared outer error channel; a more diagnostic production interface should preserve nested parser causes and file offsets rather than collapsing them all into “invalid input.” We call attention to that trade-off now so an advanced learner can refine it deliberately.

The source buffer remains borrowed for the duration of decoding, while the returned cartridge owns independent PRG and CHR vectors. Clearing or destroying the original file buffer therefore does not invalidate the mounted cartridge's program bytes. The test checks this using the same NROM code as the earlier project milestones rather than a fake cartridge structure that merely resembles it.

#### A heterogeneous store does not require void pointers

We want one collection that can hold the three supported domain types without throwing away their identities. A closed set of types fits `std::variant`:

```cpp
using AssetRecord = std::variant<
    NromCartridge,
    TextureRgba8,
    TriangleMesh
>;
```

Unlike `void*`, this variant tracks which alternative is alive. Unlike a virtual base class, it does not require the three asset types to inherit from a common interface. Unlike an independently specialized `AssetDepot<T,N>` per type, it permits one append-only sequence of heterogeneous owned values. Each approach has different space, compilation, dispatch and extensibility consequences. A variant is attractive here because the set of domains is deliberately closed and small; an extensible plugin architecture may require a different representation.

The generic `AssetStore::import<Tag>(raw)` stages an object using `Spec<Tag>::decode`, checks success, then appends the already-owned result as the correct variant alternative. It returns `std::expected<AssetId<Tag>, Failure>`. The tag parameter means a cartridge slot token and a texture slot token have different C++ types. A client cannot casually assign one to the other, while the store's runtime lookup also checks that the slot actually contains the requested variant alternative.

The slot identifier itself has a deliberately narrower guarantee. It is merely an index in **this single append-only store**. It is not a globally unique asset identity, not a generational handle, not a cryptographic content digest and not a cross-store capability. An attacker or a mistaken caller can fabricate an integer slot; lookup must still validate bounds and the stored alternative. The next existing chapter, “Grow Registries without Invalidating Identity,” is where generations, deletion, reuse and multi-world identity will be developed against this same running project.

The store also returns a pointer to a particular asset when the caller asks for a typed lookup. That pointer is a **borrow**, not an owner. A later insertion may force the underlying `std::vector<AssetRecord>` to reallocate, moving the contained asset records and invalidating pointers to their former addresses. The test therefore stores the *slot token*, performs many additional imports, and then asks the store to look up the asset again. It never dereferences a pointer that might have been invalidated. Re-looking up by index works under the append-only rule; retaining the old native pointer across growth does not become safe merely because the index remained meaningful.

#### An atomic *single insertion* is not an atomic *batch*

Our importer has a clear observable publication boundary. Consider a store whose record vector currently contains $n$ completed assets. A malformed or unsupported input must produce a source error with the same $n$ published records. A failure before staging or a simulated allocation failure after staging must also leave those records untouched. Only the successful completion of a new append changes the record count to $n+1$ and returns the new slot ID.

A naïve implementation might append a record first and then validate it, or might start mutating a published record's payload while it is still being decoded. Both allow partially initialized data to become visible. Our sequence is instead:

```text
borrow input bytes
    |
select asset specification at compile time
    |
validate external layout and semantic constraints
    |
construct one complete, independently owned domain value
    |
verify store capacity and publication preconditions
    |
append the owned variant value
    |
return a type-specific slot identifier
```

The actual implementation checks capacity before expensive decoding; its **logical** publication condition remains the same. The store uses `std::vector<AssetRecord>`, so appending may allocate new storage and move earlier records. Why can the operation preserve previously published values if that allocation fails? All three variant alternatives are declared to have non-throwing move constructors with their chosen standard allocators, and the code verifies `std::is_nothrow_move_constructible_v<AssetRecord>`. Under that concrete contract, vector's reallocation does not need a potentially throwing move of the old elements. If allocation fails, the old vector remains. This reasoning must be revisited if the stored types, allocators, destruction contracts or container change; an unconditional slogan that every `vector::emplace_back` operation has the strong guarantee would be wrong.

The importer also distinguishes **source rejection** from **publication failure**. Bad magic, invalid triangle indices, unsupported mapper and dimension limits are properties of the incoming bytes. Capacity exhaustion, an explicitly injected failure and `std::bad_alloc` are operational failures. Treating all of these as one boolean “load failed” would make debugging, recovery policy and user-facing diagnostics worse. The C++23 `std::expected` result lets the caller handle anticipated failures without having to guess whether a partially published record exists.

The guarantee is intentionally about *one call* to `import<Tag>`. If a caller imports a texture successfully and then fails while importing a mesh, the texture remains published. The importer does not magically roll the successful earlier call back. A truly atomic multi-asset batch would require staging the entire batch, checking aggregate budgets with checked addition, reserving or otherwise securing publication resources, and committing the batch under a separate all-or-nothing policy. The earlier chapter already introduces parameter packs and checked fold expressions; this gives them a genuine future engineering purpose instead of another detached syntax exercise.

Nor does an append-only store give automatic thread safety. The code has no synchronization or reader epoch protocol, and a simultaneous read and vector reallocation would be a data race or invalid view. We must solve concurrent publication and reader ownership as distinct problems rather than calling a typed identifier a lock.

#### What we are actually proving with the C++23 laboratory

The new executable uses **synthetic inputs only** and runs without graphics drivers, ROM downloads or third-party format libraries. It builds a valid two-by-three RGBA8 image and proves that the twenty-four pixel bytes survive source-buffer destruction. It tests zero dimensions, oversized dimensions, wrong magic, nonzero reserved fields, truncated payloads and trailing payloads. A boundary fixture verifies that 512-by-512 RGBA8 is exactly one megabyte and is permitted by the specified policy. This does not benchmark texture uploads or demonstrate GPU residency.

The mesh tests construct three vertices and one triangle, then verify the signed fixed-point coordinate, index ordering and owned-storage behavior. The negative cases reject an out-of-range vertex index, an index count that is not a multiple of three, unsupported flags, wrong magic, truncated payload, trailing payload, too many vertices and an empty triangle list. A semantically invalid index is rejected after the structural length check, which demonstrates why format validity and domain validity are not identical.

The combined-store tests load a real NROM cartridge object through the shared factory, followed by a texture and a mesh. Deliberate wrong-kind lookup requests return no asset rather than reinterpreting a variant's bytes. Mapper-one and bad-magic cartridges are rejected without publication. We inject failures before staging, after staging, and a synthetic `std::bad_alloc` after successful staging to verify that no new record escapes. Then we append until the 256-record capacity is reached, allowing vector reallocation to occur while checking the old assets again using their stable append-only indices. Finally, one additional insertion must fail with a typed capacity error and no change in count.

The compile-time assertions complement these runtime tests. They establish that the three supported tags satisfy the concept, that an unsupported type does not, that a cartridge ID is not implicitly convertible into a texture ID, that `NromCartridge` is noncopyable, and that the variant record has a non-throwing move construction contract. These properties are necessary but not sufficient for full software safety. The test probes and the static checks constrain different parts of the system.

An especially useful next improvement is to record *which failure phase* was exercised and how many assertions or source regions were checked, rather than printing one vague success message. Property-oriented tests could generate many sizes, flags and index sets with a fixed seed. A complete production importer would additionally fuzz real standard formats and compare against independently implemented format oracles. For this pass we keep the fixed formats small enough that every accepted byte range can be derived by hand.

#### Why this matters beyond NES

A good engine's asset pipeline begins before anything reaches the GPU. It asks: what type is this resource, what is the external schema, which lengths are trustworthy, who owns the decoded result, which invariant must hold before publication, what does its identifier mean, and what happens if an operation fails? Those questions recur when loading an NES game, a texture tile, a mesh from a reconstruction pipeline, a ray-tracing acceleration-structure input, or a recording for an XR scene.

A renderer's texture upload path may later use Vulkan, Metal or another backend, but the source decoder's proof is still valuable: if width, height or pixel length is already wrong, no GPU API call can repair the missing semantic contract. Conversely, a correct CPU-side asset does not establish GPU synchronization or actual device-resident format compatibility. We will reconnect these boundaries in the graphics and engine books precisely when GPU memory and queues enter the narrative, using real projects rather than substituting superficial cross-language examples.

The same distinction applies to mesh reconstruction. The quantized triangle experiment has simple, validated indices. Maveb-like reconstruction uses considerably richer representations, confidence data, changing geometry, spatial indexes and sometimes multiple surface encodings. Their loaders need their own domain-specific constraints, but the broader stages of input validation, ownership, staging and publication can still be shared. A common architecture should make domain-specific requirements *visible*, not flatten them into one generic “blob loaded” flag.

#### Reconstruct and extend the system before passing this milestone

Start without opening the reference source. Derive the three file headers and the precise byte-offset formula for each payload. Explain why the texture's four bytes per pixel do not imply a GPU row pitch of four times its width, and why a mesh's 16-bit index is not a pointer or a vertex object. Implement one decoder for each format using explicit little-endian byte operations, checked length arithmetic and a domain-specific validation rule. Then express a trait family and C++23 concept that admits only those types. Construct a `variant`-based append-only store that accepts completed owned values and distinguishes typed source rejection from operational failures.

The reconstruction must demonstrate one successful import for every type, one rejected input for every distinct failure class, ownership independent from the source buffer, and an unchanged store after injected staging failure. Add a counterexample showing that a retained raw pointer can become stale on vector growth while an index lookup still works under the append-only rule; do not dereference the stale pointer in a runtime experiment. Explain why the next chapter needs a generational slot if assets will be deleted or replaced. Finally, describe what must change if we require an all-or-nothing batch, streaming input, cancellation, concurrent hot reload, or asynchronous GPU creation.

This is an engineering learning gate, not a page-count gimmick. The reader should be able to derive the format, explain the types, demonstrate that validation precedes publication, classify the failures, defend the lifetime conditions and reason through a counterexample. We retain every original explanation in the surrounding chapter and attach this coherent real-project pathway where the shared publication logic becomes necessary.

**Reference and scope boundaries.** The cartridge specialization still uses the [NESdev iNES format](https://www.nesdev.org/wiki/INES) and [NESdev NROM](https://www.nesdev.org/wiki/NROM) as external format/hardware references. The C++ language/library semantics can be checked against [std::variant](https://en.cppreference.com/w/cpp/utility/variant), [std::vector](https://en.cppreference.com/w/cpp/container/vector), [std::expected](https://en.cppreference.com/w/cpp/utility/expected) and [C++ constraints](https://eel.is/c++draft/temp.constr). The new `TXR4` and `MSH1` formats are original teaching fixtures defined in this chapter; there is no outside standard or existing Kairo/Maveb implementation being falsely attributed to them. No completed PNG/glTF importer, macOS graphics integration, GPU resource, full NES emulation, or real-time asynchronous asset subsystem is claimed.

**Next existing chapter:** “Grow Registries without Invalidating Identity” will address deletion, reuse and generation-aware handles on top of the same typed asset store, with separate policies for address stability, resource retirement and concurrent readers.
