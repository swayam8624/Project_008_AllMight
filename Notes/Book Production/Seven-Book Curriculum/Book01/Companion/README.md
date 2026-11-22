# Book 1 — executable companion laboratories

These are small **standalone teaching models**, not production game-engine, GPU, Vulkan, Metal, or NES implementations. They demonstrate four contracts studied in the book, using only the standard C++23 language/library and CMake/CTest. The source is intended to remain readable and testable without access to any private application.

| Program | Contract exercised | What it deliberately does not prove |
|---|---|---|
| binary_cpu | Decode bounded instruction bytes and perform an observable 8-bit CPU state transition | Real 6502 or NES instruction timing and compatibility |
| generational_handles | Reject stale handles after a slot is freed and reused | Thread-safe ECS or an infinite generation space |
| parallel_scan | Correct exclusive scan and stable scatter against an independent serial oracle | GPU performance, barrier behavior, or hardware execution |
| retirement_contract | Require completion evidence before a simulated resource is retired | The synchronization behavior of a Vulkan/Metal driver |

## Build and verify

From the repository root on macOS or Linux:

~~~bash
cmake -S "Book Production/Seven-Book Curriculum/Book01/Companion" \
      -B build/book01-companion -DCMAKE_BUILD_TYPE=Debug
cmake --build build/book01-companion --parallel
ctest --test-dir build/book01-companion --output-on-failure
~~~

A successful test run demonstrates the included cases on that host, compiler and library. It is not a substitute for language-conformance tools, sanitizers, fuzzing, device validation or publication proofreading. The programs contain deterministic boundary cases and fail with a nonzero exit status if an expected invariant is violated.

To add sanitizers on a toolchain that supports them, configure a separate build directory with the correct compiler-specific options rather than assuming identical sanitizer support across AppleClang, Clang and GCC. Never run the teaching models with untrusted input without an appropriate input validation and resource policy.

## Book integration and reproducibility

The paperback's code appendix points readers to the main Project AllMight repository and this **Companion** folder. Published releases should pin a commit SHA in the print imprint; the printed master already records a source revision. Linkable references are intended for readers of the published revision, not assertions that future branch heads remain unchanged.

For a coursework submission, save the compiler version, CMake version, exact source revision, test results, any sanitizer configuration, and a brief independent explanation of the violated contract in each negative test.

Do not distribute this as an actual NES emulator or device API implementation. The binary machine uses a deliberately minimal, invented instruction set with four opcodes.
