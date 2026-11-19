# Preface to Volume II

The questions change when the work moves from a CPU thread into a collection of GPU invocations. The correctness of a program now depends not only on what a computation does but also on which invocation owns each output, where memory resides, whether the operations are visible to their consumers, how commands are ordered, and whether a resource survives until the last device operation that uses it.

Volume I established the habits this volume depends on. Binary formats have declared interpretations. C++ objects have lifetimes that cannot be inferred merely from their addresses. Ownership is a responsibility for keeping a resource usable, not a synonym for holding a pointer. A graph of operations is useful because its dependencies constrain valid execution orders. We now apply the same ideas to GPU kernels, command queues, descriptors, synchronization, and performance experiments.

The route begins with memory spaces, coalescing, divergence, subgroups, atomics, and parallel scans. It then establishes the build and measurement discipline needed to compare implementations. Borrowing and CPU–GPU buffer lifetimes connect language-level safety to asynchronous execution. Vulkan makes explicit the device, allocation, descriptor, pipeline, command, and synchronization objects involved in a frame. Metal supplies a contrasting programming model, with its own resource, encoding, and ordering guarantees. Finally, profiling closes the loop: an optimization is only established when it is correct and its measured effect survives appropriate controls.

## Prerequisites and expected skills

Before using the Vulkan or Metal chapters as implementation instructions, a reader should be able to explain how C++ ownership and borrowing differ, why a capacity check must occur before indexing, how a producer–consumer dependency constrains scheduling, and why a device command may outlive the host call that submitted it. If any of those explanations feels uncertain, revisit the corresponding chapters of Volume I rather than memorizing synchronization API names.

The GPU examples distinguish three kinds of claims: an architectural principle, a guarantee made by a particular API, and an observation measured on a particular GPU. That distinction matters especially on unified-memory computers. Sharing physical memory does not make all access patterns equally fast, remove resource-lifetime obligations, or imply that a read always observes a preceding write without the necessary ordering.

## The practical investigation

When following a kernel example, begin with the sequential mathematical problem. Specify a correctness oracle. Next describe the mapping of data and work to threads, threadgroups, and resources. Identify every place two operations may conflict; decide which ordering is required and by whom it is guaranteed. Finally measure the bottleneck that the model predicts, not simply whichever timer happens to be easiest to call.

For cross-API examples, do not translate a Vulkan barrier directly into a Metal method merely because both are described as synchronization. Compare their semantic obligations, memory domains, and execution scopes. Platform capabilities, versions, and device feature checks remain part of the evidence.

The laboratory at the end asks readers to follow one dependable chain of reasoning from external bytes through execution and measurement. Its value lies in an independently reproducible explanation. A result should remain understandable to another engineer who has not read the author's development notes or operated the original project.

## Reading conventions and references

This is Volume II of **Foundations of Computing Systems**. Chapter numbers continue from Volume I so that references between books remain stable. Definitions of foundational terms and the consolidated source bibliography appear in the shared back matter. The companion repository contains source material and project laboratories; a statement about a private or changeable project is a source snapshot, not a normative statement about Vulkan, Metal, or ISO C++. Use the linked primary specifications for implementation-sensitive details.
