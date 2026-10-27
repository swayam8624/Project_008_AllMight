---
tags:
  - concept
  - university-series
  - independent-series
---

# Monte Carlo rendering estimators and variance

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** University courses

## Definition and mechanism

Monte Carlo integration estimates an integral by averaging random samples with importance weights. For target integral $I=\int f(x)dx$ and samples drawn from density $p(x)$, the estimator is $\hat I=N^{-1}\sum_{k=1}^N f(x_k)/p(x_k)$. It is unbiased when samples cover every region where $f$ is nonzero and the density is correct. Its variance falls approximately as $1/N$, so halving standard error takes about four times as many samples. Importance sampling chooses $p$ to follow the magnitude of the integrand; multiple importance sampling combines strategies such as light sampling and BSDF sampling with weights that avoid overtrusting a strategy that rarely finds a contribution. Path tracing applies these estimators to light paths, while rare caustics and narrow visibility openings remain high-variance cases.

## Prerequisites

- [[Probability density and change of measure]]
- [[Reflectance and rendering equation]]

## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography|Worlds, Images and Inference]]

- [[Reservoir resampling and ReSTIR]]
## Full teaching explanation

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Numerical integration and Monte Carlo estimation|Numerical integration and Monte Carlo estimation]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#The rendering equation and light transport|The rendering equation and light transport]]
