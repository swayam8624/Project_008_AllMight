# 05 — Generic asset pipelines with stable identity

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/04 - Resource ownership, failure paths, and review practice|Previous chapter]] · [[Chapter Edition/03 - Game Engine/06 - Queries, lazy ranges, allocators, and module boundaries|Next chapter]]

Several resource kinds repeat the same loading shape, but their sizes, validation rules, and meanings differ. This chapter uses generic code and compile-time policy while keeping asset identity stable across container growth.

**By the end:** Choose which asset operations can be generic and which depend on semantic type, then implement identity that survives storage movement.

**Recall question:** How can one pipeline share mechanics across asset types without erasing their distinct validation and lifetime contracts?

<!-- Derived from Continuous Notes/Game Engine - From First Frame to Living Worlds.md; source lines 7081–8059; source-span SHA256 524d1ed59af9e706542d8675a9338678b7e4e6c308354a5cc6c6e0026c5cb191. -->

<!-- CHAPTER-SOURCE-BEGIN -->
## One asset pipeline, many asset types — generic code without accidental promises

The previous story followed a single upload from external bytes through validation, resource acquisition, private preparation and publication. It established what the loader must preserve when construction fails, who owns the native handle, and what a delayed callback may safely retain. But we now have a new practical problem: a game editor loads meshes, textures, shader descriptions and many other kinds of assets. They do not all contain the same fields or need the same validation. Yet every loader repeats the same bookkeeping: check capacity, construct a complete record, publish it only after construction succeeds, return an identifier and avoid leaking a partially prepared result.

Copying this bookkeeping into separate `MeshStore`, `TextureStore` and `ShaderStore` classes would make every later correction difficult. If we discover a failure in publication or a missing capacity check, we would have to repair several implementations and prove that they remain consistent. At the opposite extreme, using `void*` and manually interpreting bytes would throw away the type and lifetime safety we worked to establish. We need a middle path: **write the operation once, preserve the actual asset's type, and reject operations that make no sense for that type.**

This is where C++ templates become necessary. A template is not a runtime container for arbitrary objects. It is a declaration parameterized by types, values, or other template arguments; the compiler forms usable specializations when code needs them. A well-designed template lets us express a shared rule while still allowing the compiler to know whether it is operating on a `Mesh` or a `Texture<4>`. The price is that requirements, generated code, compilation cost, overload behavior and ownership guarantees must all remain explicit. We will derive them from our asset pipeline rather than presenting templates as a detached language reference.

**Working reference:** [[tools/labs/generic_asset_pipeline_lab.cpp|C++20 generic asset laboratory]]. The laboratory is a CPU-side asset preparation model with fixed optional slots, checked byte counts and deliberate failure injection. It is not a direct replacement for the Phase 2 native-resource owner, and it does not claim to execute GPU work. Our eventual engine joins the two layers: validated type-specific preparation first, a resource-owning backend transaction second.

### The repeated operation is a function whose input type can vary

Start with the question "How many source bytes does this asset represent?" A mesh stores a vector of 32-bit indices. A texture has a channel count and a texel count. For a mesh with three indices, the simplest byte-size calculation is $3\times\operatorname{sizeof}(\texttt{uint32\_t})$. On an implementation with a 32-bit exact-width unsigned integer type, this is $3\times4=12$ bytes. A four-channel texture with twelve texels needs $4\times12=48$ bytes if each stored channel is one byte; a three-channel texture with five texels needs $3\times5=15$ bytes. These calculations represent the source payload model, not GPU row padding, compression, mip chains or device allocation requirements.

We could define three differently named functions, but it would be better if an asset-aware operation could accept any supported type and use the correct byte-size rule. A **function template** expresses that shape:

```cpp
template <class T>
std::size_t asset_bytes(const T& asset) {
    return AssetTraits<T>::byte_size(asset);
}
```

The `template <class T>` declaration introduces a type parameter, meaning that `T` stands for a type selected when the function is used. Calling `asset_bytes(mesh)` does not require us to type `asset_bytes<Mesh>(mesh)` every time. During **template argument deduction**, the compiler compares the parameter pattern `const T&` with the argument expression and determines the appropriate `T`. It then checks whether the resulting specialization is a valid function. The expression `AssetTraits<T>::byte_size(asset)` is type-dependent, so its meaning is determined for the selected type rather than being a single runtime switch on an integer type code.

This is different from the preceding chapter's polymorphic `Backend`. A virtual backend can be chosen through a base pointer at runtime. Here the compiler already knows the asset type at a particular call site and can form type-specific operations. The actual machine code may be inlined or outlined, duplicated or merged, depending on compiler optimization and link behavior. A template is therefore not automatically a promise of zero instructions or one code copy for all types; its important initial benefit is that the *source rule* is written once while the selected types remain statically known.

One subtle point: deduction does not work by guessing whatever type would make the function body compile. It first follows parameter/argument deduction rules, then substitution and validity checks. An argument that is an lvalue, a const lvalue or a temporary can produce different deductions for a forwarding-reference parameter, while `const T&` usually deduces the unqualified object type for the common examples here. Some contexts, such as certain dependent nested type names, do not deduce template arguments at all. Braced initializer lists also have specific deduction rules and may fail where a named `std::vector` would succeed. We should never describe deduction as "C++ automatically figures out everything from the code"; it solves a constrained pattern-matching problem under precise rules.

### Different asset types share a store, but their meanings remain different

We now want one store class that retains a particular asset type without converting it into untyped memory. A **class template** is a family of types described by a common declaration. Consider:

```cpp
template <class T, std::size_t Capacity>
class AssetDepot {
    std::array<std::optional<T>, Capacity> slots_{};
    std::size_t count_ = 0;
    // ...
};
```

The type parameter `T` specifies the actual stored object type. The value parameter `Capacity` specifies a compile-time capacity. Unlike `T`, which is a type, `Capacity` is a **non-type template parameter**: a value that participates in defining a specialization. `AssetDepot<Mesh, 2>` and `AssetDepot<Texture<4>, 2>` are different class types. Changing the capacity also selects a different specialization; `AssetDepot<Mesh, 2>` and `AssetDepot<Mesh, 8>` are not interchangeable by ordinary class identity.

Why use a compile-time capacity in this reference laboratory? A fixed `std::array` of `std::optional` slots makes the publication invariant exceptionally clear. Before an insertion, the next slot is empty. We construct the proposed object there. Only when that construction succeeds do we increment the count of published records. If construction throws, the old published slots remain untouched. This isolates the language behavior we want to learn.

There are tradeoffs. A fixed array reserves storage for every optional slot inside the depot object, even when the store is empty. Large assets or large capacities can make that object expensive or inappropriate for stack placement; a heap-backed or paged container may be better in a real engine. Constructing `AssetDepot<Mesh, 2>` and `AssetDepot<Mesh, 8>` can also instantiate separate code, and callers cannot freely substitute one for the other. Static capacity is ideal for this *bounded demonstration*, not proof that production asset registries should never grow.

The `std::optional<T>` slot has a straightforward meaning: it either contains a live `T` object or does not. It therefore saves us from having to manually implement the earlier union's active-member tracking. We still owe the caller a clear identity and lifetime contract. The lab's `AssetId{slot}` identifies a slot only while the depot's append-only sequence remains alive. It has no generation counter, removal or reuse policy. **It is not the generational asset handle design required by a real ECS or hot-reloading engine.** If a real store deletes and reuses slots, it must add generation validation and an invalidation model rather than pretending this small example already supplies them.

### The size rule must vary with the asset's semantic type

The shared store needs a common question—"How many bytes does this asset represent?"—but not a common answer. A mesh counts indices. A texture multiplies channel count by texel count. We can separate the *policy for a type* from the shared storage operation through a class-template trait:

```cpp
template <class T>
struct AssetTraits;  // deliberately no general implementation

template <>
struct AssetTraits<Mesh> {
    static constexpr std::string_view label = "mesh";

    static std::size_t byte_size(const Mesh& mesh) {
        constexpr auto width = sizeof(std::uint32_t);
        if (mesh.indices.size() >
            std::numeric_limits<std::size_t>::max() / width)
            throw std::overflow_error("mesh size overflow");
        return mesh.indices.size() * width;
    }
};
```

The initial declaration says that `AssetTraits<T>` is a possible template family; it does **not** promise that every conceivable C++ type is supported. The `template <>` definition is an **explicit specialization** for precisely `Mesh`. It says that the shared asset protocol is meaningful for this type and supplies that type's operations. Unlike an instance-level virtual method, this policy is selected using the known type, not an object's dynamic identity.

For textures we have an entire family of related types. `Texture<3>` and `Texture<4>` carry different compile-time channel counts, but they share the same formula for byte count. Instead of writing a separate complete specialization for each channel count, use a **partial specialization**:

```cpp
template <std::size_t Channels>
struct AssetTraits<Texture<Channels>> {
    static constexpr std::string_view label = "texture";

    static std::size_t byte_size(const Texture<Channels>& texture) {
        if (texture.texel_count >
            std::numeric_limits<std::size_t>::max() / Channels)
            throw std::overflow_error("texture size overflow");
        return texture.texel_count * Channels;
    }
};
```

This matches any member of the `Texture<Channels>` type family, subject to the texture type's own permitted channel counts. The compiler determines `Channels` from the selected `Texture` type. The multiplication must still be checked at runtime, because `texel_count` comes from data and may be too large. A compile-time channel count does not remove the need for input validation.

**Explicit versus partial specialization** is a difference in how much of the template argument pattern we fix. `AssetTraits<Mesh>` fixes the complete concrete type. `AssetTraits<Texture<Channels>>` fixes the outer type family while leaving the channel count parameterized. Function templates cannot be partially specialized in the same way as class templates; for function behavior differences we commonly use overloading, constrained templates, delegation to specialized traits, or other supported design techniques. Several partial specializations can also overlap, which creates an additional ordering problem; a design must guarantee a unique most-specialized match for the types it intends to accept.

This division avoids an implementation trap. A generic store should know how to **hold, construct and publish** an asset; a trait should explain which semantic operations a supported type provides. Neither should pretend the source-byte count automatically equals device allocation size, GPU row pitch or a serialized format's total length. Those are separate policies with their own units and constraints.

### A resource store must preserve the caller's intention to copy or transfer

Our Phase 2 owner had explicit move operations. If a generic store takes an asset's constructor arguments by value, it can create unnecessary intermediate copies. If it indiscriminately applies `std::move` to everything, it can steal storage from an argument that the caller intended to retain. We need to preserve whether the original argument is an lvalue or an rvalue.

First recall the value categories through the mesh's two constructors:

```cpp
struct Mesh {
    std::vector<std::uint32_t> indices;
    bool copied_input;

    explicit Mesh(const std::vector<std::uint32_t>& input)
        : indices(input), copied_input(true) {}

    explicit Mesh(std::vector<std::uint32_t>&& input)
        : indices(std::move(input)), copied_input(false) {}
};
```

A named `std::vector` passed as an lvalue selects the borrowing `const&` constructor and makes an independent copy of the indices. A temporary or an explicitly moved vector can select the `&&` overload, allowing the vector's resources to be transferred into the mesh. The `std::move` expression itself is a cast that makes an rvalue access path available; the actual resource transfer happens in the selected constructor or assignment operation. After a move, the original vector remains valid but its exact contents are not generally a promise our algorithm should depend on.

Now consider a template written with `T&&`. **It is not always an rvalue-reference parameter** in the simple sense. When `T` is deduced for a suitable function-template parameter, `T&&` is a *forwarding reference*. Passing an lvalue of type `U` deduces `T=U&`; passing an rvalue usually deduces `T=U`. That leads us to **reference collapsing**, the rules that simplify references formed through type aliases and template substitution:

| Inner reference | Outer reference | Collapsed result |
|---|---|---|
| `U&` | `&` | `U&` |
| `U&` | `&&` | `U&` |
| `U&&` | `&` | `U&` |
| `U&&` | `&&` | `U&&` |

In ordinary language, if either side is an lvalue reference, the collapsed result is an lvalue reference; only two rvalue-reference layers collapse to an rvalue reference. This preserves a fundamental promise: an expression supplied as a named, retained object does not suddenly become permission to move from it merely because it crosses a generic function boundary. By contrast, a parameter of the form `const T&&` is **not** the same forwarding-reference pattern, and a type parameter already fixed by a class template may not be deduced in the way this function-template pattern requires.

Our insertion method uses both a **variadic template parameter pack** and forwarding references:

```cpp
template <class... Args>
    requires std::constructible_from<T, Args&&...>
AssetId emplace(Args&&... args) {
    if (count_ == Capacity)
        throw std::length_error("asset depot full");

    slots_[count_].emplace(std::forward<Args>(args)...);
    return AssetId{count_++};
}
```

The symbols `Args...` mean a sequence of zero or more template type arguments, not one special runtime type. The function receives one forwarding-reference parameter corresponding to each deduced type. `std::forward<Args>(args)` uses that deduction to produce an access expression with the same effective value category as the original caller-supplied argument. The `...` after the forwarding expression expands the pack into the arguments for `optional<T>::emplace`. This means the store can call whatever `T` constructor is valid for those argument types, without manually duplicating constructor overloads in `AssetDepot`.

**Perfect forwarding** is the common name for this preservation mechanism. The word "perfect" should not be mistaken for guaranteed zero cost, zero allocations or universally correct semantics. It preserves argument categories *subject to C++ deduction rules*; the target constructor may still allocate or copy, and some arguments have special deduction behavior. It also preserves dangerous borrows if the target object stores references to short-lived data. The target type's constructor is still responsible for adopting or copying any state it retains.

A fully worked trace makes this concrete. The lab constructs `std::vector<std::uint32_t> input{3,7,9}`. Calling `meshes.emplace(input)` deduces an lvalue reference type for its one argument, forwards it as an lvalue, and selects `Mesh(const vector&)`. The mesh records `copied_input=true`, while the source vector remains usable with three elements. Calling `meshes.emplace(std::move(input))` treats the argument as an rvalue, forwards it accordingly and selects `Mesh(vector&&)`. The second mesh records `copied_input=false` and contains three indices. We deliberately do not assert that the moved-from input vector must be empty: **its exact moved-from state is not the same portable guarantee as the destination's correct contents.**

This trace is evidence about constructor selection and ownership on the tested type. It is not an assertion that moving every asset is cheaper than copying, or that a forwarded temporary can safely be retained by a target that stores an unchecked reference.

### A collection of assets makes parameter packs and fold expressions necessary

The editor may submit several kinds of resources in one batch. We want the total source-byte count before deciding whether a batch fits an upload budget. A fixed function accepting one mesh and two texture arguments would not scale to other combinations. We already have `AssetTraits<T>::byte_size` for each supported type; we need a way to repeat that operation for every asset in an arbitrary argument list.

A **parameter pack** represents the type and value arguments of this variable-length compile-time input. A **fold expression** combines them using a chosen operator. For a pure addition, one might write `(std::size_t{0} + ... + asset_bytes(assets))`. But this omits a real correctness condition: accumulating large `std::size_t` values can wrap because it is unsigned, quietly producing a smaller total. Source-byte accounting that determines whether an allocation fits must not ignore overflow.

The tested lab therefore folds over a checked addition operation:

```cpp
template <Uploadable... T>
std::size_t total_bytes(const T&... assets) {
    std::size_t result = 0;
    [[maybe_unused]] const auto add_checked = [&result](std::size_t next) {
        if (next > std::numeric_limits<std::size_t>::max() - result)
            throw std::overflow_error("batch size overflow");
        result += next;
    };

    (add_checked(asset_bytes(assets)), ...);
    return result;
}
```

The `(expression, ...)` form is a **unary right fold over the comma operator**. For the built-in comma behavior selected here, evaluations proceed in the specified left-to-right order: determine the current asset size, validate its addition against the remaining numeric range, then add it. With three assets of 12, 48 and 15 bytes, the intermediate totals are 12, 60 and 75 bytes. That trace should be reproducible without running the program. The result is not 75 kilobytes and not an allocated GPU buffer size. It is exactly 75 source payload bytes under the demonstrated type-specific formulas.

The empty pack matters. A unary comma fold is permitted with no operands and has `void()` as its identity, so the function can return the initialized zero. In our initial strict GCC test, however, the local checked-addition lambda was then defined but unused, triggering a warning elevated to a compilation error by `-Werror`. The final lab explicitly marks it `[[maybe_unused]]`, recognizing that the empty-pack specialization does not need a callable. This is a useful example of real template instantiation behavior: source that works when instantiated with three arguments can fail under a different pack size or compiler diagnostics. The zero-argument path is part of the contract and therefore must be compiled and tested.

Fold expressions are not universally evaluated left-to-right. Their grouping and operand evaluation follow the particular operator's rules; for many operators, grouping does not imply the execution order of arbitrary side effects. We intentionally use a built-in comma fold because sequencing is an important requirement here. A production batch planner might instead return a checked result type, distinguish overflow from unsupported asset type, and enforce hardware-specific alignment separately. A fold does not remove any of those requirements.

### A generic function still needs a boundary describing which types qualify

The traits model is useful only if unsupported types fail early and understandably. Without a public constraint, a caller can instantiate `asset_bytes(UnknownAsset{})` and receive an error somewhere inside a dependent function body. We would prefer the interface to state what a supported asset must provide.

Historically, C++ libraries often used **SFINAE**—*Substitution Failure Is Not An Error*—for this purpose. The phrase has a precise scope. During template argument substitution in certain immediate contexts, an invalid substitution can remove a candidate from consideration rather than producing an immediate hard error. It does not mean arbitrary invalid code in a template body is harmless. It also does not make a runtime bad input acceptable.

Here is the lab's representative detection trait:

```cpp
template <class T, class = void>
struct HasByteSize : std::false_type {};

template <class T>
struct HasByteSize<T, std::void_t<decltype(
    AssetTraits<T>::byte_size(std::declval<const T&>())
)>> : std::true_type {};
```

The primary definition reports false. The partial specialization is considered when the expression inside `decltype` is valid during substitution. `std::declval<const T&>()` supplies a type-level expression in an unevaluated context; it does not construct a real `T`. `decltype` asks about an expression's type, not its runtime result. If the expression is invalid for `UnknownAsset`, the specialization is discarded and the primary false trait remains. If it is well-formed for `Mesh` or `Texture<4>`, the specialization reports true.

The important limitation is that this trait checks whether a particular **expression is well-formed**. It does not check whether a size calculation is numerically correct or whether a device can accept the resulting buffer. The trait does not execute the function body, so a function that always throws could still satisfy the detection rule. Nor does this form necessarily produce the clearest diagnostics for a new contributor. C++20 **concepts** and **requires-expressions** make the public requirement far easier to read.

### Write the contract in the type system at the point where it is needed

Our asset protocol requires two things: a readable type-specific label and a byte-size expression producing `std::size_t`. We can state exactly that as a **concept**, a named compile-time predicate:

```cpp
template <class T>
concept Uploadable = requires(const T& asset) {
    { AssetTraits<T>::label } -> std::convertible_to<std::string_view>;
    { AssetTraits<T>::byte_size(asset) } -> std::same_as<std::size_t>;
};
```

A **requires-expression** checks expressions and type relationships for well-formedness under its parameter declarations. The braces around a compound requirement permit a return-type requirement introduced by `->`. The labels `std::convertible_to<std::string_view>` and `std::same_as<std::size_t>` are themselves standard-library concepts. The latter expresses an exact type requirement for the declared byte-size expression; a size function returning `int` is not accepted as an exact match merely because an integer can be converted to `std::size_t`. The name `Uploadable` means "has the static interface required by this laboratory", not "this object is guaranteed safe to upload".

Critically, the `requires` expression examines the expression's validity **without executing a real upload or checking data-dependent values**. A `Texture<4>` with an impossibly large texel count can still satisfy the concept. The call to `byte_size` must later reject arithmetic overflow at runtime, which our lab explicitly tests. Concept satisfaction is a **compile-time structural contract**; semantic truth and runtime preconditions require further evidence.

The laboratory includes:

```cpp
static_assert(Uploadable<Mesh>);
static_assert(Uploadable<Texture<3>>);
static_assert(Uploadable<Texture<4>>);
static_assert(!Uploadable<UnknownAsset>);
```

These check that the declarations and trait specializations match our intended boundary. The template definition also constrains the store itself:

```cpp
template <Uploadable T, std::size_t Capacity>
class AssetDepot {
    // Only types meeting Uploadable can form usable specializations here.
};
```

The concise `template <Uploadable T>` form abbreviates a constraint on `T`; it does not transform the chosen asset into a new base class or inject a virtual interface. Types remain concrete. The API can then add a more specific **requires-clause** for an operation that constructs them:

```cpp
template <class... Args>
    requires std::constructible_from<T, Args&&...>
AssetId emplace(Args&&... args);
```

This says: not every supported asset is constructible from every possible argument list. A mesh might accept a vector of indices; it should not silently accept an unrelated integer and guess what it means. Constraint checking can reject the invalid call during overload resolution. The lab goes further: it defines a `CanEmplace` concept that tests the actual expression against a store type, proving both vector-lvalue and vector-rvalue calls are available and an arbitrary integer call is not.

There are subtle boundaries here. A requires-clause can be written after the template parameter list or after a function declarator; concepts can also constrain abbreviated function templates. The compiler normalizes constraints to compare them for overload selection, and semantically similar Boolean expressions are not automatically interchangeable for that comparison. Reusing a **named concept** creates a stable requirement vocabulary and often makes constrained overload ordering more predictable than independently restating complicated conditions.

### Why two overloads can share a name without creating a runtime type switch

The editor wants a diagnostic route function that produces a useful label for recognized assets and a fallback for anything else. One unconstrained function template can say "unsupported"; a second, otherwise similarly shaped function template can require `Uploadable` and use the trait:

```cpp
template <class T>
constexpr std::string_view route(const T&) {
    return "unsupported";
}

template <Uploadable T>
constexpr std::string_view route(const T&) {
    return AssetTraits<T>::label;
}
```

Both overloads are candidates for a `Texture<4>` value, but the constrained overload is more specialized under the relevant rules, so it produces `"texture"`. For `UnknownAsset`, the constrained overload's requirement is not satisfied, leaving the fallback `"unsupported"`. This is resolved at compile time for the statically known argument type; it is not selecting a derived object's virtual method at runtime.

**Constrained overload resolution** uses ordinary viability and conversion rules together with template partial ordering and constraint subsumption. Constraint subsumption is not a general theorem prover. In particular, two independently written expressions that happen to mean the same thing to a mathematician may not count as the same atomic constraint under the language's identity rules. Writing `requires(N > 0)` and `requires(N >= 1)` does not establish a universal compiler-recognized ordering relationship simply because integers make the statements logically related. The precise normalization and identity rules matter. Named concepts help organize reusable constraints, but one must still test potentially overlapping overload sets for ambiguity.

The important design insight is that **specialization chooses the type-specific asset rule**, while **a constrained overload chooses which static interface may be called**. The public store API therefore has a predictable and explainable set of permitted operations. It does not accept an arbitrary pointer and hope a cast will work at runtime.

### Publication still needs a correctness proof after introducing templates

A generic container can be perfectly well-typed and still violate the exception guarantee established in Phase 2. We must therefore prove the new store's publication logic with the same care as its compile-time API. The reference store retains `std::array<std::optional<T>, Capacity>` and an integer count of published entries. Its insertion is:

```cpp
if (count_ == Capacity)
    throw std::length_error("asset depot full");

slots_[count_].emplace(std::forward<Args>(args)...);
return AssetId{count_++};
```

This sequence has a deliberate order. The capacity check happens before construction. The next slot is unoccupied by the store invariant. `optional<T>::emplace` constructs the candidate object in the currently unpublished slot; if the constructor throws, no new completed asset is published and `count_` remains unchanged. Only after construction returns successfully does `count_++` add the slot to the published prefix. With this append-only design, the earlier slots remain unaffected. There is no vector reallocation in this particular storage layout, so the strong-state argument differs from Phase 2's `std::vector` proof.

Let `c` be the number of published entries before the call and `C` the static capacity. We maintain the invariant:

\[
0\le c\le C,\qquad
\text{slots }[0,c)\text{ are engaged},\qquad
\text{slots }[c,C)\text{ are disengaged}.
\]

If `c=C`, the function rejects the insertion and changes nothing. If `c<C` and the proposed construction throws, the next optional remains disengaged and the published count stays `c`. If construction succeeds, the new slot is engaged, and incrementing the count makes the published interval `[0,c+1)`. This is a short, inspectable correctness proof that depends on the container's real semantics, not merely the word "generic".

The lab checks a `CheckedShader` whose constructor throws for a nonpositive version number. The first shader (version 3) is published in slot zero. The attempt to emplace version −1 throws; the store still reports one published asset, and the first shader remains valid. Emplacing version 4 then successfully reuses the *still-unpublished* slot one. The mesh store also tests capacity overflow: once its two slots are full, an extra insertion throws before touching existing assets.

This code does **not** implement removal or recycled slots. An asset ID can be used only while its depot is alive and while the append-only invariant holds. There is no thread synchronization: concurrent publication or access is not part of the proven contract. And `optional<T>` does not give external GPU resources a completion fence. It constructs and destroys C++ values under ordinary language rules; a resource-owning value stored inside it still needs a correct backend release protocol.

### The real performance question is what gets instantiated, stored and moved

Generic code is often described as a "zero-cost abstraction". That phrase is meaningful only when we specify *relative to which baseline, under which workload, and which cost*. A function template instantiated for `Mesh` and `Texture<4>` can produce distinct specialized functions. The compiler may inline them, use the known channel count to simplify multiplication or combine equivalent machine code. It may also produce more binary code, longer compilation times and different instruction-cache behavior. The C++ language does not promise that a particular template call becomes one machine instruction or that a distinct instantiation always occupies distinct final machine-code bytes.

The fixed `AssetDepot<T,Capacity>` stores `Capacity` optional slots inline. That means its memory cost grows with both the number of slots and the size/alignment of `T` plus any representation overhead needed by the optional state. The exact size is implementation-dependent; writing `Capacity*sizeof(T)` as an equality would be misleading. Heap allocations inside `T`, such as a mesh's vector storage, remain separate from the fixed depot slots. A reference obtained from a slot can remain at a stable address during subsequent successful appends, but it still dangles if the depot itself dies or moves in a way that invalidates those references. Static capacity does not make lifetime management disappear.

A production comparison would create equally capable versions using fixed optional slots, a growing vector, and a paged/generational registry, then measure construction success/failure, byte footprint, allocations, iteration locality, compile time and hot-loop behavior. A runtime polymorphic asset wrapper may be necessary when a collection must contain *unrelated asset types together under one dynamic interface*. A `variant` may be appropriate for a closed set of heterogeneous values. A class template is appropriate when callers statically know the stored `T`. These mechanisms solve different problems; selecting them by slogan would undo the work of understanding their contracts.

The first meaningful research-style experiment from this chapter would be to prepare thousands of mesh-like and texture-like records under different size distributions, inject constructors that fail at controlled points, measure publication invariants and allocations, and compare the generic implementation with a hand-written concrete baseline. Another would measure whether compile-time channel counts result in improved throughput after compiler optimization and whether the additional specializations enlarge the binary enough to affect instruction-cache behavior. Such outcomes must be measured, not invented from template syntax.

### One complete trace and a reconstruction challenge

The tested program constructs a two-slot mesh depot. It starts with indices `{3,7,9}` and inserts them first as an lvalue and then as a moved rvalue. The first mesh records that it copied; the second records that it received the move path. Both contain the expected index count. A third insertion is rejected because capacity is exhausted, and the published mesh count remains two. A shader depot accepts version 3, rejects version −1 without publishing, then accepts version 4 into its next slot. Next the program constructs a four-channel texture of 12 texels and a three-channel texture of 5 texels. Their byte sizes are 48 and 15 respectively; the three-index mesh contributes 12. The checked heterogeneous batch function produces **75 bytes**, and its empty-pack case produces zero. `route` chooses `"mesh"` and `"texture"` for supported types and `"unsupported"` for an unknown type. Finally, a deliberately enormous texture count triggers the overflow check before the multiplication could wrap.

The entire trace is covered by assertions that stay active in a release-style test build, with compile-time assertions guarding unsupported types and constructor argument patterns. The reference program has been compiled with GCC 14.2 and address/undefined-behavior sanitizers, Clang 17 with warnings-as-errors, and GCC with `-O2 -DNDEBUG`. Each run reported `generic_asset_pipeline_lab: all checks passed`. The source committed to this repository was compared byte-for-byte against the compiled source using Git blob SHA `e673da4c89e50114b3e49c5b1b32dda7bf019ec8`. This is concrete evidence for the standalone C++20 model, **not** a source audit of Kairo, a real GPU pipeline, a multithreaded asset registry, or exhaustive fault injection.

Build it yourself from the repository root:

```sh
g++ -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  tools/labs/generic_asset_pipeline_lab.cpp -o /tmp/generic_asset_pipeline_lab
/tmp/generic_asset_pipeline_lab
```

To reconstruct it from memory, first explain why copying three independent stores is a maintenance problem but replacing them with raw `void*` is a type and lifetime problem. Build a function template that deduces an asset type, then a class template whose non-type capacity parameter determines the slot array. Write a full trait specialization for `Mesh` and a partial specialization for `Texture<Channels>`. From first principles, derive the four reference-collapsing cases; show why a named vector chooses the copy constructor and a forwarded rvalue chooses the transfer path. Implement a variadic batch-size function with overflow checks and an empty-pack result, then explain the fold's sequencing. Replace a SFINAE detection trait with a named concept and a requires-expression, constrain store construction and prove the fallback/constrained overload selection. Finally, trace a throwing constructor through the unoccupied slot to show exactly why previously published values remain untouched.

The original source contract calls this **M013, Parts 201–216**. The chapter is *authored reconstruction*, not a received original M013 transcript; the source ledger maps each part to its place in the story and its specific evidence. [[Supplementary/Foundations#M013 — Parts 201–216: generic asset preparation with constrained APIs|M013 coverage map]] · [[Visual Recall/02 - C++ - Objects, Ownership and Lifetime - Visual Recall#One asset pipeline, many types — M013 retrieval|reconstruction board]]. We have not inspected current `KairoMath/Matrix.cppm` or `KairoECS/SparseSet.cppm` revisions, which are original curriculum audit targets rather than proven facts about external repositories.

## A texture tile that chooses its work before the frame begins

The generic asset pipeline is now able to prepare meshes, textures, and shader records without copying the entire storage implementation for each type. An `AssetTraits` specialization describes the type-specific facts; a constrained `AssetDepot` constructs its selected `T` only after the argument types are validated; and a published prefix remains unchanged when the next constructor fails. That gives us a good type system and a readable ownership contract, but a renderer faces an additional question that occurs thousands or millions of times: **which decisions can be settled while we build the program, and which decisions must still happen for every resource or frame?**

Consider one practical operation. A texture arrives in a known pixel format. We need to determine its channel count, check the supplied dimensions, estimate the number of bytes, decide whether the tile fits a budget, perhaps compute a small spatial-addressing key, and record a work plan. If the format and the validation policy are compile-time choices, repeatedly asking what the format is during every pixel operation seems unnecessary. But if the width, height, source bytes, and runtime memory budget come from the asset or user, treating those as predetermined compile-time constants would be a mistake.

The solution is not “move everything into templates.” We must split the **stable configuration known to the compiler** from **the values that arrive while the program runs**, while preserving correct checks at their respective boundaries. That will naturally bring us to type traits, compile-time functions, policy types, static dispatch, lookup tables, and memory layout. The laboratory for this chapter is [[tools/labs/compile_time_asset_policy_lab.cpp|a standalone C++20 tile-planning program]]. It plans CPU-side work and validates an optional source payload; it does not pretend that producing a work plan has allocated GPU memory or submitted a Metal/Vulkan command.

### The pixel format is not an integer whose meaning we must rediscover

Suppose the tile is an 8-by-2 image with four 8-bit channels per pixel. It has \(8\times 2=16\) pixels. Each pixel contributes four source bytes, so its uncompressed, tightly packed payload has \(16\times4=64\) bytes. For an RGB image with four rows and four columns, the source payload has \(4\times4\times3=48\) bytes. These equations arise from the representation we have chosen: width and height count pixels, while `channels` counts 8-bit source components per pixel. They do **not** automatically give the amount of device memory after alignment, row-pitch requirements, mipmaps, block compression, or backend-specific allocation overhead.

Our asset type already knows its format. We therefore define a finite domain:

```cpp
enum class PixelFormat { R8, RGB8, RGBA8 };

template <PixelFormat Format>
struct PixelTraits; // intentionally no definition for arbitrary values

template <>
struct PixelTraits<PixelFormat::R8>
    : std::integral_constant<std::size_t, 1> {};

template <>
struct PixelTraits<PixelFormat::RGB8>
    : std::integral_constant<std::size_t, 3> {};

template <>
struct PixelTraits<PixelFormat::RGBA8>
    : std::integral_constant<std::size_t, 4> {};
```

We already met full and partial specializations when the mesh and texture types required different size rules. The new idea is that the pixel-format trait **inherits from a type that represents a compile-time constant**. `std::integral_constant<T,V>` is a class template whose `value` is the constant `V` of type `T`. It is often used to convert a compile-time fact into an object-like type that generic machinery can inspect. `std::true_type` and `std::false_type` are familiar instances of this pattern with Boolean values. For example, `PixelTraits<PixelFormat::RGBA8>::value` is the compile-time integer four.

Why make a value available as a type rather than simply store four in a field? Because the compiler can choose definitions and overloads using the type. It can evaluate bounds and compile-time assertions before execution, and it can form an operation specialized to the exact selected format. **Type traits** are the general category of compile-time questions about types or their associated properties: is a policy class empty? Is this resource wrapper nothrow movable? Is this asset type trivially copyable? Do two types denote the same type? The standard library answers many such questions through traits such as `std::is_empty_v<T>`, `std::is_trivially_copyable_v<T>`, and `std::is_same_v<T,U>`. The suffix `_v` is the convenient value form, generally based on the corresponding class template's `::value`.

These traits are facts about the **specified C++ type property**, not magical correctness checks. `std::is_empty_v<Policy>` cannot prove the policy is thread-safe or that it leaves validation unchanged. `std::is_trivially_copyable_v<T>` cannot prove that `T` has a stable wire format, as the preceding chapter demonstrated. Our format traits also do not validate a corrupt external image: they describe the chosen *internal model*. We must still verify file metadata, decode external formats, and reject unsupported values before forming an internal typed description.

The lab uses compile-time assertions to confirm that the RGB trait supplies three and that the RGBA trait derives from `std::integral_constant<std::size_t,4>`. This is a small but important transition: an external pixel count is data, while the chosen internal format's channel count is part of the program's type-level contract.

### The renderer asks whether one branch can disappear completely

Now consider a tile-planning function which wants to mark whether a format includes an alpha component. It could compare a runtime enum on every call. But the function already has `Format` as a template parameter. We can use:

```cpp
bool alpha = false;

if constexpr (Format == PixelFormat::RGBA8) {
    alpha = true;
}
```

The expression in `if constexpr` is a compile-time condition. For an instantiation with `Format == PixelFormat::RGBA8`, the selected branch is kept; for a different format the discarded statement is not instantiated in the same way as ordinary runtime control flow. This is especially useful when branches contain type-dependent operations that are well-formed only for certain types. A regular `if` requires both branches to be valid C++ for the instantiated function, even if a runtime condition always happens to be false. `if constexpr` can remove the need to compile a type-dependent discarded branch once the condition is fixed.

There are boundaries. `if constexpr` is not a preprocessor switch that arbitrarily hides invalid non-dependent code. Outside templates, the compiler still checks many forms of invalid code even in a branch that would be discarded; inside a template, names and expressions have dependent versus non-dependent validity rules. The selected branch can still perform runtime work. And an ordinary compiler may optimize a constant regular `if` to the same machine code anyway. We choose `if constexpr` primarily to **express an intended compile-time semantic distinction**, not to claim a guaranteed speedup without examining generated code.

In the tested program, the RGBA specialization reports `alpha=true` and the RGB specialization reports `alpha=false`. The tile's width and height are still runtime arguments. They retain their own checks, even though the channel count is fixed. A compile-time choice about **how** an operation works does not make the operation's **input values** valid.

### Some calculations can be used at compile time without being forced there

Before allocating or planning, the renderer multiplies dimensions. A naive `width * height * channels` may overflow `std::size_t` and wrap modulo its unsigned range. That is dangerous for allocation and bounds decisions. We need a function which is useful both when values are known at compile time and when they arrive from a file at runtime:

```cpp
constexpr std::size_t checked_product(std::size_t x, std::size_t y) {
    if (y != 0 && x > std::numeric_limits<std::size_t>::max() / y)
        throw std::overflow_error("byte-count multiplication overflow");
    return x * y;
}

static_assert(checked_product(8, 4) == 32);
```

The word `constexpr` means this function is **eligible** for evaluation in a constant-expression context when the necessary conditions hold. It does not say every invocation executes during compilation. In the static assertion, `checked_product(8,4)` is evaluated as a constant expression. When we later call `checked_product(runtime_width,runtime_height)`, the same source function may execute at runtime. A compiler may also optimize a runtime call using ordinary constant propagation where it has sufficient information; that optimization is distinct from the language-level requirement to produce a constant expression.

Why is the overflow check written as a division? Multiplying first would already lose the information we needed to detect overflow. For nonnegative unsigned numbers \(x,y\), a product fits in a maximum representable value \(M\) exactly when \(x\le M/y\) for \(y\ne0\). We therefore test the inequality before multiplication. The `y == 0` case needs separate handling to avoid division by zero; multiplying by zero is safe and yields zero. In the tile pipeline, we apply this function once to `width * height` and again to `pixels * channels`. This establishes a correct *count arithmetic* invariant, although it does not establish enough available memory or any GPU-specific alignment guarantee.

At runtime, the laboratory asks for a tile with width `SIZE_MAX` and height two. The first multiplication check throws before computing an overflowing product. The policy and recorder are not invoked after that failure, so the rejected request does not appear as a planned tile. In a compile-time constant-evaluation context, a call that attempts to evaluate the throwing path cannot produce the required constant expression; the program must reject its use as a compile-time constant. This is not an excuse to rely on exceptions in a real-time hot loop: the public engine may instead return a typed error, as Phase 2 developed. The function illustrates the distinction between the *arithmetic rule* and the *chosen error-delivery policy*.

### Configuration that must be known before the program executes

The renderer may choose a supported tile shape at build time, such as an 8-by-8 block. That choice could determine constant buffers, threadgroup dimensions or a compile-time lookup table in a backend-specific implementation. A normal `constexpr` function may be called at runtime, which is useful for arithmetic but not quite the contract we want for an immutable build configuration. C++20 provides `consteval`:

```cpp
struct TileDefinition {
    std::size_t width, height;
};

consteval TileDefinition make_tile_definition(std::size_t width,
                                               std::size_t height) {
    if (width == 0 || height == 0 || width > 64 || height > 64)
        throw "tile dimensions must be in [1, 64]";
    return {width, height};
}

inline constexpr TileDefinition kDefaultTile =
    make_tile_definition(8, 8);
```

A `consteval` function is an **immediate function**. Its potentially evaluated calls must satisfy the language's immediate-invocation rules and produce a constant expression where required. The invalid-dimension path has no acceptable constant result. A call such as `make_tile_definition(runtime_width,runtime_height)` in an ordinary runtime context is not simply another runtime validation operation; it is ill-formed if the argument values cannot satisfy the constant-evaluation requirement. That difference is the point of writing `consteval`.

The returned `TileDefinition` is stored in an `inline constexpr` variable, whose value is available as a constant expression. The `inline` specifier here supports coherent definitions in headers across translation units according to the language's rules; it does **not** mean the variable must be physically duplicated in every executable object or that the compiler has to perform a particular inlining optimization. The lab checks that the chosen tile holds 64 pixels. A caller-provided tile dimension still needs **runtime** validation, as shown in `TilePlanner::prepare`; it would be wrong to pretend that validating an 8-by-8 default validates every image loaded by the user.

Three terms are now distinguishable. `constexpr` enables constant evaluation of functions and designates qualifying variables as constant-expression values. `consteval` requires immediate compile-time evaluation for the function's relevant calls. `constinit`, which we will need for a global diagnostic counter, guarantees **static or thread storage duration initialization is constant-initialization**; it does not make the resulting variable immutable. Those are different contracts, even though their names look similar.

### Why precompute a spatial-address lookup instead of rebuilding it?

The same engine that loads textures may divide images into small tiles and assign compact keys to neighboring positions. One common ordering scheme is **Morton order**, also called Z-order, which interleaves the binary digits of two coordinates. It is useful because coordinates that are near one another often receive nearby keys, but it does not guarantee perfect spatial locality or replace a complete spatial index.

Take a four-bit coordinate \(x\). Write its bits as \(x_3x_2x_1x_0\). To make room for a second coordinate, place each bit at an even position:

\[
\operatorname{spread}(x)
=\sum_{i=0}^{3} \left(\left\lfloor\frac{x}{2^i}\right\rfloor\bmod 2\right)2^{2i}.
\]

The division and remainder here simply ask for bit \(i\); in C++ we use a right shift and bitwise AND. For \(x=3\), binary `0011`, the low two bits are set, so spreading produces `00000101`, decimal five. For \(y=5\), binary `0101`, spreading produces `00010001`, decimal seventeen. Shift the spread \(y\) bits left by one and combine:

\[
\operatorname{morton}(3,5)
=\operatorname{spread}(3)\;|\;
\big(\operatorname{spread}(5)\ll 1\big)
=5\;|\;(17\ll1)
=5\;|\;34
=39.
\]

The bitwise OR is correct here because one spread result occupies even bit positions and the other, after shifting, occupies odd bit positions; there is no overlapping set bit to carry. If the coordinates are outside the range zero through fifteen, the four-bit function cannot represent them and must reject them or use a wider scheme. This small example is **one four-bit-per-axis tile key**, not a complete texture layout for arbitrary image dimensions.

We can write the bit-spreading calculation once as a `constexpr` function and generate all sixteen possible values at compile time:

```cpp
constexpr std::uint8_t spread4(std::uint8_t n) noexcept {
    std::uint8_t result = 0;
    for (unsigned bit = 0; bit != 4; ++bit) {
        result |= static_cast<std::uint8_t>(
            ((n >> bit) & 1u) << (2u * bit)
        );
    }
    return result;
}

consteval std::array<std::uint8_t, 16> make_morton_table() {
    std::array<std::uint8_t, 16> table{};
    for (std::size_t n = 0; n < table.size(); ++n)
        table[n] = spread4(static_cast<std::uint8_t>(n));
    return table;
}

inline constexpr auto kSpread4 = make_morton_table();
static_assert(kSpread4[3] == 5 && kSpread4[5] == 17);
```

This is a **compile-time lookup table**. The expensive-to-explain transformation is fully specified, and each possible four-bit input has an entry known before runtime. The actual function `morton4(x,y)` validates the two coordinates, reads their table entries and interleaves them. The lab checks the derived result `morton4(3,5)==39` both at compile time and during execution.

A table is not automatically faster than direct bit operations. On some processors, a handful of shifts and masks can be cheaper than additional memory loads; for other workloads a table may be beneficial, especially if it remains in cache. A wider table increases data footprint, which can harm locality. On the GPU, wavefront execution, cache layout and address calculation can change the answer again. The purpose here is to show *when compile-time precomputation becomes available* and how to validate its meaning. A fair performance experiment must compare these algorithms on the same machine and workload, not assume the presence of `constexpr` guarantees a speedup.

### A global counter illustrates a different constant-initialization promise

During testing, we want to count plans that reached the recorder. A global variable can have **static storage duration**, and it may be accessed long after the particular planner object was constructed. But we do not want its initialization to depend on dynamic initialization performed by another translation unit. We declare:

```cpp
constinit std::uint32_t g_plans_recorded = 0;
```

This variable is initialized in the static-initialization phase under the `constinit` rules. The compiler rejects an initializer that cannot meet the required constant-initialization contract. Unlike `constexpr`, `constinit` does not require the variable to be const. In the single-threaded lab we increment it after successful recorder calls and verify the final value two.

This does **not** make a global mutable counter good architecture. It is neither automatically atomic nor a synchronization primitive. If multiple worker threads incremented it without synchronization, they could create a data race. A production diagnostic counter would likely use an appropriate atomic or aggregate per-thread counts and combine them under a defined memory ordering and reporting policy. `constinit` solves an **initialization-order concern**, not runtime concurrency, visibility or ownership. A separate global with non-trivial dynamic initialization can still impose dependencies, and initializing one object early does not establish a lifetime relationship with device work.

### One validation rule must vary without making every call virtual

Our tile planner always needs to check arithmetic overflow, positive dimensions and, when verifying a source buffer, its byte length. But different contexts may apply different **budget policies**. An editor's import queue might cap one tile at a specified number of bytes. A metadata-only discovery pass may have no additional application-specific byte budget, while still rejecting impossible dimensions and overflowing arithmetic.

We represent those policies as ordinary types:

```cpp
struct NoBudgetPolicy {
    void validate(std::size_t) const noexcept {}
};

struct ByteBudgetPolicy {
    std::size_t maximum;

    void validate(std::size_t count) const {
        if (count > maximum)
            throw std::length_error("tile exceeds byte budget");
    }
};
```

A policy is a **choice about how an operation is performed**, separated from the larger operation that uses it. The type `NoBudgetPolicy` carries no stored state, whereas `ByteBudgetPolicy` stores a numeric limit. The phrase "no budget" means no *additional application cap*; neither policy bypasses width/height overflow or, in verified-payload mode, the actual buffer-length check. Choosing one policy through a template parameter means that the compiler knows which `validate` function the planner calls. If we instead needed to change policy at runtime using a common interface, a virtual policy or type-erased function might be more appropriate. This is precisely the distinction between policy-based static choice and dynamic extensibility.

The planner is parameterized by both format and policy:

```cpp
template <PixelFormat Format, class BudgetPolicy, class Recorder>
class TilePlanner {
    [[no_unique_address]] BudgetPolicy budget_;
    Recorder recorder_;

public:
    TilePlanner(BudgetPolicy budget, Recorder recorder)
        : budget_(std::move(budget)), recorder_(std::move(recorder)) {}

    // prepare(width, height, payload, mode) is implemented in the lab.
};
```

The policy instance's *type* is fixed by the specialization, but its stored state, such as a 64-byte budget, can still be supplied at runtime. That distinction is important: choosing `ByteBudgetPolicy` at compile time does not mean the number 64 must be a template argument. We can change budgets between planner objects without compiling a different type for every numeric value.

For the RGBA example, width eight and height two require \(8\times2\times4=64\) bytes. With a 64-byte budget, the plan is accepted and recorded. An 8-by-3 tile would require 96 bytes, so the same planner throws before recording it. A different 4-by-4 tile also requires 64 bytes, but when the verified-payload mode is used with an empty buffer, it fails the length check. Both failure paths leave the recorder's count at one. The error source and the publish boundary remain observable; policy selection does not hide them.

### The recorder is known statically; explain the CRTP rather than mystifying it

The planner must report successful work plans. In Phase 1 we used a virtual `Backend` because a runtime-selected implementation had to satisfy one interface. The tile planner can make a different choice: it can know its recorder implementation as a template argument. The **Curiously Recurring Template Pattern**, or **CRTP**, is one way to express this static relationship:

```cpp
template <class Derived>
struct RecorderBase {
    void record(const TilePlan& plan) {
        static_cast<Derived&>(*this).record_impl(plan);
    }
};

struct CountingRecorder : RecorderBase<CountingRecorder> {
    std::size_t calls = 0;
    std::size_t total_bytes = 0;

    void record_impl(const TilePlan& plan) {
        ++calls;
        total_bytes += plan.bytes;
    }
};
```

The derived class declares itself as the base template's parameter. The base function can therefore convert its `*this` to the named derived type and call `record_impl`. The compiler resolves the concrete call through known types rather than through a virtual function table. This is *static polymorphism*—different compile-time types expose a common operation without requiring an ordinary runtime virtual interface.

The cast is not an arbitrary trick that makes any base object a derived object. The pattern relies on the real object being a correctly formed `Derived` whose `RecorderBase<Derived>` base subobject is being used. Constructing an unrelated base object and invoking the derived cast would not establish a valid derived object. The derived operation must also exist and be accessible under the chosen design. Concepts or direct constrained calls can communicate many of those requirements more explicitly, especially in modern C++; CRTP is a tool, not compulsory boilerplate.

The lab creates a `CountingRecorder`, submits one accepted 64-byte plan, and verifies both `calls==1` and `total_bytes==64`. Failed plans never reach `record`. The plan itself is a small value containing dimensions, byte count, one illustrative Morton key and whether the format includes alpha. These fields describe **planned work**; they are not proof of completed GPU transfer.

Why not always use CRTP? A `TilePlanner<Format,Policy,CountingRecorder>` is a different type from a planner instantiated with another recorder. If the editor must store arbitrary recorder implementations in one container and select them at runtime, it needs another abstraction, such as an erased callback or virtual interface. CRTP can enable inlining, but whether it does so is an optimization question; source-level absence of `virtual` is not a measured throughput comparison. It may also increase code size through multiple instantiations. We must select the dispatch mechanism according to whether the variation occurs at compilation or execution.

### The same operation can be verified or merely described—without a Boolean flag

There is another source of variation: should `prepare` check the actual byte buffer, or should it produce **only a metadata plan** when no buffer is available? A Boolean `verify_payload` would work, but it is easy for a caller to confuse a plan with a verified upload or to leave a condition buried in a long method. Since the caller knows which operation it wants at the call site, we can represent the decision using distinct empty **tag types**:

```cpp
struct VerifyPayloadTag {};
struct MetadataOnlyTag {};

void check_payload(std::span<const std::byte> bytes,
                   std::size_t expected, VerifyPayloadTag) {
    if (bytes.size() != expected)
        throw std::invalid_argument("payload size mismatch");
}

void check_payload(std::span<const std::byte>,
                   std::size_t, MetadataOnlyTag) noexcept {
    // Deliberately produce a plan only: no verified-buffer claim.
}
```

This is **tag dispatch**. The tag argument is an object of a type selected by the caller. Ordinary function overload resolution chooses the matching function at compile time, because `VerifyPayloadTag` and `MetadataOnlyTag` are distinct argument types. The compiler does not need a runtime string or integer to choose between the operations. An empty tag normally contains no application data; its *type* is what carries the decision.

The semantics matter more than the dispatch technique. A request with an empty payload must be rejected in verified mode if the expected size is nonzero. In metadata-only mode, the same expected size may be calculated and recorded, but the planner **must not claim the payload exists, is decoded, or is ready for a GPU consumer**. The lab tests this exact difference. A 4-by-4 RGB tile produces a 48-byte manifest without source data; the result is explicitly a plan, not an upload. Tags create a readable distinction, but a production API might go further and return different types for `VerifiedTile` and `TileManifest` so that downstream code cannot confuse them. The current lab does not implement that stronger distinction.

Tag dispatch and `if constexpr` are related but not identical. Tags select among overloads using distinct parameter types; `if constexpr` selects an instantiation branch within one function template. Policies carry a larger strategy and may store runtime state. CRTP supplies a compile-time-known base/derived call shape. These mechanisms can cooperate, but using all of them in every class would make ordinary code needlessly elaborate. Here each appears only because a specific decision—payload readiness, format, budget, or recorder implementation—has a distinct useful contract.

### If a policy has no data, why can its ordinary member increase object size?

We now have a no-budget policy object that has no non-static data members. It exists as a real C++ type, but adding it as an ordinary member to every planner may seem wasteful. The reason requires the earlier memory-layout model: C++ objects and subobjects have identity and alignment rules, and a normal empty class type still occupies at least one byte when instantiated as a complete object. Its size and the padding introduced around it can affect the containing class.

Consider three structurally similar holders:

```cpp
struct PlainPolicyHolder {
    NoBudgetPolicy policy;
    std::size_t counter;
};

struct EboPolicyHolder : private NoBudgetPolicy {
    std::size_t counter;
};

struct CompactPolicyHolder {
    [[no_unique_address]] NoBudgetPolicy policy;
    std::size_t counter;
};
```

The first has an ordinary empty member, so the implementation must represent its identity subject to the language rules; alignment of the following `std::size_t` can introduce padding. The second uses inheritance from an empty base. **Empty-base optimization (EBO)** allows the implementation to avoid additional storage for certain empty base subobjects, while satisfying the relevant address/identity constraints. The third uses C++20's `[[no_unique_address]]` attribute. It makes the data member a *potentially overlapping subobject*, allowing an implementation to overlap its storage with other eligible members or padding where the language permits.

On the tested Linux GCC/Clang setup, the sizes printed are:

| Holder | Observed `sizeof` |
|---|---:|
| `PlainPolicyHolder` | 16 bytes |
| `EboPolicyHolder` | 8 bytes |
| `CompactPolicyHolder` | 8 bytes |

These are **local observations**, not universal results. Different pointer widths, data models, alignment rules, compilers and ABI policies can produce different layouts. The language's constraints about which subobjects may share addresses are subtler than "all empty objects take zero bytes." For example, potentially overlapping subobjects of the same type may still have address-distinctness obligations, and the location of the first member and base classes can limit EBO arrangements. For nonempty members marked `[[no_unique_address]]`, eligible tail padding may also be reused in some layouts; that does not allow us to invent overlapping live object representations or an invalid serialization scheme.

There is a particularly important portability detail: the standard spelling `[[no_unique_address]]` is supported in modern GCC and Clang as tested here, but Microsoft Visual C++ has historically not honored that exact attribute for layout in C++20 mode, providing `[[msvc::no_unique_address]]` as an alternative extension. A cross-platform library cannot assume one attribute has the same size effect on every target. Confirm the actual toolchain behavior and relevant ABI compatibility before using object-size changes as an interface promise. See the cited portability reference in the verification ledger.

Why prefer a `[[no_unique_address]]` member in this planner rather than deriving privately from the policy? Because composition more directly communicates "this planner *has a validation policy*" and avoids some inheritance constraints, while still giving a conforming compiler freedom to avoid unnecessary policy storage. That is a design preference for this example, not a claim that all inheritance is harmful. An empty policy is still meaningful through its **type and behavior** even if a particular object representation adds no size.

The key distinction is **source-level capability versus observable layout**. We can prove that `NoBudgetPolicy` is empty with a type trait. We can make its member eligible for overlap with `[[no_unique_address]]`. We cannot prove that the compiled planner occupies a specific number of bytes until the relevant compiler, ABI and member types are fixed and measured. Nor should a raw class-layout measurement be confused with GPU buffer packing or an on-disk format.

### Static choice and runtime choice have different ownership and performance consequences

Return to the recorder. With CRTP, the concrete `CountingRecorder` is part of the planner's type, so the call target is known while compiling that specialization. We might instead pass a runtime-selected callable using `std::function<void(const TilePlan&)>`. This **type erasure** hides the concrete callable behind one uniform runtime interface and can hold different copyable callable target types. The wrapper also carries lifetime and potential allocation/copy behavior of its stored target. The Phase 2 callback chapter already derived those details; here they become part of a choice about the planner's extensibility.

The lab constructs a runtime-erased lambda that accumulates the byte count from a 48-byte metadata plan and verifies the result. It does not benchmark `std::function` versus CRTP or claim one implementation allocated and the other did not. A callable stored inline by one standard library might allocate with another implementation or capture. A direct CRTP call could be inlined, but the binary may have more specializations and a larger instruction footprint. A virtual interface could be a better contract if plugins select recorder backends dynamically. `std::variant` could suit a closed list of recorder forms. The right choice depends on **when** the implementation can be selected and how long it must remain valid.

A practical comparative experiment would implement the same trace recorder using (a) a concrete direct call, (b) CRTP, (c) a virtual base interface and (d) `std::function`, with identical records and equivalent data dependencies. Measure the cost of generating and recording many plans, count allocations separately, inspect generated code, and check whether the compiler devirtualizes targets in whole-program builds. Then vary record size and callback capture size. Static dispatch has an optimization opportunity, not an unconditional performance certificate.

Policy-based design can itself introduce code bloat: every combination of format, policy and recorder may produce a distinct specialization. If dozens of variants change only a small setup decision, a runtime enum or erased callback at a coarse boundary may reduce binary size without harming the hot loop. This is why the final architecture should keep compile-time variation where it actually resolves work inside tight, frequently executed paths and avoid gratuitous template parameters elsewhere.

### The full walk through our example, including its failure boundaries

A request arrives to plan an 8-by-2 RGBA tile with a 64-byte application budget. The selected template format fixes the channel count at four. Runtime width and height produce sixteen pixels, and checked multiplication produces 64 bytes without overflow. The budget policy permits the request. The verified-payload tag requires the input span to contain exactly 64 bytes; the lab provides that payload. The `if constexpr` format branch marks the plan as alpha-bearing. The precomputed Morton table supplies the demonstration key 39 for coordinates (3,5). The CRTP recorder receives the completed `TilePlan`, increments its call count, and accumulates 64 bytes. Only after the recording call returns does the diagnostic global count increase. Under the tested single-threaded conditions, the plan has reached the recording boundary exactly once.

Now change one input at a time. An 8-by-3 RGBA plan would require 96 bytes, so the budget policy rejects it. The recorder count stays one. A 4-by-4 RGBA plan has a permitted 64-byte size but receives an empty input span under `VerifyPayloadTag`, so the buffer-length check rejects it. The recorder still reports one accepted plan. Next, a different planner specializes to three-channel RGB, uses the empty no-budget policy, and receives a 4-by-4 request under `MetadataOnlyTag`. Its 48-byte metadata plan is recorded without claiming the pixels were present. Finally, a deliberately enormous width triggers the multiplication overflow guard before any policy or recorder can accept a nonsensical byte count.

These observations exercise three separate validation layers: arithmetic safety, application budget, and readiness of the supplied byte span. They should not be collapsed into one vague "validated" flag. The manifest case is legal only because its semantics explicitly say it is **not** verified payload data. In a real renderer the next stages would also check format support, image extent limits, row pitch, actual device memory and queue readiness before presenting a GPU-usable texture. None of those native checks is implemented by this CPU laboratory.

There is also a transaction boundary worth examining. The `TilePlanner::prepare` function invokes `recorder_.record(plan)` and **then** increments a global counter. If a general recorder throws during `record`, the code has no guarantee that the recorder itself was unchanged before throwing. This is not Phase 2's full strong-guarantee asset publication transaction, and we must not reuse that label casually. For the included `CountingRecorder`, the operations used in the test are simple integer increments without ordinary throwing operations under the tested state, so the demonstrated failure paths all occur **before** the recorder call. A production recorder with disk, network or GPU side effects needs an explicit commit/rollback or compensation contract. Static dispatch does not supply atomicity.

### A mastery test that connects every mechanism without breaking the narrative

You should now be able to reconstruct the following reasoning without opening the source. Start with a mesh/texture asset pipeline and explain which values are known by the compiler and which appear only at runtime. Derive the source-byte size \(W\times H\times C\) and explain why multiplication must be checked before forming the result. Design a trait whose compile-time channel count is an `integral_constant` value, then show when `if constexpr` eliminates a format-specific branch and when `constexpr` merely permits a calculation to execute during compilation.

Build a legal compile-time tile configuration with `consteval`; explain why a runtime argument cannot replace a required constant expression. Derive a four-bit Morton table from interleaving coordinate bits and verify why (3,5) yields 39. Explain why the resulting `inline constexpr` table and the `constinit` mutable counter make different guarantees. Choose two budget policies and show why the checked arithmetic contract remains valid under both. Build a `RecorderBase<Derived>` and explain precisely why the derived cast is sound only for a genuinely appropriate object; contrast that static call with runtime type erasure. Use empty tag types to distinguish a verified payload from a metadata-only plan and explain why the latter cannot be used as proof of upload readiness. Finally draw the three policy holder layouts and state what EBO and `[[no_unique_address]]` *permit*—and what they do not promise across compilers.

The runnable witness for these exercises is [[tools/labs/compile_time_asset_policy_lab.cpp|the M014 tile-planning laboratory]]. It was compiled and executed with GCC 14.2 using C++20, `-Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined`; with Clang 17 under strict C++20 warnings; and with GCC using `-O2 -DNDEBUG`. All three executions reported `compile_time_asset_policy_lab: all checks passed`, and the executable also printed the three local policy-holder sizes. The committed lab's Git blob matches the locally tested file exactly (SHA `f4379dbce70c261f2832f8311a628e346f7471f7`). This is a tested teaching model, **not** a benchmark, Kairo/Maveb source audit, portable ABI certification, thread-safety proof, or graphics driver test.

Rebuild using the full source and command:

```sh
g++ -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  tools/labs/compile_time_asset_policy_lab.cpp -o /tmp/compile_time_asset_policy_lab
/tmp/compile_time_asset_policy_lab
```

This chapter provides an authored reconstruction of the exact original **M014, Parts 217–230**. It is inserted where our generic asset system requires a compile-time policy decision, rather than treating the fourteen numbered items as fourteen standalone lessons. [[Supplementary/Foundations#M014 — Parts 217–230: one tile planner and its compile-time policy decisions|The coverage map]] preserves every title; [[Supplementary/Sources and Code Anchors#Phase 4 additive reconstruction — M014 / Parts 217–230|the evidence ledger]] identifies executed and unverified claims; [[Visual Recall/02 - C++ - Objects, Ownership and Lifetime - Visual Recall#The tile planner makes a choice — M014 reconstruction|the recall path]] reconnects the ideas to the same asset pipeline. No new original lesson transcript has been received, and the suggested `KairoMath/Matrix.cppm` / `KairoECS/SparseSet.cppm` project anchors have not been re-inspected in this phase.

## An asset registry grows while its handles must not change

The last asset-pipeline chapter could plan a texture tile, choose a compile-time pixel format and policy, reject an invalid byte count, and record accepted work. But we deliberately left the asset depot with a fixed `Capacity`. That is a useful testing model; it is not how a long-running editor behaves. An editor loads ten meshes, then ten thousand. It closes a scene, removes some of them, hot-reloads others, searches for a texture by name, schedules hundreds of uploads, and allows rendering code to keep a reference to the *logical* asset while the underlying collection changes.

The first temptation is to replace `std::array<std::optional<T>,Capacity>` with `std::vector<T>` and declare the problem solved. That gives growth, but opens a second question: **does the address or iterator used to identify an asset remain valid after the vector moves its storage?** The answer is not always. The next temptation is to use a linked list for address stability, but then we give up contiguous iteration and often pay for extra allocations. Another tempting choice is a hash table for all names, but a hash table can rehash and invalidate its iterators, and a maliciously poor hash can turn an apparently constant-time lookup into a long scan.

These are not twenty unrelated library facts. They are decisions about three independent promises the editor must make: **where objects physically live**, **how logical identities survive movement and deletion**, and **which operations need sequential traversal, lookup, or scheduling**. Our running design will use contiguous storage where useful, a generation-aware logical handle to reject an old occupant, an owned string for persistent names, a hash index for name lookup, and standard adapters for the task queue. We will compare the alternatives at the exact points where their tradeoffs matter. The companion executable is [[tools/labs/asset_container_lab.cpp|the C++20 growing asset registry laboratory]].

### A vector has three quantities, but only one of them counts living elements

Start with a dense collection of integers representing indices or compact asset records. A `std::vector<int>` is a dynamically growing sequence whose elements are contiguous. On typical implementations the vector object contains metadata describing a pointer, a size and an end-of-capacity boundary; **that three-pointer arrangement is an implementation model, not a mandated class layout**. What the C++ interface guarantees is enough for our reasoning:

- `size()` counts the constructed, live elements in the current sequence.
- `capacity()` is the number of elements that can be held without another allocation of the element storage.
- `data()` points to the beginning of the contiguous element range when applicable; element `i` is reachable through `data()+i` for indices within `[0,size())`.

In other words, reserving physical capacity is not the same as constructing objects in that capacity. Suppose we begin with an empty vector and call `reserve(2)`. The vector is still empty: its size is zero, and accessing `v[0]` is invalid even though the vector has enough reserved memory for two future `int` elements. After pushing 10 and 20, its live range has size two. Pushing another element when `size()==capacity()` requires new element storage and relocation of the existing values; after that operation, the new live range contains 10, 20, 30.

The tested code records the capacity before and immediately after that third insertion. On the local Linux standard-library implementation it printed `vector capacity growth: 2 -> 4`. That number is a **measurement**, not an invariant of the C++ standard. Other implementations can select different capacities. The portable fact is that `capacity()` must be large enough for the new element and that the old storage may be replaced when the vector has reached its existing capacity.

```cpp
std::vector<int> values;
values.reserve(2);                    // capacity >= 2, size == 0
values.push_back(10);                  // size == 1
values.push_back(20);                  // size == 2

const auto before = values.capacity();
values.push_back(30);                  // requires reallocation if size was capacity
const auto after = values.capacity();  // after > before in this case

values.resize(5);                      // now creates two more live int objects
// values[3] == 0 and values[4] == 0 for this allocator/value type.
```

This is where `reserve` and `resize` separate. `reserve(n)` requests *space* for at least `n` elements, leaving `size` untouched. `resize(n)` changes the number of *live* elements: growing constructs new ones, and shrinking destroys those removed. For ordinary `int` with the default allocator, appended value-initialized entries are zero. For a nontrivial asset type, `resize` needs a valid construction path and can have very different costs. Calling `reserve` before a known batch can avoid repeated allocations; calling `resize` to prepare capacity when no objects are intended is a semantic error. Shrinking size does not necessarily shrink capacity. `shrink_to_fit` requests a reduction but does not promise an exact amount or timing. A bad practice is to call `reserve(size()+1)` before every insertion, which can defeat geometric growth and cause repeated relocations.

One important nonstandard-looking exception to "vector is a normal contiguous array of its value type" is `std::vector<bool>`. It is a specialized representation that may pack Boolean values into bits and uses proxy references rather than ordinary independent `bool&` elements. A graphics engine's tightly packed mask should choose an explicit bit-packing abstraction when representation is part of the contract; do not infer that `std::vector<bool>` gives the same address/view semantics as `std::vector<std::uint8_t>`.

### Why vectors usually grow geometrically: derive the cost instead of repeating “amortized O(1)”

Allocating a new block for *every* append would be expensive. If a sequence grows from zero to `n` elements, and on each append we move every existing element into a precisely sized replacement block, the total number of moved elements is:

\[
0+1+2+\cdots+(n-1)=\frac{n(n-1)}{2}.
\]

This is quadratic in `n`. It is not a reasonable growth strategy for a large asset array.

A common implementation instead chooses a new capacity that is a factor `g>1` larger when the vector fills. If the capacities are approximately `1,g,g^2,\ldots,g^k`, the total number of previously stored elements copied or moved across those reallocation events is a geometric series:

\[
1+g+g^2+\cdots+g^{k-1}
=\frac{g^k-1}{g-1}.
\]

Since the last capacity is on the order of the final element count, the total relocation work over all appends is proportional to `n` for fixed `g>1`. Divide total work by `n` appends and the *amortized* relocation cost per append is constant, even though one particular append may relocate a large existing sequence. For a factor around two, the geometric series of prior capacities is bounded by a small multiple of the final size; for a smaller growth factor the storage overhead may be lower but the number of reallocations higher. These are tradeoffs, not standardized growth factors.

**Amortized complexity** is not the same as every operation being worst-case constant time. A single append that grows storage can be linear in the old size. That distinction matters if the operation occurs on a frame-critical thread with a strict latency budget. You might preallocate the expected high-water mark, use a ring buffer, use a paged registry, or move allocations into a noncritical phase. Choosing a geometric factor yourself also entails working-set and memory-retention tradeoffs. The C++ standard guarantees the relevant vector operation complexity, but does **not** mandate the specific ratio, or promise that copying and moving an arbitrarily expensive asset type costs a fixed number of cycles.

An engine's data-oriented design is shaped by locality as well as asymptotic complexity. A vector of small records that are read sequentially may use caches efficiently. A vector of pointers into separately allocated objects might preserve the vector's contiguous **pointers** while still requiring many cache misses to reach the objects. Physical contiguity belongs to the actual `T` elements in the vector; it does not recursively make every allocation reachable through `T` contiguous.

### A growing vector invalidates addresses, not logical identity—if we design identity correctly

Our old fixed-capacity depot returned a slot number. A naive replacement might return `Asset*` or retain a `std::span` of the vector's elements. That can work within a carefully bounded interval, but it breaks when a later reallocation moves elements.

When `std::vector` reallocates, all iterators, references, and pointers into its previous element storage are invalidated. Calling an old iterator or dereferencing an old pointer after reallocation is not a way to ask the vector for the new location. **We do not run intentionally invalid accesses in the laboratory.** Instead, the test checks that the capacity-changing operation occurred and states the standard's resulting invalidation rule. The distinction between observing the event and deliberately executing undefined behavior is part of the required engineering proof.

When a vector appends *without* reallocation, existing element references/iterators remain valid, but the past-the-end iterator is invalidated. Inserting or erasing at an interior position can invalidate iterators and references at or after the point because the following elements shift. `pop_back` invalidates the removed element and the old end. `clear` ends the lifetimes of all elements. `reserve` invalidates all element references only when it actually changes capacity. Keep those rules attached to the operation; do not write "vector pointers never survive mutations" as a substitute for reasoning about the concrete mutation.

A **logical handle** addresses a different problem. It should identify the asset through a registry, not by carrying the current memory address. In our CPU teaching model, the handle is:

```cpp
struct AssetHandle {
    std::size_t slot;
    std::uint64_t generation;

    friend bool operator==(const AssetHandle&,
                           const AssetHandle&) = default;
};
```

The `slot` is an index into `std::vector<Slot>`. The generation identifies which occupant is expected at that slot. Suppose `rocks.mesh` is inserted into slot zero with generation one. If the vector later reallocates to hold many other slots, the numeric slot index zero still denotes the first *position of the new vector*. The registry checks the slot and generation afresh each time a handle is resolved. That is why a stable logical handle can survive physical relocation even though a previously borrowed `Asset*` cannot.

Now remove the asset. The registry destroys the occupant and increments that slot's generation before making it reusable. A new asset may occupy slot zero at generation two. The stale handle `{0,1}` fails, even though slot zero now contains a perfectly live *different* asset. This is the same ABA-like identity hazard we studied in ownership and ECS: equal numeric storage locations do not guarantee equal object identity across lifetimes. Our test verifies that the old handle cannot resolve and the new handle can.

The implementation handles one additional edge condition deliberately: a 64-bit generation must not silently wrap and accidentally recreate an ancient ID. The example rejects recycling a slot whose generation has reached `UINT64_MAX`, rather than wrapping. In a production system, the type width, exhaustion policy, registry lifetime and cross-session identity all need explicit design. The test registry does not implement concurrent access, persistence, device completion or globally unique IDs. Those are different contracts.

### A stable handle is not a transactional insertion algorithm by itself

The registry stores a `std::vector` of slots, an additional vector of reusable free-slot indices, and a name-to-handle index. Inserting a new asset first validates nonempty name/data and uniqueness. If a reusable slot exists, it is selected; otherwise the slot vector grows. The code constructs a complete `Asset` containing an owning `std::string` name and `std::vector<std::uint8_t>` source payload. Only then does it insert a name-to-handle entry in the hash table. That entry is the registry's publication path for name lookups.

Why do we need more than "construct, then insert"? Hash-table insertion can allocate or throw. If an exception occurs *after* the slot has become occupied but *before* the name index is updated, a naive implementation leaks an unreachable live slot into its own registry state. The lab therefore surrounds the publication operation with a rollback path. If the hash insertion or deliberate commit-stage injection throws, it resets the unpublished slot. If the slot was newly appended, it also pops the unused last slot; if it was selected from the free list, it leaves that index free for another attempt. The test injects two separate failures: before slot mutation and after construction but before name publication. Both preserve the previously published asset and do not increase the persistent slot count.

```cpp
// Simplified structure of the actual tested publication protocol:
if (!reusing_free_slot)
    slots_.emplace_back();      // may allocate; no published entry added

Slot& pending = slots_[index];
pending.value.emplace(Asset{std::move(name), std::move(bytes)});

try {
    names_.emplace(pending.value->name, AssetHandle{index, pending.generation});
} catch (...) {
    pending.value.reset();      // destroy the unpublished candidate
    if (!reusing_free_slot)
        slots_.pop_back();      // restore the prior slot count
    throw;
}
if (reusing_free_slot)
    free_.pop_back();           // commit retirement of the free slot
```

The complete code has additional validation, duplicate handling and a test injection flag. The rollback depends on specific operations being nonthrowing on this chosen element type, which the lab checks with `std::is_nothrow_move_constructible_v<Asset>`. The program also arranges free-list capacity before removal. That supports a meaningful *single-threaded observable-state* guarantee for the demonstrated paths, but it does not prove every allocator, custom hash, throwing equality operator, or reentrant observer is handled correctly. If a user-installed comparator or callback can throw after publishing an external side effect, a more extensive transaction or compensation protocol is necessary. Do not extrapolate the test's result beyond the operations it actually exercises.

Deletion is also a protocol, not simply `erase(name)`. The example first reserves a place in the free list and removes the name index, then ends the occupant's C++ lifetime, increments the generation, and records the slot for reuse. Every outstanding raw view into that deleted `Asset` becomes invalid. A logical handle to that exact generation becomes stale. In a renderer, the GPU may still be using the native resource after the CPU asset's logical deletion, so the native object may require deferred retirement. The CPU handle registry by itself cannot prove it is safe to free GPU memory at this point.

### Why a fixed array is still the right answer in some parts of this engine

Not every collection needs dynamic growth. The tile planner's four-element channel metadata, one small camera basis, or a fixed group of diagnostic counters may have a known number of elements. `std::array<T,N>` owns exactly `N` elements, stores them contiguously within the array object, supports indexed access and does not add a dynamic growth protocol. Its size is part of its type; `std::array<int,3>` and `std::array<int,4>` are different types.

In the laboratory, `std::array<int,3>{2,4,6}` is a fixed contiguous sequence, and `data()[2]` denotes the third element. The array does not choose a heap allocation by itself, but where the containing object lives still depends on its own storage duration and ownership. An `std::array` that is a member of a heap-allocated object is stored inside that heap object; an automatic local one commonly resides in automatic storage. Saying "std::array is always on the stack" is inaccurate.

A zero-length array specialization has its own documented iterator/data subtleties; the C++ standard does not make an arbitrary zero-sized array a useful source of a dereferenceable first element. For graphics buffers, array bounds and alignment remain explicit. A fixed `std::array` is excellent for the program-known four-channel metadata; it is not appropriate for an editor's unbounded list of user-imported textures. Choosing between vector and array follows from the collection's **growth contract**, not from a simplistic claim that one is always faster.

### A queue of both recent and old work needs inexpensive front and back operations

The editor also accumulates a stream of user events or network packets. It may append events at the back and consume them from the front. Removing the first element of a vector shifts all later elements toward the beginning, a linear operation. If this is a dominant operation, a `std::deque<T>` is often a better fit.

`deque` means **double-ended queue**. It provides indexed random access and efficient insertion/removal at both ends, but it is **not one contiguous `T[]` range** in the same way as `vector`. Typical implementations use segmented blocks and a structure to locate those blocks; the exact block size and representation are not standardized. The lab pushes 2 at the back, 1 at the front, and 3 at the back, then checks indexed order 1,2,3. This demonstrates its sequence semantics and both-end operations, not a particular physical block layout.

The invalidation rules differ importantly from vector. End insertion into a deque can invalidate **iterators**, even though references to existing non-erased elements remain valid under the specified operations. Middle insertion and erasure have more disruptive rules. `deque` is therefore not a universal answer to pointer/iterator stability, and a `span` cannot be made over an arbitrary whole deque as though it were one contiguous vector. Its segmented layout can increase pointer chasing and affect traversal locality, whereas vector's contiguous storage often wins for scan-heavy data. Use `deque` because the queue operation needs both ends, not because its name sounds more stable.

### Node-based linked structures trade locality for specific mutation guarantees

Some editor data change their order frequently while callers retain iterators to individual nodes. A `std::list<T>` is a doubly linked sequence abstraction with efficient insertion/removal at a known position and a splice operation that can transfer existing nodes under the operation's allocator requirements. Many implementations use one allocation per node with two link pointers, but the precise representation is not mandated.

The lab creates a list of operations `import` and `compile`, holds an iterator to `compile`, then **splices** an `upload` node from another list. The iterator to `compile` remains usable; it is not affected by transferring a different node. This is a valuable guarantee for algorithms that need to reorganize work without relocating each payload. It does not automatically justify list usage in a renderer. A linked traversal visits nodes with less spatial locality and more pointer indirection than a vector scan. Finding the position in the first place can cost linear time; constant-time insertion at a known iterator is not constant-time search.

`std::forward_list<T>` is a singly linked sequence abstraction. It commonly stores fewer links per node and offers only forward iteration. That changes the interface: to remove an element efficiently, we often need an iterator to the element **before** the one being removed, so operations are named `insert_after` and `erase_after`. The lab constructs 1,2,3 and calls `erase_after(begin())`, obtaining 1,3. The operation removes two because the supplied iterator names its predecessor.

Why not use forward_list everywhere if it has fewer links? Because its API sacrifices backward traversal, direct indexing, and common operations such as constant-time `size()`. For scanning dense renderable components, node allocation and pointer chasing usually deserve scrutiny. Linked structures are valuable when the real workload needs their mutation and iterator guarantees; "O(1) insertion" without including the search cost or allocator cost is a misleading argument.

### Asset name lookup is a separate job from storing the asset bytes

The registry has two fundamentally different access patterns. During rendering it may iterate all live asset records or construct dense component arrays. During an editor search it may ask "Where is `rocks.mesh`?" A sequential vector of all strings could answer the question by comparing every name, but that scales linearly with the number of assets in the worst case. We want a dedicated name index.

A `std::map<Key,Value>` maintains keys in comparison order. A `std::set<Key>` maintains unique keys, without a separate mapped value. Both are typically implemented using balanced search trees, but C++ specifies the relevant ordering and complexity contracts rather than mandating a particular red-black-tree node layout. For a balanced tree, the height grows on the order of `log₂ n`, giving logarithmic lookup/insertion/erasure. The map can provide ordered iteration, lower/upper-bound queries and predictable key comparison semantics. The set answers membership questions with ordered unique keys.

The laboratory's map stores names `mesh`, `shader` and `texture` and iterates them in their standard lexical comparison order; the set inserts IDs 8,3,8,5 and retains three unique values, beginning with 3. Those tests demonstrate the semantics, not the actual shape or rotations of a tree. Storing a key in a `map` also has a real ownership implication: an inserted `std::string` key owns its characters, while a `std::string_view` key may become invalid if the original bytes are destroyed. Ordered keys must obey a **strict weak ordering** relation; an inconsistent comparator can break the container's assumptions. Mutating a key in place would also break ordering, which is why associative-container key access is constrained.

Ordered containers have node-based storage, comparison costs and memory overhead. They are not categorically slower or faster than hash tables. They may win when the editor needs sorted traversal, range queries or deterministic order independent of hash bucket arrangement. If the actual requirement is only fast exact-name lookup, the next option is worth examining.

### A hash index finds names quickly on average—but its correctness is not its speed

`std::unordered_map<Key,Value>` and `std::unordered_set<Key>` provide associative lookup organized by a hash function and a key-equivalence relation. The map stores a value alongside each unique key; the set stores membership. A hash converts the key into a `std::size_t`-like hash value; the container uses that result to choose a bucket or another implementation-specific lookup location.

Two *equivalent* keys must produce the same hash value. But **different** keys may produce the same value: that is a **collision**. A correct hash table must still distinguish colliding keys by equality checks, not assume the hash alone proves identity. The laboratory uses an intentionally terrible hasher:

```cpp
struct ConstantHash {
    std::size_t operator()(const std::string&) const noexcept {
        return 0; // all names collide, deliberately
    }
};

std::unordered_map<std::string, int, ConstantHash> bad;
for (int i = 0; i < 24; ++i)
    bad.emplace("asset" + std::to_string(i), i);
```

Every name hashes to the same bucket. Lookups remain **correct**—the test retrieves `asset19` and gets 19—but the bucket containing `asset0` has all 24 elements. The number of collisions is an observable property of this chosen function and workload, not a claim about the standard library's default hasher. The example is a counterexample to the naive belief that unordered means guaranteed constant-time lookup.

Under typical hash distribution assumptions, insertion, lookup and deletion have **average constant-time** complexity. In the worst case, many keys collide and operations can become linear in `n`. The relevant hash *quality* depends on the key distribution and attacker model. A fast hash that clusters similar file paths may degrade in a real asset catalog; a cryptographic or keyed hash may be appropriate when untrusted users can choose adversarial keys, though it has different cost and threat-model considerations. Security, distribution, and performance should not be collapsed into one adjective such as "good". Always verify that the equality and hash functions are mutually consistent, including case-folding or path-normalization policies: if an equality relation declares two strings equivalent, they must have equal hashes.

### Load factor controls rehash pressure, not the absence of collisions

A hash table's **load factor** is:

\[
\alpha = \frac{n}{b},
\]

where `n` is the number of elements and `b` is the number of buckets. The `max_load_factor` setting establishes the desired upper bound after applicable operations and controls when growth is needed. If we want to support at least `N` elements while targeting maximum load `m`, the bucket count needs to be at least approximately:

\[
b \ge \left\lceil \frac{N}{m} \right\rceil.
\]

In the lab we set `max_load_factor(0.5f)` and later call `reserve(96)`, requesting enough bucket capacity for that many elements under the configured maximum. This request is not the same as reserving space for 96 contiguous objects. `unordered_map::reserve` is connected to bucket reorganization, not vector's element storage. Some implementations may grow or choose a bucket count using their own policies; the exact bucket number is not a portable constant.

When an unordered container **rehashes**, its iterators are invalidated. Pointers and references to elements generally remain valid across rehash under the container's specified guarantees; erasing the actual element destroys that element and invalidates views of it. This is a subtle difference from vector, where reallocation invalidates references to the former element storage. Our laboratory deliberately does not dereference any pre-rehash iterator after calling `reserve`. Checking the resulting `bucket_count` and `load_factor` is safe; accessing an invalidated iterator would not be.

Lower load factor can reduce average bucket occupancy but increase bucket metadata and memory use; it cannot repair a constant hash function. The terrible hasher still puts all 24 names in one bucket even if the table allocates hundreds of buckets. This is a useful research bridge: vary key distributions, number of assets, maximum load factors and hash algorithms; measure lookup latency distributions, total allocations and memory use. Do not claim hash lookup is always constant time, or that a lower load factor always improves overall throughput.

The actual registry uses `std::unordered_map<std::string, AssetHandle> names_` as a **name index**, while the vector holds the actual asset objects. It owns the string key because the index must remain valid after the temporary input buffer or file parser is gone. Its `find(std::string_view)` creates a temporary owned string for the lookup; that may allocate. A production table could use compatible transparent hashing and equality to support heterogeneous lookup without making that temporary, but the exact C++20 requirements and implementation support must be checked rather than assumed. Keeping this allocation explicit in the teaching code makes the source of cost visible.

### The order in which work executes is yet another independent decision

The editor now has assets, but it also has events: decode mesh A, upload texture B, compile shader C, process an error, or undo a command. Not every worklist needs random indexing or name-based lookup. Sometimes the public **access policy** is the whole reason to choose a container.

A `std::queue<T>` is a *first-in, first-out* adapter. Insert at the back, process from the front. In the lab we push 7 and then 8, confirm the first readable item is 7, pop it, and then see 8. A `std::stack<T>` is a *last-in, first-out* adapter, useful for undo histories, nested operations, or temporary traversal states: push 7, push 8, pop 8 before 7. Both are **container adapters**: they expose selected operations on top of an underlying container rather than providing full general iteration. Their named semantics are the API contract; whether they use a particular default underlying `deque` or another suitable container is an implementation/configuration detail.

A `std::priority_queue<T>` retrieves the highest-priority item according to a comparator; it is commonly implemented as a heap over a sequence such as `std::vector`. A binary heap supports access to the top in constant time and insertion/removal of the top with logarithmic complexity. Its underlying storage is not automatically a fully sorted array. If we have tasks at priorities 3, 1 and 3, which of the two priority-3 tasks comes first? **Priority queues do not promise stable ordering of equivalent-priority elements.**

The lab encodes a deliberate tie-breaking policy:

```cpp
struct UploadJob {
    int priority;
    std::uint64_t sequence;
    std::string name;
};

struct JobLess {
    bool operator()(const UploadJob& a, const UploadJob& b) const {
        if (a.priority != b.priority)
            return a.priority < b.priority; // larger numeric priority wins
        return a.sequence > b.sequence;     // earlier sequence wins a tie
    }
};
```

With two priority-3 jobs having sequence two and one, the job with sequence one is selected first, followed by sequence two; the priority-1 job follows. The comparator supplies the ordering that the adapter itself does not promise. Real job schedulers must also consider fairness, starvation, cancellation, dependencies and concurrent ownership. A `std::priority_queue` is not automatically a thread-safe work-stealing scheduler, and choosing a heap is not sufficient to implement a render graph.

### A span can inspect bytes without owning them—and that is precisely its danger

Decoding and validation often need to examine a contiguous subset of asset bytes. Copying the entire byte vector into every helper would increase allocation and bandwidth. Taking an unbounded raw pointer loses the length needed for safe range checks. C++20's `std::span<T>` gives a non-owning view of a contiguous sequence, conceptually a pointer plus an extent, with the exact representation left to the implementation. A dynamic-extent span records a runtime length; a fixed-extent span can put a known extent into its type.

In our laboratory the checksum helper is:

```cpp
std::size_t checksum(std::span<const std::uint8_t> bytes) noexcept {
    std::size_t sum = 0;
    for (std::uint8_t byte : bytes)
        sum += byte;
    return sum;
}
```

This accepts a vector-backed contiguous payload without taking ownership of it. A caller can pass the current live byte range, and the function iterates only the span's extent. The test checks that `{10,20,30}` sums to 60. The helper remains valid because the owning asset and its byte vector are alive and unchanged **during that synchronous call**. The span does not keep the asset alive, does not prevent the vector from reallocating, and does not synchronize another thread that might mutate the bytes.

A span's size makes some bugs easier to prevent, but it is not a general proof that its pointer is valid. If you return a span to a local vector, the vector's storage is destroyed at scope exit and the returned span dangles. If you store a span of a vector and then resize or append in a way that reallocates that vector, the span may dangle. If you erase an asset and then use a previously obtained byte span, the asset's payload may have been destroyed. C++20's range and borrowing concepts do not magically extend ownership here; lifetime is a separate contract. For GPU API calls, a span can be a convenient CPU input view, but an asynchronous upload must copy or otherwise retain its storage through the required completion interval.

### Paths and asset names need two different string types at two different boundaries

The registry's name lookup also exposes the important distinction between `std::string` and `std::string_view`. A `std::string` owns its character sequence and maintains its size, and can grow or shrink. On many standard-library implementations, **small-string optimization (SSO)** stores short strings within the string object's own representation instead of allocating a separate heap buffer. That can be extremely useful for short entity names or small path fragments, but the C++ standard does not mandate a particular SSO capacity or even that every implementation employ the same optimization. The lab prints the capacities of `"mesh"` and a 1,000-character string; on the local Linux environment they were `15/1000`. Those are observed capacities, not universal SSO thresholds or allocation counts.

A `std::string_view` does **not** own characters. It refers to a contiguous sequence of characters with a length. That is perfect for parsing or comparing a borrowed path while the owner remains alive:

```cpp
std::string owner = "texture/stone/normal.png";
std::string_view full{owner};
std::string_view part = full.substr(8, 5); // "stone", length 5

std::string retained{part}; // copy the five characters into owning storage
owner.clear();              // don't use part after this invalidates its content
// retained remains "stone".
```

The substring is not necessarily NUL-terminated at its end. Passing `part.data()` to a C API expecting a NUL-terminated string would be incorrect: the next bytes in the original string could continue into `/normal.png`. A `string_view` is not just a cheap `const char*`; it is a *pointer-and-length view*. The view's constructor or substring operation does not extend the underlying string's lifetime. Taking a view of a temporary `std::string` and retaining it for later is a common dangling-reference bug.

This directly explains our registry API choices. A synchronous `find(std::string_view)` can borrow an input path just for the lookup. A stored key in `names_` must own its characters, so the implementation copies or moves the string into `std::string` storage. A callback that retains a path for asynchronous work must also own the characters or have an explicitly longer-lived owner. Choosing the right type depends on **who uses the text and for how long**, not simply on the cost of one character copy.

Strings have their own iterator/reference invalidation rules when they mutate. An SSO implementation might change between internal and allocated storage as it grows, so do not retain a `string_view` into a mutable string across operations that can invalidate it. An empty string or zero-length view may have a valid size while its `data()` is not a dereferenceable character location; check length rather than assume every pointer designates a readable element.

### The container selection can be stated as an engineering contract, not a popularity ranking

The final registry design uses a vector because indexed slots give good dense storage and allow stable **index-based** identity across capacity growth; its elements are not stable raw-address identities. A free-list vector reuses vacant indices. A hash map provides average efficient name lookup with owned keys; it does not give deterministic iteration order or immunity to bad hashes. The byte payload within each asset is another vector because decoding and checksum operations need contiguous bytes. A `span` borrows that payload during an explicitly bounded call. A string view borrows a name only during lookup, while a string owns the name in the stored index. Queue/stack/priority queue adapters organize event handling according to scheduling semantics. Arrays, deques and linked lists remain alternatives where their particular fixed-size, double-ended or node-stability properties solve a real workload.

Do not read this as a fixed universal design. If the renderer iterates tens of millions of tiny components, a sparse-set or archetype ECS with compact structure-of-arrays storage may outperform this generic registry. If assets are asynchronously loaded and evicted, the registry will need thread-safe state transitions and deferred GPU retirement. If hash collisions are adversarial, hash function and key normalization become security concerns. If memory is fragmented, custom allocators or pools may matter. If stable addresses are required during concurrent access, a paged registry or carefully owned indirection may be appropriate. Those are **new requirements** for future system layers, not gaps to paper over with claims that a container solves everything.

The next concept should become necessary only after we understand what happens to *iterators* and *algorithms* when these containers change. For now, the standard contracts above are enough to choose the right representation for the demonstrated growing registry and recognize the failures that would invalidate borrowed state.

### Trace the full lifecycle and verify precisely what the code proves

Begin with an empty `AssetRegistry`. Insert `rocks.mesh` with bytes 10,20,30. It receives a logical handle containing slot zero and generation one. Look up its name and resolve the handle, then checksum its bytes to obtain 60. Next inject a preparation failure for `broken.mesh` before changing any registry structure. The published asset and total slot count remain unchanged. Inject a second failure **after** a candidate slot has been constructed but before the name index is committed. The rollback destroys that candidate and restores the prior persistent slot count. The old mesh is still discoverable.

Now erase the first asset. Its owned string and byte vector are destroyed, its name index entry disappears, and its generation advances. The old handle must no longer resolve. Insert `fresh.mesh` and deliberately reuse the same slot; the new handle has the same numeric index but a different generation. The test proves that the new asset resolves and the old one does not. This is the distinction between a physical slot and a logical occupant in executable form.

In the same run, vector growth prints 2→4 before the later resize creates five live integers; list splicing retains an unrelated iterator; forward-list removal uses a predecessor; ordered collections preserve key order and uniqueness; a deliberately pathological hasher sends 24 keys to one bucket without breaking equality; reserve/rehash exercises load-factor policy; priority scheduling honors an explicit tie break; queue and stack expose FIFO and LIFO behavior; a span reads live bytes synchronously; and a string-view substring is copied into ownership before the source is mutated. None of the tests dereferences an invalidated iterator or dangling view.

The complete source is [[tools/labs/asset_container_lab.cpp|the M015 C++20 asset-container laboratory]]. Its actual local commands were:

```sh
g++ -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  tools/labs/asset_container_lab.cpp -o /tmp/allmight_m015_san
/tmp/allmight_m015_san

clang++ -std=c++20 -O1 -Wall -Wextra -Wpedantic -Werror \
  tools/labs/asset_container_lab.cpp -o /tmp/allmight_m015_clang
/tmp/allmight_m015_clang

g++ -std=c++20 -O2 -DNDEBUG -Wall -Wextra -Wpedantic -Werror \
  tools/labs/asset_container_lab.cpp -o /tmp/allmight_m015_release
/tmp/allmight_m015_release
```

Each executable reported `asset_container_lab: all checks passed`. The GCC sanitizer run reported no address or undefined-behavior diagnostics. The locally executed file is 292 lines, 13,525 bytes, with Git blob `c9a323bcf4a3f2e41df13db9d10d381bdd71468f`; the committed source was checked against that exact blob. These results support the **specific representative CPU-side tests**. They do not establish stress-tested concurrency, a portable allocator performance ranking, exhaustive exception injection for arbitrary user hashers, a stable ABI layout, true GPU lifetime safety, or the current implementation of the separately named Kairo codebases.

**Blank-screen reconstruction:** derive the difference between vector size and capacity, the amortized geometric-series cost and the invalidation rules of appending with versus without reallocation. Explain why a raw pointer cannot safely serve as a long-lived asset identity, then implement a slot-and-generation handle with a retire/reuse check. Choose a contiguous store, an ordered name index and a hash index for different requirements, showing what you gain and lose in lookup order, locality and worst-case behavior. Construct a constant-hash collision demonstration and derive the load-factor equation. Use a deque for both-end work, list splice for node transfer and forward_list's predecessor-based removal; then build FIFO, LIFO and priority scheduling with explicit tie semantics. Finally explain why a span of bytes and a string view are *borrowed*, why their lifetime cannot outlast their owner or a reallocation, and why short-string optimization cannot be inferred as a portable fixed-size promise. Recreate the two injected transaction failures and verify that an old handle never resolves after its slot is reused.

This is an **authored reconstruction** of M015, original Parts 231–250. Every original title is mapped to its first-needed explanation in [[Supplementary/Foundations#M015 — Parts 231–250: growing the asset registry without losing identities|the coverage ledger]], and the evidence boundaries are recorded in [[Supplementary/Sources and Code Anchors#Phase 5 additive reconstruction — M015 / Parts 231–250|the verification ledger]]. The associated [[Visual Recall/02 - C++ - Objects, Ownership and Lifetime - Visual Recall#A growing asset store must preserve its promises — M015 retrieval|recall exercise]] asks the reader to rebuild the mechanism, not repeat a glossary. The original curriculum's `KairoECS/SparseSet.cppm` and `KairoRenderer/RenderGraph.cppm` remain **project-specific audit targets**, not claims about their currently inspected source.

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/03 - Game Engine/00 - Reading Map|Reading map]] · [[Chapter Edition/03 - Game Engine/04 - Resource ownership, failure paths, and review practice|Previous chapter]] · [[Chapter Edition/03 - Game Engine/06 - Queries, lazy ranges, allocators, and module boundaries|Next chapter]]
