---
tags:
  - concept
  - university-series
  - independent-series
aliases:
  - "autodiff"
  - "automatic differentiation"
  - "backpropagation"
  - "backprop"
  - "JVP"
  - "Jacobian-vector product"
  - "VJP"
  - "vector-Jacobian product"
  - "reverse mode"
  - "dual numbers"
  - "reverse accumulation"
---

# Automatic differentiation and backpropagation

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** Phase 9

## Definition and mechanism

Automatic differentiation evaluates derivatives by applying the chain rule to the operations in a program. Each operation has a local derivative rule; composing the rules gives the derivative of the complete computation without symbolically expanding it or approximating it by finite differences. Forward mode propagates a tangent perturbation and computes a Jacobian-vector product (JVP), useful when there are few inputs. Reverse mode propagates sensitivities from outputs back to inputs and computes a vector-Jacobian product (VJP), making gradients of one scalar loss with respect to millions of parameters practical. Backpropagation is reverse-mode differentiation through a neural network's computation graph. It does not remove numerical problems: nondifferentiable operations require conventions, saturated activations can yield tiny gradients, and an incorrect graph or reduction yields an incorrect gradient. Gradients are local sensitivity information, not proof of a global optimum. Finite differences remain a useful small-scale check, but cancellation and step-size tradeoffs make them a poor training method.

### Phase 9–10 development

Forward mode propagates an input tangent through local derivatives and computes a Jacobian-vector product. Reverse mode propagates an output sensitivity backward and computes a vector-Jacobian product. These are chain-rule evaluations on the operation graph, distinct from finite differences, which perturb inputs and can suffer cancellation. The reverse pass requires a valid reverse-topological order and storage that remains alive until propagation finishes. Differentiating a converged solution uses an implicit equation and its local Jacobian; it is not automatically the same as differentiating every solver iteration.

## Prerequisites

- [[Constrained optimization and inverse kinematics]]
- [[Matrix multiplication and composition order]]
- [[Floating-point reassociation]]
- [[Derivative as a local linear map]]
- [[Jacobians, pushforwards, and pullbacks]]

## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Learning a visual classifier from examples|Learning a visual classifier from examples]]
- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Optimization, constraints, and inverse kinematics|Optimization, constraints, and inverse kinematics]]
- [[Learned image features and CNNs]]
- [[Attention and visual transformers]]
- [[Latent-variable models and generative flows]]
- [[Radiance fields and Gaussian splatting]]
- [[Perception feedback and robot learning]]
- [[Implicit differentiation and parameter sensitivities]]

## Full teaching explanation

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Learning a visual classifier from examples|Learning a visual classifier from examples]]
- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Optimization, constraints, and inverse kinematics|Optimization, constraints, and inverse kinematics]]
- [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Numerical differentiation and automatic differentiation|Phase 9 teaching section]]
- [[Visual Recall/09 - Calculus and Optimization - Visual Recall.md]] · Phase 9 visual recall

## Recall and next depth

- Compare a JVP and VJP by naming the direction propagated and the Jacobian multiplication performed.
- Why does a correct gradient still fail to guarantee a global minimum?

## Source and navigation

Teaching narrative: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Learning a visual classifier from examples|open the main explanation]]. Browse the [[Keyword Index#University graphics vision and photography - independent series|University graphics and vision index]]. Additional chapter connections: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Optimization, constraints, and inverse kinematics|Optimization, constraints, and inverse kinematics]]; [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#QR turns least squares into a triangular solve plus an irreducible residual|QR turns least squares into a triangular solve plus an irreducible residual]]; [[Continuous Notes/02 - C++ - From Objects to Reliable Programs#A compiled interface rather than repeated text|A compiled interface rather than repeated text]].

Caption/source records: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#SC-02 - Stanford CS231N Spring 2025 Lecture 2: Image Classification with Linear Classifiers|SC-02]], [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#SC-03 - Stanford CS231N Spring 2025 Lecture 3: Regularization and Optimization|SC-03]], [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#SC-04 - Stanford CS231N Spring 2025 Lecture 4: Neural Networks and Backpropagation|SC-04]].
