# Preface

Software is a sequence of agreements between representations and machines. A physical state represents a bit; bits are interpreted as numbers, instructions, addresses, or records; a compiler translates a source program into executable operations; and a running program must continue to respect object lifetimes, data dependencies, and the devices that carry out its work. Errors commonly occur where one of these interpretations changes. The same bytes may describe an integer, part of a floating-point value, an instruction, or an invalid serialization. The same address may refer to live storage, an object whose lifetime has ended, or a buffer still being read by a GPU.

This textbook develops a way of reasoning across those boundaries. We begin with representations and finite arithmetic, proceed through C++23 types and resource ownership, introduce algorithms and reproducible experiments, and then examine the different execution and memory contracts of parallel processors, Vulkan, and Metal. The central habit is to state what must remain true, find the mechanism that enforces it, and devise a test that could reveal an incorrect assumption.

Worked examples recur in different settings. A small binary-machine laboratory makes bytes and instructions observable. An asset registry shows why an identifier must not be confused with a pointer. GPU kernels show how a correct serial algorithm may require changes to storage, synchronization, and work decomposition before it can scale. A recurring example is useful only if the reader can transfer its underlying argument to another program: a texture importer, a physics simulation, or a numerical experiment must still obey its own contracts.

The text does not ask readers to trust a sample because it compiled once. A test is evidence about specified inputs in a specified environment; a language or API guarantee must instead be traced to the relevant rule or specification. Devices and implementations evolve, so examples explicitly distinguish general obligations from platform-specific observations.

## The seven-book journey

**From Bits to Research** is planned as a coherent, seven-book course in computing and the construction, observation, and scientific study of visual worlds. Each book is intended to make the next possible, rather than treating mathematics, software engineering, physics, rendering, vision, or research as isolated collections of definitions. This first book is **From Bits to Parallel Systems**: it builds the representation, C++23, CPU, GPU, and verification foundations on which the later work depends.

The sequence continues through mathematics for visual computing; geometry, cameras and image formation; light transport and image synthesis; spatial worlds, engine architecture and simulation; GPU architecture and real-time rendering; and vision, XR, experimentation and research communication. These are the seven *teaching stages*, not claims that every later volume is already publication-ready. Print Volume I and Print Volume II of this first book are merely a physical split of **Book One**.

The destination includes not only performing research but **showing it rigorously and beautifully**. Later stages will develop publication-quality plots, carefully designed scientific figures, diagrams of algorithms and data structures, perceptually honest colour and scale choices, 3D experimental scenes, reproducible animations and comparative visual demonstrations. A compelling image is useful when the reader knows how it was generated, what it measures, what is simulated rather than observed, and which alternative interpretation it might conceal. Visual polish should serve truth and understanding, not replace evidence.

The companion programme, source repositories and references are collected in the back matter. The starting point for all seven books is [Project AllMight](https://github.com/swayam8624/Project_008_AllMight).

## Who this book is for

The primary audience is advanced undergraduate students, graduate students, native-software engineers, and graphics or simulation researchers seeking a stronger foundation beneath working code. Readers with minimal programming experience can begin with the representation and machine-model material, but the later chapters demand actual C++ compilation and a willingness to reason carefully about resource lifetime and concurrency.

There are three useful paths through this edition. The **foundations path** follows the chapters in order, completing the binary-machine, C++ ownership, and algorithm laboratories before GPU material. The **systems path**, intended for programmers already comfortable with C++, uses the early chapters as a contract review and concentrates on build reproducibility, synchronization, and failure analysis. The **graphics path** returns to the representation, ownership, and algorithm prerequisites before studying Vulkan and Metal. None of these pathways removes the need to understand what a pointer, an object lifetime, or a dependency means.

## How to work with a chapter

First reconstruct the mechanism in words and identify the assumptions. For a numerical representation, write the bits and derive the value. For a language rule, predict whether the program compiles and which lifetimes are active. For an asynchronous command, draw the timeline showing when a resource can be accessed and when it can safely be reused.

Next reproduce the example with the specified toolchain. Record its inputs and expected behavior before running it. Repeat with at least one boundary case and one deliberately invalid case. For a performance result, separate an independent correctness oracle from the timed region and record the hardware, software versions, data set, warm-up, and measurement method. A faster incorrect result is not an optimization.

Every chapter ends with three explicit obligations: **Reconstruct** the mechanism without copying a diagram, **Falsify** an assumption with a controlled test, and **Transfer** the reasoning to another application. These tasks are not page-filling exercises. They are a check that a concept has become usable. A reader who completes a laboratory should retain its source, inputs, observed behavior, counterexample and environmental record.

Listings are classified as complete programs, fragments, pseudocode, or GPU/API examples where the context makes that distinction material. Source paths and identifiers from the companion projects are examples of real-world engineering structures, not general language requirements. Reconstruct the abstract contract even when a particular project's implementation is unavailable.

## Conventions and scope

In mathematical expressions, a displayed equality asserts a relationship under its stated assumptions. An engineering maxim is not the same thing as a theorem, and a diagram is not a hardware schematic unless it explicitly identifies itself as one. Numerical quantities carry units; finite-width arithmetic is treated as finite-width arithmetic, not as arithmetic over unbounded integers. The terms *host*, *device*, *object*, *storage*, *resource*, *invocation*, and *subgroup* have distinct meanings.

This edition selects the first sixteen chapters of a larger thirty-nine-chapter Computing Foundations manuscript and integrates eighteen independently staged C++23 project lessons. The additional mathematics, geometry, optics, and research-source chapters are separate; this volume does not claim to replace them or reproduce the entire ISO C++23 standard. It is a systems-programming textbook with a strong path into GPU computing, not a standard-library encyclopedia.

The two paperback volumes divide the physical text for readability. The digital edition includes the whole selected sequence. Volume I develops representation, objects, ownership, generic interfaces, identity, and algorithms; Volume II extends the contracts to GPU execution, Vulkan, Metal, and measurement. The boundary is practical, not a claim that CPU and GPU questions can be studied independently.

## Companions, references, and corrections

The book's code and experimental narratives are accompanied by source references and laboratory records. When a named project is discussed, treat its status as the snapshot described by the text rather than a promise about its current repository state. A companion-code catalogue and verification checklist appear in the back matter. External standards and vendor documentation are collected separately so that a reader can distinguish a normative API rule from a teaching model.

A figure or mathematical expression that fails to render, an unreadable code listing, a clipped page, or an unverified command in a purportedly runnable example is a defect to correct rather than an acceptable shortcut. The most valuable result of studying the book is the ability to explain and test a mechanism without needing its original author's confidence as evidence.
