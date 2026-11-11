# Perspective-correct interpolation

A projected sample has screen-space barycentric weights. An attribute linear on the original primitive is reconstructed by interpolating attribute divided by clip $w$ and reciprocal $w$, then dividing the two results. Already-divided depth follows a different affine screen interpolation rule.

Teach and reconstruct: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Decide which samples a triangle owns]].

Related integration record: [[Supplementary/Manual Integration - Sources and Verification]].
