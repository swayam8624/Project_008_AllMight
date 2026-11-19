# Preface

A program is not a collection of syntax rules. It is a chain of interpretations and obligations: physical states become bits, bits become values, values occupy storage, objects acquire lifetimes, algorithms transform state, and devices execute work under contracts that are sometimes quite different from those of a single CPU thread. A program may compile correctly and still mishandle a file, retain a pointer past the lifetime of its referent, submit work that observes the wrong memory, or produce measurements that do not support the claim being made. The purpose of this book is to make those boundaries explicit and testable.

We begin with representation because everything that follows depends on it. We then use C++23 to investigate object identity, construction, destruction, borrowing, generic interfaces, error handling and resource management. A small binary-machine project provides one recurring concrete setting, but it is not the only setting: graphics assets, data structures, CPU traces, parallel task graphs, GPU buffers and real graphics APIs supply contrasting examples. The technical argument should survive the change of application. A reader who understands the representation and lifetime contracts of a cartridge, for example, should be able to recognize analogous risks in an image loader or a command buffer without treating these as identical data formats.

The later chapters extend the discussion beyond the language. A GPU is not merely a faster CPU with more threads; locality, execution divergence, barriers, visibility, queues and resource states become first-class design constraints. Vulkan and Metal provide concrete but non-interchangeable ways to express those constraints. Throughout, the emphasis is on what the program promises, what the implementation actually does, and which experiment could falsify a mistaken assumption.

This is a working academic edition developed from the preserved Computing Foundations manuscript and 18 additive C++23 laboratories. The current edition includes the first sixteen of the manuscript's thirty-nine existing chapter-edition chapters. It is not a condensed substitute for the remaining mathematical, optical and geometric chapters, and it is not a claim to cover every library facility or wording detail of the ISO C++23 standard. The project laboratories demonstrate bounded contracts; they do not certify an entire emulator, game engine, rendering system or research pipeline. These distinctions are important, not editorial disclaimers: technical education depends on knowing the boundaries of the evidence.

# Intended Audience and Prerequisites

This book is written primarily for undergraduate and graduate students in computer science, computer engineering and graphics; working programmers moving from high-level application code into native systems; and researchers who need to inspect, implement and defend experimental software. It is also intended for motivated independent readers who prefer to derive and test mechanisms rather than memorize a glossary.

The opening chapters develop binary representations and core language ideas from first principles. Nevertheless, the exercises are substantial. Comfort with basic algebra and a willingness to compile, run and debug C++ programs will help. Later GPU chapters presume that the reader is willing to follow pointers, object ownership, concurrency, memory models, mathematical notation and API specifications carefully. The book explains these mechanisms, but does not remove the need to perform the experiments.

For an introductory programming course, instructors may use the representation and ownership sections selectively. For an advanced systems, graphics or research-software course, the CPU-to-GPU progression offers a common thread for code review, laboratory work and experimental design.

# How to Read and Use the Laboratories

A technical explanation becomes useful when you can reconstruct it. For each substantive mechanism, start by stating the problem and the permitted assumptions in ordinary words. Predict the behavior of the smallest interesting example before running it. Then inspect the implementation, explain the invariant it maintains, and try at least one test that should fail. When performance is measured, keep the correctness oracle separate from the timing path and record the environment that produced the observation.

Code listings are meant to be read with their surrounding contracts, not copied without context. Some listings are complete reference programs; others deliberately isolate a particular operation or failure boundary. Treat the stated compilation mode, environment and validation limits as part of the example. A sequence of successful tests is valuable evidence for the tested cases; it is not proof that every possible input or target architecture behaves identically.

Figures show relationships that are easier to inspect visually: movement of data, object and resource lifetime, ownership transfer, command submission and dependencies. They are part of the explanation. If a figure or mathematical expression is missing, the release is incomplete and should not be distributed. A page budget is never a justification for deleting the underlying reasoning.

# The Two-Part Intellectual Route

The first half follows the path from bits to reliable executable programs. Its central ideas are interpretation, numerical representation, object layout, value semantics, lifetime, typed interfaces, persistent identity, data structures and CPU execution. The associated laboratories repeatedly ask whether the bytes, the object and the program agree about what is valid.

The second half follows the path from executing programs to parallel devices. Its central ideas are memory access, ordering, reproducibility, asynchronous execution, buffer lifetime, GPU resource contracts and graphics API integration. It uses the CPU-side evidence model from the first half to reason about hardware that may complete work after the host has returned from a call.

The digital reading edition presents this route as a continuous book. When the printed page count requires two KDP paperback volumes, the split occurs at the transition from algorithms and CPU-side dependencies to GPU memory and parallel execution. Both volumes belong to a single curriculum; the split is a publishing constraint, not a conceptual boundary.

# Conventions, Limitations, and Further Study

The book distinguishes an illustrative machine from a specified real hardware architecture, an API rule from a hardware assumption, a mathematical model from a measurement, and a sample implementation from a verified product. Numerical quantities retain their units. Code and formulas should preserve their original meaning when typeset; unresolved diagram, math, font, cross-reference or page-overflow problems are release blockers.

Readers seeking a complete treatment of the C++23 language and library should supplement this work with the ISO standard, appropriate implementation documentation, and language-focused references. Likewise, serious GPU programming requires current API specifications and device-specific profiling rather than a single generalized memory diagram. The goal of this book is not to replace those sources; it is to equip the reader to use them critically and accurately.

The concluding mastery laboratory is not a certificate. It is a set of questions and experiments through which a reader can demonstrate a more valuable ability: to identify the contract, produce evidence, discover where it fails, and revise the implementation without confusing an observed result with a universal guarantee.
