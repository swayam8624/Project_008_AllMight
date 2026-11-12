# Shamir secret sharing

Shamir secret sharing splits a secret into shares so that any threshold number can reconstruct it while fewer shares reveal no information in the ideal finite-field construction when coefficients are chosen uniformly at random.

Choose a prime field $\mathbb F_p$ containing the secret value and a threshold $t$. Construct a random polynomial

$$
f(x)=s+a_1x+\cdots+a_{t-1}x^{t-1}
$$

with secret $s=f(0)$. Give participant $i$ the share $(x_i,f(x_i))$ for distinct nonzero $x_i$.

Any $t$ shares recover $f(0)$ through Lagrange interpolation:

$$
s=
\sum_i y_i
\prod_{j\ne i}
\frac{-x_j}{x_i-x_j}
\pmod p.
$$

Every division is multiplication by a finite-field inverse. Ordinary integer division is wrong.

## Security condition

The coefficients must be generated with a cryptographically secure random generator. Deterministic or predictable coefficients destroy the information-theoretic secrecy claim even though interpolation still reconstructs correctly.

Share identifiers also matter: using duplicate x coordinates makes interpolation invalid.

## Evidence boundary

tools/labs/crypto_primitives_lab.cpp demonstrates a three-share reconstruction over prime field 257 and reconstructs the same secret from two different three-share subsets.

Its polynomial coefficients are deliberately deterministic so the test is reproducible. Therefore the laboratory proves interpolation arithmetic, **not** production secrecy.

A production system additionally needs CSPRNG coefficients, authenticated share transport/storage, versioning, participant identity, compromise/recovery policy and secure erasure.

Related: [[Integrity and authenticity]], [[HMAC and HKDF]].
