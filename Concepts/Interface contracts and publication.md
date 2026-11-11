# Interface contracts and publication

An interface contract states the caller’s valid inputs, ownership, result, failure behavior, timing and concurrency requirements. Publication is the transition that lets consumers rely on a prepared result under those requirements. CPU construction, GPU submission and GPU-read readiness are different events; an obsolete asynchronous request must not publish into a replacement destination.

Read the connected explanation in [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#An API is a promise about an operation]].
