---
tags:
  - concept
  - foundation
---

# Atomic publication and durability

**Curriculum depth:** Introduced · **First seen:** C003 · **Latest development:** C003

## Definition and mechanism

Publishing a prepared temporary file by replacement concerns observers of the filesystem namespace. Crash durability concerns whether contents and namespace updates survive failures; it needs a specified platform/filesystem protocol. Neither follows from building a vector of bytes. The supplied AtomicFile excerpt is an unverified repository claim, not a tested durability guarantee.

## Prerequisites

- [[Serialization]] — Publication persists already-specified bytes, rather than defining their format.
- [[Ownership and non-owning views]] — A mapping or publication pipeline must keep referenced bytes valid.

## Used by

Update downstream uses when later chunks build on this concept.

## Recall and next depth

Name the observers and failure model for an atomic guarantee.

**Pending:** Platform replacement semantics, flush ordering, recovery and cross-volume limits.

## Source and navigation

[[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#Where the bytes live|C003 teaching chapter]] · [[Supplementary/Foundations#M003 - Parts 41-60|Part definitions]] · [[Dependency Map]]

## Executed POSIX durability sequence

The durability contract now has an actual syscall-level witness in `tools/labs/atomic_publication_durability_lab.cpp`.

The reference creates a staging file with exclusive creation, writes the complete payload, calls `fsync` on the file, closes it, renames it within the same directory, opens the containing directory, and calls `fsync` on that directory before treating publication as complete. It then verifies the final bytes and absence of the staging name. Separate tests reject a pre-existing staging file and a traversal-style final name.

This is stronger than “write temp then rename,” because rename atomicity and durable persistence are different properties. The file sync requests persistence of file contents/metadata; the directory sync requests persistence of the directory-entry change. Successful calls on CI are evidence that the operating-system/filesystem accepted that ordering. They are **not** a laboratory power-cut test and cannot promise identical persistence guarantees on every filesystem, storage controller, Windows API, network filesystem or production asset pipeline.

