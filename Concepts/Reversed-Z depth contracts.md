# Reversed-Z depth contracts

Reversed depth maps near geometry to larger depth values and far geometry toward zero. Its strong precision benefit comes from pairing reciprocal-distance projection with floating-point storage. Clear values, comparisons, occlusion bounds and reconstruction must agree.

Teach and reconstruct: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Give depth an explicit meaning]].

Related integration record: [[Supplementary/Manual Integration - Sources and Verification]].

## Mastery expansion — reverse the comparison and projection together

Depth precision is governed by the projective mapping and floating-point representation, not merely by choosing a large far plane. Reversed-Z maps the near plane toward the larger depth value and far/infinity toward the smaller one, typically using a greater/greater-equal depth comparison and a projection matrix derived for the API's clip/NDC convention.

The implementation contract must keep projection, depth clear value, comparison operator, depth reconstruction and shadow/depth consumers consistent. Reversing only one of these pieces silently breaks visibility.

Floating-point depth benefits because values are denser near zero in representable spacing while perspective already concentrates geometric resolution near the camera; reversing can make those effects complement rather than oppose each other.

**Required next witness:** numeric comparison of standard versus reversed depth across logarithmically spaced camera-space distances plus a projection/reconstruction round trip for Vulkan and Metal conventions.

