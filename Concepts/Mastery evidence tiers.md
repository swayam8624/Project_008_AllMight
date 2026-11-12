# Mastery evidence tiers

PROJECTALLMIGHT uses multiple evidence tiers because different tests prove different claims.

**Canonical teaching** means the reader-facing material contains the forcing problem, definitions, derivation, representation, algorithm, worked trace, failures and alternatives.

**Executable reference** means a small independent program reconstructs the core mechanism and tests its invariants. It proves reference logic, not a production integration.

**Native backend evidence** means the mechanism was executed through the real API/hardware boundary, such as Vulkan validation, Metal resource behavior or GPU timestamps.

**Production-repository evidence** means the current Kairo/Maveb/Vulkax/Nanoquant code was inspected and reproduced at a pinned revision.

**Source verification** means an external source claim was checked against its actual page, transcript, specification or paper rather than merely inherited from extracted notes.

A chunk may be strong at one tier and weak at another. Do not collapse those states into a single word such as “done.”

The machine-readable accounting lives in `mastery/mastery_ledger.json` and is checked by `tools/check_mastery_ledger.py`.

**Next depth:** map every M001–M180 chunk to exact section anchors, executable witnesses and pinned production commits.
