# Merkle trees and authenticated structure

A Merkle tree hashes data into leaves and repeatedly hashes child digests into parent digests until one root remains. The root commits to the entire ordered leaf set under the collision-resistance assumptions of the hash and the tree encoding.

A safe design domain-separates leaves from internal nodes, for example:

$$
L_i=H(0x00\,\|\,data_i),
$$

$$
P=H(0x01\,\|\,left\,\|\,right).
$$

Without an unambiguous encoding, different logical tree structures can accidentally serialize to the same byte concatenation before hashing.

## Inclusion proof

To prove that one leaf belongs to a tree, provide the sibling hash at each level and the left/right ordering bits. The verifier starts from the leaf hash and reconstructs the root. Proof size is $O(\log n)$ for a balanced binary tree.

The trusted root must come from a trusted channel or authenticated record. A Merkle proof checked against an attacker-provided root proves nothing about authenticity.

## Odd leaf policy

When a level has an odd number of nodes, the format must specify what happens: duplicate the last node, promote it, or use another encoding. Different policies create different roots.

tools/labs/crypto_primitives_lab.cpp uses an explicit domain-separated construction and duplicates the last digest on odd levels. The test proves determinism and that changing one sample leaf changes the root in the exercised case.

Related: [[Checksums CRCs and hashes]], [[Integrity and authenticity]].
