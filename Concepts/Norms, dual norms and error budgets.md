# Norms, dual norms and error budgets

A norm defines a size rule. Its dual bounds scalar measurements over its unit ball. Box and Euclidean error budgets yield different worst-case sensitivities.

[[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Bound a measurement using a dual norm|Full definitions, derivation and worked trace]].

## Mastery expansion — a norm defines what “large error” means

A norm is not decoration on an error value; it defines the geometry in which error is judged. For a vector (x), common choices include (\|x\|_1=\sum_i|x_i|), (\|x\|_2=\sqrt{\sum_i x_i^2}), and (\|x\|_\infty=\max_i|x_i|). They answer different engineering questions.

The dual norm is defined by (\|y\|_* = \sup_{\|x\|\le 1} y^Tx). It appears naturally when bounding linear measurement error: (|y^T\delta x|\le\|y\|_*\|\delta x\|). Thus a “small” state error can still create a large observable error when the measurement is sensitive in the chosen dual geometry.

An error budget should state the representation, norm, absolute/relative scale, expected range and accumulation model. Never combine heterogeneous quantities such as meters, radians and pixel intensity into one unexplained scalar.

**Required next witness:** propagate bounded input perturbations through a transform/projection chain and compare theoretical norm bounds with measured errors.

