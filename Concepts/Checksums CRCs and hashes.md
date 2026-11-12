---
tags:
  - concept
  - foundation
---

# Checksums, CRCs, and hashes

**Curriculum depth:** Reference implemented and known-answer tested · **First seen:** C006 · **Latest development:** Phase 28 repair

A checksum or digest maps an arbitrary byte sequence to a smaller fixed-size value. Different families make different promises.

An additive checksum is mainly an accidental-error detector and is weak against structured changes such as reordering or compensating byte modifications.

A CRC interprets bits as a polynomial over $GF(2)$ and computes a remainder under a declared generator polynomial and parameter set. CRCs are excellent for many transmission/storage error patterns, but they are not cryptographic authenticators.

A cryptographic hash such as SHA-256 is designed so that finding collisions or preimages is computationally hard under the security assumptions of the primitive. A hash still does **not** authenticate a file when an attacker can replace both the file and the untrusted hash value.

## SHA-256 compression structure

SHA-256 pads the message with a one bit, enough zero bits to leave 64 bits at the end of the final 512-bit block, and then the original message length modulo $2^{64}$. Each 512-bit block expands sixteen 32-bit words into a 64-word schedule and updates an eight-word chaining state through 64 rounds.

The important learning boundary is representation: message words are interpreted in the algorithm’s specified byte order. Reinterpreting native host integers without explicit conversion can produce a platform-specific hash implementation.

## Known-answer evidence

tools/labs/crypto_primitives_lab.cpp implements SHA-256 directly and validates the standard digest of “abc” and the empty message. The same SHA-256 implementation is then used by HMAC, HKDF and a domain-separated Merkle tree.

This raises the evidence above “definition present,” but the implementation is educational. It is not reviewed for constant-time behavior, hardware acceleration, fault attacks, side channels or production cryptographic use.

## What to use for what

Use a checksum/CRC when the threat is accidental corruption and the protocol specifies the polynomial/parameters.

Use a cryptographic hash when content identity or collision/preimage resistance is required.

Use a MAC such as HMAC when a party holding a secret key must authenticate content.

Use a digital signature when verification must be possible without sharing the signing secret.

## Prerequisites

[[Bitwise XOR]], [[Serialization]], [[Endianness]].

## Used by

[[Integrity and authenticity]], [[HMAC and HKDF]], [[Merkle trees and authenticated structure]], [[Production manifests and transactional publication]].

## Source and navigation

[[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#Bytes become a durable agreement|Continuous teaching story]] · [[Dependency Map]]
