---
tags:
  - concept
aliases:
  - "quantile"
  - "median absolute deviation"
  - "MAD"
  - "RMSE"
  - "MAE"
  - "MAPE"
---

# Robust summaries and error metrics

**Curriculum depth:** Developed · **First seen:** Phase 10 · **Latest development:** Phase 10

## Definition and mechanism

Histograms, quantiles, medians, and robust scales summarize different features of a finite dataset. Histogram bar area represents probability only when heights are divided by sample count and bin width. Quantile conventions differ across software, so report the interpolation rule. The median minimizes absolute deviation and resists some extreme observations; the mean minimizes squared deviation and is more sensitive to outliers. MAD and trimmed means provide other robustness choices with efficiency tradeoffs. MSE and RMSE emphasize large errors, MAE is less sensitive to them, and relative metrics can become unstable near zero. Aggregating per-scene scores differs from pooling every observation. Choose the metric from the scientific question, units, and failure cost, and report tail behavior when rare large errors matter.

## Prerequisites

- [[Expectation, covariance, and dependence]]
- [[Absolute and relative error]]

## Used by

- This node is a current endpoint in the supplied Phase 9–10 path.

- [[Reproducible numerical benchmarking]]
## Full teaching explanation

- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#10. Inference and error metrics need explicit assumptions|Phase 10 teaching section]]
- [[Visual Recall/10 - Probability and Monte Carlo - Visual Recall.md]] · visual recall

## Source coverage

Supplied lesson parts: 956 (T06 · M058), 957 (T06 · M058), 958 (T06 · M058), 959 (T06 · M058), 962 (T06 · M058), 963 (T06 · M058), 964 (T06 · M058), 965 (T06 · M058).
