---
tags:
  - concept
  - university-series
  - independent-series
aliases:
  - "VAE"
  - "variational autoencoder"
  - "GAN"
  - "generative adversarial network"
  - "normalizing flow"
  - "generative flow"
  - "latent variable model"
  - "rectified flow"
---

# Latent-variable models and generative flows

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** University courses

## Definition and mechanism

Generative models describe how data could be produced, often by first sampling a latent variable $z$ and mapping it to an observation $x$. A variational autoencoder (VAE) learns an encoder distribution $q(z|x)$ and decoder $p(x|z)$; its evidence lower bound balances reconstruction likelihood against a KL penalty that keeps the latent distribution tractable. A generative adversarial network (GAN) trains a generator against a discriminator, creating a game that can produce sharp samples but may be unstable or collapse to limited modes. Normalizing flows use invertible transformations so the data density can be computed with a change-of-variables Jacobian determinant. These methods solve different tradeoffs among likelihood, sample quality, latent structure, and optimization stability. Their assumptions matter: a VAE's posterior family may be too simple, a GAN score is not a calibrated density, and a flow's invertibility constrains architecture. Generated samples are not evidence that the model has learned the true causal process.

## Prerequisites

- [[Probability density and change of measure]]
- [[Constrained optimization and inverse kinematics]]
- [[Research validity and ablation]]
- [[Automatic differentiation and backpropagation]]

## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Generative models learn distributions and samplers|Generative models learn distributions and samplers]]
- [[Radiance fields and Gaussian splatting]]

## Full teaching explanation

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Generative models learn distributions and samplers|Generative models learn distributions and samplers]]

## Recall and next depth

- State the VAE reconstruction/KL tradeoff, GAN adversarial objective, and flow change-of-variables role.
- Which assumption or training failure distinguishes each family?

## Source and navigation

Teaching narrative: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Generative models learn distributions and samplers|open the main explanation]]. Browse the [[Keyword Index#University graphics vision and photography - independent series|University graphics and vision index]]. Additional chapter connections: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Numerical integration and Monte Carlo estimation|Numerical integration and Monte Carlo estimation]]; [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Optimization, constraints, and inverse kinematics|Optimization, constraints, and inverse kinematics]]; [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#A linear equation is a measurement, and several equations are geometry|A linear equation is a measurement, and several equations are geometry]].

Caption/source records: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#SC-13 - Stanford CS231N Deep Learning for Computer Vision Spring 2025 Lecture 13: Generative Models 1|SC-13]], [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#SC-14 - Stanford CS231N Deep Learning for Computer Vision Spring 2025 Lecture 14: Generative Models 2|SC-14]].
