---
tags:
  - concept
  - foundation
---

# Checksums CRCs and hashes

**Curriculum depth:** Developed · **First seen:** C006 · **Latest development:** C006

## Definition and mechanism

Checksums/digests summarize specified bytes. Additive checksums miss reorderings; CRCs use GF(2) polynomial remainders and declared parameters. Cryptographic hashes need a trusted reference to support integrity claims against substitution.

## Prerequisites

- [[Bitwise XOR]] — supplies the representation or validity rule used above.
- [[Serialization]] — supplies the representation or validity rule used above.

## Used by

- [[Integrity and authenticity]] — uses this mechanism to define its representation, error or validation contract.

Connect later dependent concepts here when their explanations use this mechanism.

## Recall and next depth

Reconstruct the example and identify the condition that would invalidate it.

**Pending:** No cryptographic primitive or production integrity container was implemented.

## Source and navigation

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Bytes become a durable agreement|Continuous teaching story]] · [[Supplementary/Foundations#M006 - Parts 89-100|Source coverage ledger]] · [[Dependency Map]]
