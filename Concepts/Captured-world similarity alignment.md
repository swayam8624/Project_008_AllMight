# Captured-world similarity alignment

Two reconstructions of the same scene can disagree by a similarity transform even when both are internally consistent. A similarity transform Sim(3) contains rotation $R$, translation $t$ and a uniform scale $s$:

$$
p_B = sRp_A+t.
$$

Rigid SE(3) alignment is the special case $s=1$.

This distinction matters when combining monocular/COLMAP-style reconstruction, device tracking, LiDAR geometry, Gaussian scenes or engine coordinates. A translation/rotation-only fit can look approximately correct near the alignment origin while scale error grows with distance.

## Covariance transformation

A covariance represents second-order spread around the transformed mean. Translation does not affect covariance. Under the linear part $sR$,

$$
\Sigma_B
=
(sR)\Sigma_A(sR)^{\mathsf T}
=
s^2 R\Sigma_A R^{\mathsf T}.
$$

That $s^2$ factor is required because variance has squared units.

## Estimation

With corresponding point sets, similarity alignment can be estimated by centering the data, solving for rotation from cross-covariance, recovering scale from the centered energies, then translation from the transformed centroids. Robust systems must additionally handle outliers, degenerate configurations and coordinate-convention mistakes.

A good alignment report should include more than one mean error. Inspect residual distributions, spatial pattern, held-out correspondences and downstream image/depth consequences. A low training-set residual does not prove the transform generalizes.

## Engine boundary

The alignment should be represented as one explicit typed transform with declared source and destination frames. Applying scale separately to positions while forgetting camera translations, covariance, velocity or distance thresholds creates a mixed-coordinate system.

Related: [[Gaussian covariance projection and footprint]], [[Coordinate frames and change of basis]], [[Conditioning and singularity]].
