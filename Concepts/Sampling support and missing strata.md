# Sampling support and missing strata

Support identifies where a sampling distribution can generate samples. An ordinary importance estimator must cover contributing regions with a valid combined strategy. Repeated sampling of only five fixed cells of a nine-cell pixel can converge to the wrong pixel integral.

Teach and reconstruct: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#A missing region cannot be repaired by taking more samples there]].

Related integration record: [[Supplementary/Manual Integration - Sources and Verification]].

## Mastery expansion — an estimator must sample every region it claims to integrate

Stratification partitions a domain into strata. If an estimator claims the integral over the full domain but repeatedly samples only a fixed subset of strata without compensating for the selection process, the estimator changes support and can become biased.

For stratum (S_i) with measure (w_i), a full stratified estimator combines per-stratum estimates using the corresponding measures. If strata are themselves randomly selected, the probability of selecting a stratum must enter the estimator weight.

This is the same support rule used by importance sampling: a region with nonzero integrand and zero sampling probability cannot be recovered by clever reweighting afterward.

A common failure is “take the first (N) cells” from a larger stratification. A deterministic prefix can repeatedly ignore the same region.

**Required next witness:** integrate a piecewise function using full stratification, fixed-prefix strata and randomly selected compensated strata; compare expected values over many seeds.

