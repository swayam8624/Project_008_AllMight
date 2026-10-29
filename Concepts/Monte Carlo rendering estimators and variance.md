---
tags:
  - concept
  - university-series
  - independent-series
aliases:
  - "Monte Carlo integration"
  - "MIS estimator"
---

# Monte Carlo rendering estimators and variance

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** Phase 10

## Definition and mechanism

Monte Carlo integration estimates an integral by averaging random samples with importance weights. For target integral $I=\int f(x)dx$ and samples drawn from density $p(x)$, the estimator is $\hat I=N^{-1}\sum_{k=1}^N f(x_k)/p(x_k)$. It is unbiased when samples cover every region where $f$ is nonzero and the density is correct. Its variance falls approximately as $1/N$, so halving standard error takes about four times as many samples. Importance sampling chooses $p$ to follow the magnitude of the integrand; multiple importance sampling combines strategies such as light sampling and BSDF sampling with weights that avoid overtrusting a strategy that rarely finds a contribution. Path tracing applies these estimators to light paths, while rare caustics and narrow visibility openings remain high-variance cases.

### Phase 9–10 development

For independent samples from density $p$, the sample average estimates the integral using terms $f(X_i)/p(X_i)$ where $p$ is positive on the integrand support. Unbiasedness requires the support condition and integrability; finite variance additionally requires a finite second moment of $f/p$. A variance estimate describes sampling spread, not systematic bias. Dependent samples add covariance terms, and multiple-importance weights combine proposal densities while preserving mixture support. A confidence interval is justified only when its sampling and moment assumptions are stated.

## Prerequisites

- [[Probability density and change of measure]]
- [[Reflectance and rendering equation]]
- [[Inverse, rejection, and importance sampling]]
- [[Expectation, covariance, and dependence]]

- [[Stable probability normalization and log-sum-exp]]
## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography|Worlds, Images and Inference]]

- [[Reservoir resampling and ReSTIR]]
- [[Confidence intervals and sequential evidence]]
- [[Antialiasing and sample allocation]]

## Full teaching explanation

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Numerical integration and Monte Carlo estimation|Numerical integration and Monte Carlo estimation]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#The rendering equation and light transport|The rendering equation and light transport]]
- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#8. Monte Carlo estimates trade computation for uncertainty|Phase 10 teaching section]]
- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#7. Transformations turn simple inputs into useful samples|Phase 10 teaching section]]
- [[Visual Recall/10 - Probability and Monte Carlo - Visual Recall.md]] · Phase 10 visual recall
