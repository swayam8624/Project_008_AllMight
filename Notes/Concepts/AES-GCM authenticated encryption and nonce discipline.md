# AES-GCM authenticated encryption and nonce discipline

AES-GCM is an **AEAD** construction: it encrypts plaintext while also authenticating the ciphertext and optional **additional authenticated data (AAD)**. The AAD is not encrypted, but any change to it must invalidate the tag. This is useful for records where headers such as version, object identity or sequence number must remain readable yet cryptographically bound to the payload.

## The contract is ciphertext plus tag, not ciphertext alone

Encryption consumes a key, nonce/IV, plaintext and AAD and returns ciphertext plus an authentication tag. Decryption is not successful merely because bytes were produced. The API must verify the tag and release plaintext only after authentication succeeds.

That boundary is security-critical. A decrypt routine that hands partially decrypted bytes to the caller before tag verification turns an authenticated primitive into an unsafe protocol building block.

## Nonce discipline

For GCM, nonce reuse under the same key is catastrophic rather than a small quality loss. Reusing a nonce repeats the counter-mode keystream relationship and also undermines the polynomial authentication construction. Therefore a system must own a **nonce-generation and uniqueness policy**, not just call AES-GCM.

A 96-bit nonce is the common GCM fast path. It may be generated randomly with collision analysis or constructed from a unique per-key counter/record identity. Whichever policy is chosen must define persistence, crash recovery, key rotation and replay semantics. "Use random IVs" is incomplete without bounding reuse probability and preserving the key/nonce lifecycle.

## AAD and replay are different questions

AAD authentication proves that the supplied header was the header used when the tag was generated. It does **not** by itself prove freshness. A valid old packet can still be replayed. Protocols therefore often authenticate a sequence number or epoch as AAD and separately reject already-consumed or out-of-window sequence values.

## Executable native witness

`tools/native/openssl_aes_gcm_smoke.cpp` uses OpenSSL's EVP interface and checks a standard AES-128-GCM zero-key/zero-IV known-answer case. It then verifies authenticated round-trip behavior, tag tamper rejection, AAD tamper rejection, nonce-sensitive output, and an API rule that clears plaintext on authentication failure.

`.github/workflows/security-native-contract.yml` builds that witness against the current OpenSSL development package in CI. This proves the primitive/API contract on that implementation; it does not prove nonce storage, key management, replay policy, HSM integration, side-channel properties, FIPS configuration or current ReliQuary protocol parity.

Related: [[Integrity and authenticity]], [[Checksums CRCs and hashes]], [[Merkle trees and authenticated structure]], [[Binary format contracts]].
