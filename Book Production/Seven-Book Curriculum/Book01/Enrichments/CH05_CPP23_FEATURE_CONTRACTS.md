# Modern C++23 as a Set of Interface Contracts

A reader can understand lifetimes, RAII and templates without yet understanding what the C++23 standard adds. This bridge makes that distinction explicit. Some facilities discussed here first appeared in C++20 and remain foundational to C++23 programs; others are new in C++23. We distinguish the language requirement, the library facility, and the availability of a particular standard-library implementation rather than treating the compiler's `-std=c++23` option as proof that every header is implemented.

## Borrowing and spans: convenient syntax does not create ownership

A `std::span<T>` is a non-owning view into contiguous elements. It stores enough information to identify a sequence, but it does not keep the array alive. Returning a span into a local vector creates a lifetime bug even if the compiler accepts the source. A safe design either retains an owner at the correct scope or returns an owning value.

For processing a buffer supplied by a caller, a span can provide a useful explicit boundary. The function's contract still needs to say whether modification is allowed and whether the buffer can be concurrently accessed. The word *view* is not a synchronization primitive.

## Expected values make recoverable failure visible

In C++23, `std::expected<T,E>` represents either a result of type `T` or an error of type `E`. This is not a promise that the operation cannot throw at all: allocation, construction and other operations may still throw. It makes the routine's ordinary recoverable error channel explicit so callers must decide how to handle an absent value.

The following complete program decodes an eight-bit record and reports the distinction between invalid data and a valid zero.

```cpp
// expected_record.cpp — C++23 standard library implementation required
#include <cstdint>
#include <expected>
#include <iostream>
#include <span>
#include <string_view>
#include <vector>

enum class DecodeError { too_short, unsupported_version };

std::expected<std::uint8_t, DecodeError>
decode(std::span<const std::uint8_t> bytes) {
    if (bytes.size() < 2) {
        return std::unexpected(DecodeError::too_short);
    }
    if (bytes[0] != 1) {
        return std::unexpected(DecodeError::unsupported_version);
    }
    return bytes[1];
}

int main() {
    const std::vector<std::uint8_t> valid{1, 0};
    const std::vector<std::uint8_t> invalid{2, 5};

    auto first = decode(valid);
    auto second = decode(invalid);
    if (!first || second) return 1;

    std::cout << static_cast<unsigned>(*first) << '\n';
}
```

Build with:

```sh
c++ -std=c++23 -Wall -Wextra -Wpedantic -Werror expected_record.cpp -o expected_record
./expected_record
```

Expected output: `0`. The invalid case is an explicit error, not an integer whose value happens to be zero. In a more complete decoder, limits and version checks should precede any allocation or arithmetic dependent on an untrusted length.

## Concepts state the syntactic requirement; implementations enforce the semantic contract

A concept can reject a type that does not support the intended operations. It cannot prove the programmer's undocumented semantic assumptions, such as whether an iterator remains valid after a mutation. This is especially important for APIs involving identities, borrowing, and resources.

For example, `std::ranges::contiguous_range<R>` requires an appropriate contiguous range interface. `std::ranges::range_value_t<R>` supplies its element value type. These facilities make generic algorithms easier to constrain, but the caller still owns the underlying storage and the called function still must define its borrowing and mutation rules.

The Ranges library includes lazy views: composing a filter or transform does not necessarily perform the computation immediately. A view may retain references to its source. Never assume that writing a pipeline takes a snapshot of its input. Test invalidation and the lifetime of the source range, especially when returning a view from a function.

## A map of important C++23 additions

| Facility | C++ level | The problem it addresses | Implementation caveat |
|---|---|---|---|
| `std::expected` | C++23 | Ordinary result-or-error values | Check library header/version |
| `std::generator` | C++23 | Lazy synchronous coroutine sequence | Library support may lag compiler flag |
| `std::mdspan` | C++23 | Non-owning multidimensional data layout | Check header and library feature macro |
| `std::print` | C++23 | Formatted direct output | Availability depends on library |
| `std::move_only_function` | C++23 | Type-erased move-only callable | Check library implementation |
| `std::flat_map` and related adapters | C++23 | Flat storage with associative interface | Check library support |
| `std::ranges::to`, `views::zip` | C++23 | Range materialization and composition | Feature-specific support varies |
| Explicit object parameters | C++23 language | Expressing receiver-type requirements | Check compiler language support |
| Multidimensional subscript operator | C++23 language | Native syntax for indexed multidimensional objects | Confirm language support |
| `if consteval` | C++23 language | Distinguishing manifest constant evaluation | Compiler support and design semantics |
| `std::atomic_ref`, `std::barrier` | C++20 foundation | Atomic access to suitably aligned storage; coordinated phases | **Not** new C++23 features |
| `std::span`, concepts, ranges | C++20 foundation | Borrowing and constrained generic algorithms | **Not** new C++23 features |

This matrix deliberately avoids pretending that the book exhaustively covers every clause of ISO/IEC 14882. A complete language-reference edition requires a clause-by-clause standard matrix and independent compiler checks. The present goal is to connect major facilities to the actual systems contracts established earlier.

## Coroutines are not threads

A coroutine can suspend and resume an invocation while retaining a frame. This does not imply that its work is parallel or executes on another operating-system thread. A `std::generator` offers a synchronous sequence of produced values; its consumer still has lifetime obligations to the generator object and values it exposes. A GPU dispatch is a different execution system entirely.

The compiler transforms a coroutine into stateful machinery subject to the language's suspension and destruction rules. A useful first exercise is to identify which objects survive suspension, which references remain borrowed, and which operation destroys the coroutine frame. Do not add concurrency merely because the control flow uses `co_yield` or `co_await`.

## Atomic references and phase barriers solve different problems

An `std::atomic_ref<T>` provides atomic operations on a suitably aligned existing `T` object under its rules. It does not retroactively make every non-atomic access to that object safe. A `std::barrier` coordinates participating threads across phases. Neither statement is a description of a Vulkan pipeline barrier or a Metal resource dependency.

When porting an algorithm to a GPU, define a producer, its output domain, the consumers, and the ordering needed for the consumers to observe the data. Then select mechanisms whose scopes satisfy those obligations. Copying a similarly named C++ synchronization primitive into an API design is not a proof.

## When to use modules

C++20 modules and C++23 build systems both require an explicit dependency model. The source interface unit, import graph, build tool and compiler's module artifacts are all part of reproducibility. Replacing headers with `import` without a valid build graph can make incremental builds less reliable, not more. A laboratory should check both a cold build and a rebuild after an interface change.

## The reconstruction test

For each row in the feature table, explain whether the guarantee comes from language wording, a library class, a compiler implementation, or a platform API. Which examples allocate? Which views own their data? Which operations are permitted to throw? What evidence would show the claim fails for a given toolchain? A student who can answer those questions has learned a more durable skill than memorizing a feature list.


## A non-owning matrix view is not a matrix allocation

A graphics program often has a rectangular array of values but wants to interpret it as rows and columns. C++23 `std::mdspan` makes that distinction more explicit: the owner allocates the elements, while the view records a mapping from multidimensional indices to an underlying contiguous or strided address space. The mapping is not an allocator, and it does not prolong the owner's lifetime. It is therefore a useful language counterpart to GPU buffer views.

Consider a row-major array storing six values, interpreted as two rows and three columns. The conceptual address formula is

$$
\operatorname{offset}(r,c)=3r+c,\qquad
0\le r<2,\quad 0\le c<3.
$$

For example, `(r,c)=(1,2)` selects offset five. In a different mapping policy, the same two-dimensional index can map to a different offset. That is why a multidimensional *shape* is not equivalent to a byte layout.

```cpp
// Requires a standard library with C++23 <mdspan> support.
#include <array>
#include <cstddef>
#include <mdspan>

int main() {
    std::array<float, 6> owner{1, 2, 3, 4, 5, 6};
    using extents_t = std::extents<std::size_t, 2, 3>;
    std::mdspan<float, extents_t, std::layout_right>
        matrix(owner.data());
    return matrix[1, 2] == 6.0f ? 0 : 1;
}
```

This is deliberately a *feature-gated* complete example: it requires the target library's `<mdspan>` header and support for the C++23 multidimensional subscript operator. A compiler that accepts `-std=c++23` without those library features is not necessarily broken. Check implementation release notes and relevant feature-test macros before using this example as a CI requirement. In a renderer, a similar view is valid only as long as the backing data and its chosen synchronization contract remain valid.

## Explicit object parameters make receiver types visible

A member function normally has an implicit object argument called `this`. The C++23 explicit object parameter feature permits spelling out that receiver in the parameter list. This allows an interface to express certain constness, value-category and generic-receiver requirements directly.

```cpp
// Illustrative C++23 language feature; check compiler support.
struct Sample {
    int magnitude = 0;
    int read(this Sample const& self) {
        return self.magnitude;
    }
};
```

Here the const receiver prevents mutation through `self`. It does not make the containing object's storage immutable to every other alias or thread. If a member returns a reference, a caller still needs to understand the object's lifetime. The feature changes how a contract can be *expressed*; it does not remove lifetime and concurrency obligations.

To test a toolchain, consult the `__cpp_explicit_this_parameter` feature-test macro when it is provided. Write a negative test that attempts a forbidden mutation and confirm that the compiler rejects it. Do not infer language availability from a marketing name or from the package manager's compiler version alone.

## Lazy generators, frames, and borrowed elements

A coroutine function can suspend while retaining a compiler-managed frame containing its state. C++23 provides `std::generator` as a synchronous coroutine sequence abstraction in supporting standard libraries. It is not a worker thread, GPU dispatch, or implicit asynchronous scheduling service.

```cpp
// Illustration; requires C++23 <generator> library support.
#include <generator>

std::generator<int> indices(int count) {
    for (int i = 0; i < count; ++i) {
        co_yield i;
    }
}
```

A consuming loop repeatedly resumes the generator, observes a value and eventually destroys the coroutine state. References yielded from a coroutine require particular care: the referent must stay alive and accessible across the consumer's use. This is the same ownership rule from the earlier object-lifetime chapters, expressed through a different control-flow mechanism. The frame's destruction is a concrete event worth tracing, not merely a consequence of leaving a source-code scope.

**Experiment.** Instrument the constructor, move and destructor of an object held in a generator frame. Create a sequence, consume only part of it, then destroy the generator. Predict exactly which object destructions are observable. Repeat with a deliberately borrowed source whose owner outlives the generator; do not invoke undefined behavior as a way of checking whether a borrow is valid.

## Move-only callbacks and resource ownership

A callable that owns a `std::unique_ptr` cannot generally be copied, because copying would violate its exclusive ownership. C++23's `std::move_only_function` is a type-erased callable wrapper designed to accommodate move-only targets. This is useful for resource cleanup and asynchronous work, provided the callable's execution and destruction lifetimes are still explicit.

```cpp
// C++23 library support required for std::move_only_function.
#include <functional>
#include <memory>

std::move_only_function<int()> make_reader(int value) {
    auto owner = std::make_unique<int>(value);
    return [owned = std::move(owner)] { return *owned; };
}
```

The callable owns its captured integer; it does **not** guarantee that a GPU command using an unrelated buffer has finished. Using move-only type erasure can simplify a host-side ownership boundary, but device completion needs its own proof. A failed attempt to copy the returned callback should be an intentional compile-time rejection in a conformance test.

## Three independent implementation checks

Whenever a new C++23 facility enters an example, record three separately verifiable claims. First, state the **language rule** or library contract, ideally with the relevant standard section. Second, state which **compiler and standard library** provide the feature and how you verified it; library availability may lag parsing support. Third, state what **runtime behavior** was exercised, including input sizes, failure paths and any platform-dependent assumptions.

For example, a C++23 program that contains `std::expected` can prove that an error is handled without using an ambiguous success value. It cannot, by merely compiling, establish that every error path is allocation-free, that an unrelated `std::span` remains valid, or that a GPU buffer has retired. The precise separation of those claims is more valuable than memorizing a long feature checklist.

The complete ISO C++23 language and library are larger than this chapter. For unexplained facilities, use the clause-level language reference and the implementation's documented feature-test macros to build a coverage record rather than guessing that a symbol is supported or assuming the current chapter is exhaustive.
