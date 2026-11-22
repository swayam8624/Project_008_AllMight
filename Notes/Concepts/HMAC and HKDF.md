# HMAC and HKDF

HMAC authenticates a message under a shared secret key. HKDF uses HMAC to turn input keying material into one or more pseudorandom-looking keys with explicit context separation.

## Extract then expand

HKDF has two conceptual stages.

Extract computes a pseudorandom key:

$$
PRK=\operatorname{HMAC}_{salt}(IKM).
$$

Expand derives output keying material in blocks:

$$
T(1)=\operatorname{HMAC}_{PRK}(info\,\|\,0x01),
$$

$$
T(i)=\operatorname{HMAC}_{PRK}(T(i-1)\,\|\,info\,\|\,i).
$$

Concatenate blocks until the requested output length is reached.

The info field is not decoration. It lets the same root keying material derive separate keys for different purposes, versions, peers or directions. Domain separation reduces accidental cross-protocol key reuse.

A salt can improve extraction and should follow the protocol design. HKDF does not create entropy that is absent from the input keying material.

## Evidence

tools/labs/crypto_primitives_lab.cpp validates HMAC-SHA256 against RFC 4231 and HKDF-SHA256 extract/expand against RFC 5869 test vectors.

That proves the bit-level reference on those vectors. It does not establish side-channel resistance, secret erasure, key storage, protocol context selection or application security.

Primary standards: RFC 2104 / RFC 4231 for HMAC testing and RFC 5869 for HKDF.

Related: [[Integrity and authenticity]], [[Checksums CRCs and hashes]].
