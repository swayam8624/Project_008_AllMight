# Bandit exploration and regret

A bandit algorithm repeatedly chooses an action and observes the reward of the chosen action. Exploration trades current reward for information; regret compares cumulative reward against a specified benchmark. Stationarity, reward bounds and feedback assumptions matter. Selecting render work adaptively does not by itself preserve an unbiased pixel estimator.

Read the connected explanation in [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#From Random Samples to Deliberate Computation]].


Expanded direct teaching: [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Choose the next measurement using the geometry of what is still unknown]].

## Mastery expansion — exploration is justified by an objective and assumptions

A stochastic multi-armed bandit repeatedly selects an action and observes a reward. Cumulative regret compares obtained reward with the reward from repeatedly selecting the best arm under the assumed model.

UCB methods add an uncertainty bonus to an empirical estimate, encouraging insufficiently sampled arms. Thompson sampling samples from a posterior model. Epsilon-greedy explores more crudely.

Assumptions matter: stationarity, independence/noise model and the observation process. Under drift, confidence built from all historical samples may become misleading; sliding windows, discounting or change detection may be more appropriate.

Adaptive stopping also complicates naïve confidence statements because the number of observations is chosen using the data itself.

**Required next witness:** simulate stationary and drifting rewards with fixed seeds, compare regret/exploration traces and show a stationary-confidence failure under drift.

