# Sampling support and missing strata

The **support** of a sampling distribution is the set of outcomes it can generate with nonzero probability. Importance sampling can change how often regions are visited, but it cannot estimate a nonzero contribution from a region that the estimator never samples unless another technique covers that region.

For an integral

$$
I=\int_D f(x)\,dx,
$$

an ordinary importance estimator using density $p(x)$ is

$$
\hat I=\frac1N\sum_{i=1}^N\frac{f(X_i)}{p(X_i)},
\qquad X_i\sim p.
$$

This requires $p(x)>0$ wherever $f(x)$ contributes to the target integral, up to measure-zero exceptions.

If a region has nonzero integral but $p=0$ there, taking more samples from the existing support cannot repair the missing contribution. The estimator converges to the wrong target.

## Stratification

Partition domain $D$ into strata $D_k$. A correct stratified estimator accounts for each stratum’s measure/probability. If a 3×3 pixel partition has nine equal cells but the algorithm always samples only the first five and treats their mean as the full-pixel mean, it has changed the integration domain.

Randomly selecting strata with the appropriate selection probability can remain unbiased; permanently omitting fixed strata cannot.

## MIS support

Multiple importance sampling can repair a weak proposal when another technique covers the missing region and the combined estimator uses compatible PDFs. The PDFs must include discrete technique/light selection probabilities as well as continuous densities.

## Debugging

Visualize sample positions and build a support occupancy histogram. Many “mysterious bias” bugs become obvious when entire regions are empty.

For image-space algorithms, also test edges/corners of the reconstruction filter, not only the pixel center.

Teach and reconstruct: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#A missing region cannot be repaired by taking more samples there]].

Related: [[Multiple importance sampling]], [[Latin-hypercube sampling]], [[Antialiasing and sample allocation]].
