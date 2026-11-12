# Perspective-correct interpolation

A projected sample has screen-space barycentric weights. An attribute linear on the original primitive is reconstructed by interpolating attribute divided by clip $w$ and reciprocal $w$, then dividing the two results. Already-divided depth follows a different affine screen interpolation rule.

Teach and reconstruct: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Decide which samples a triangle owns]].

Related integration record: [[Supplementary/Manual Integration - Sources and Verification]].

## Mastery expansion — interpolate projective quantities before dividing

Screen-space barycentric weights are affine in the projected triangle, but attributes attached to 3-D vertices generally are not affine after perspective division. If clip-space vertices have (w_i) and screen barycentric weights (\lambda_i), an attribute (a) is reconstructed as

[
a=\frac{\sum_i \lambda_i a_i/w_i}{\sum_i \lambda_i / w_i}.
]

Equivalently, interpolate (a/w) and (1/w) linearly across the screen, then divide. This is why UVs interpolated naively in screen space appear to swim or warp on receding polygons.

The same projective reasoning applies to other varying quantities, but some values—flat IDs, derivatives, normalized directions after interpolation—have additional rules.

**Required next witness:** a CPU raster triangle comparing affine and perspective-correct UVs against an analytically projected 3-D point.

