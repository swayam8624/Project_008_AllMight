---
tags:
  - concept
  - foundation
---

# Integrity and authenticity

**Curriculum depth:** Derived with executable primitive witnesses · **First seen:** C006 · **Latest development:** Phase 28 repair

**Integrity** means detecting unauthorized or accidental modification relative to a trusted reference. **Authenticity** additionally answers who or what was authorized to create the message.

Hashing a file and storing the digest beside it protects against accidental corruption only when the digest itself is trusted. An attacker able to replace both can recompute the hash.

A message authentication code solves a different problem: parties share a secret key $K$ and compute a tag such as

$$
\operatorname{HMAC}_K(m).
$$

Someone without the key should not be able to forge a valid tag for a modified message under the security assumptions of HMAC and the underlying hash.

A digital signature separates signing and verification keys and supports different trust models. The choice between MAC and signature is therefore architectural, not a matter of which API call is available.

## HMAC construction

For hash block size $B$, normalize key $K$ to one block, then compute

$$
\operatorname{HMAC}_K(m)
=
H((K\oplus opad)\,\|\,
  H((K\oplus ipad)\,\|\,m)).
$$

The inner and outer domains prevent the construction from being equivalent to a naive hash of key concatenated with message.

tools/labs/crypto_primitives_lab.cpp checks HMAC-SHA256 against an RFC 4231 known-answer vector.

## Key management is part of authenticity

A mathematically correct MAC does not help if:

- the key is embedded publicly in the executable;
- keys are reused across incompatible protocols without domain separation;
- a compromised key is never rotated;
- comparison leaks useful timing information in an adversarial environment;
- replayed old authenticated messages are accepted without nonce/counter/version policy.

Cryptographic primitives are building blocks, not complete protocols.

## Production boundary

The educational implementations in this repository exist to make the bytes/rounds/contracts reconstructable. Production systems should use vetted cryptographic libraries and platform key-storage facilities rather than treating a teaching SHA/HMAC implementation as hardened security code.

Related: [[Checksums CRCs and hashes]], [[HMAC and HKDF]], [[Merkle trees and authenticated structure]], [[Shamir secret sharing]].
