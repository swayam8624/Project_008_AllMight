# Mastery evidence levels

PROJECTALLMIGHT should not use one word such as “developed” for fundamentally different evidence states. A concept can be well explained but not executable, executable in a CPU reference but not on the native GPU, or validated in a teaching lab but not in the current production repository.

Use the following evidence ladder.

## 0 — Indexed

The topic has a real node, canonical name, prerequisites and destination. This proves discoverability only.

## 1 — Explained

The forcing problem, definitions, conventions and causal mental model are stated without relying on unexplained jargon.

## 2 — Derived and traced

The central mathematics or algorithm is derived with symbol meanings, assumptions and at least one nontrivial worked trace containing intermediate state.

## 3 — Reference implemented

A complete, rebuildable implementation exists in the teaching surface or a linked standalone laboratory. Positive and negative cases execute.

## 4 — Independently verified

The mechanism is checked against an oracle, invariant, property test, differential implementation, numerical bound or external normative reference strong enough to expose plausible bugs.

## 5 — Native-system verified

Claims involving a real API/device/tool are exercised on that system: Vulkan validation/device output, Metal GPU capture, Blender/Maya/Houdini/Nuke host execution, real filesystem semantics, real network stack, and so on.

## 6 — Production-repository grounded

The current Kairo/Maveb/Vulkax/Nanoquant or other target repository revision is inspected and the exact implementation, ownership, lifetime, synchronization, performance and failure paths are tied back to the lesson.

## 7 — Reconstructed mastery

The learner can derive, implement, trace, profile, debug and compare alternatives without copying the source, and can solve a transfer problem in a different context.

A concept can hold several evidence dimensions separately. For example, Vulkan synchronization may be Derived=Yes, CPU reference=Yes, Native Vulkan=No, Kairo parity=Unverified. Do not collapse that into one optimistic “complete” flag.

## Required proof fields

For each M001–M180 chunk, the evidence ledger should record:

- canonical section;
- original part range;
- definitions and conventions;
- derivation;
- worked trace;
- executable/reference artifact;
- positive/negative/property tests;
- alternatives/failure modes;
- external source review;
- native platform evidence if applicable;
- production repository evidence;
- research bridge;
- retrieval/oral reconstruction status.

The purpose is not bureaucracy. It prevents a three-megabyte note from masking one missing proof obligation.
