---
tags:
  - concept
  - university-series
  - independent-series
aliases:
  - "CNN"
  - "convolutional neural network"
  - "convolutional feature"
  - "feature map"
  - "receptive field"
  - "weight sharing"
---

# Learned image features and CNNs

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** University courses

## Definition and mechanism

A convolutional neural network (CNN) learns filters that detect useful image patterns from data. A convolution layer applies the same small kernel at many image locations, so a feature such as an edge can be recognized wherever it appears. Early layers often respond to local contrast or texture; later layers combine these responses into task-specific patterns. Stride and pooling reduce spatial resolution, trading detail for computation and larger receptive fields. Training adjusts filter values by gradient descent using backpropagation and labeled or self-supervised objectives. Weight sharing gives translation equivariance: shifting an input shifts the feature map, but does not guarantee full translation invariance or robustness to scale, viewpoint, lighting, or distribution change. Padding and stride determine border behavior and output size; too much downsampling can erase small objects. A learned feature is a representation useful for a training objective, not a human-interpretable semantic guarantee. Performance depends on data coverage, augmentation, architecture, optimization, and honest held-out evaluation.

## Prerequisites

- [[Convolution and filter footprints]]
- [[Sampling aliasing and reconstruction]]
- [[Research validity and ablation]]
- [[Retinal encoding and receptive fields]]
- [[Automatic differentiation and backpropagation]]

## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Convolutional models preserve image neighborhoods|Convolutional models preserve image neighborhoods]]
- [[Attention and visual transformers]]
- [[Contrastive vision and image-language alignment]]
- [[Perception feedback and robot learning]]

## Full teaching explanation

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Convolutional models preserve image neighborhoods|Convolutional models preserve image neighborhoods]]

## Recall and next depth

- Follow how one shared kernel creates a feature map and how stride changes its resolution.
- What property does convolution provide under translation, and what transformations does it not make invariant?

## Source and navigation

Teaching narrative: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Convolutional models preserve image neighborhoods|open the main explanation]]. Browse the [[Keyword Index#University graphics vision and photography - independent series|University graphics and vision index]]. Additional chapter connections: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Sampling continuous signals and image formation|Sampling continuous signals and image formation]]; [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Image sampling, filtering, and efficient reconstruction|Image sampling, filtering, and efficient reconstruction]]; [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Edges, contours, and subpixel localization|Edges, contours, and subpixel localization]].

Caption/source records: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#SC-05 - Stanford CS231N Spring 2025 Lecture 5: Image Classification with CNNs|SC-05]], [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#SC-06 - Stanford CS231N Deep Learning for Computer Vision Spring 2025 Lecture 6: CNN Architectures|SC-06]], [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#SC-07 - Stanford CS231N Spring 2025 Lecture 7: Recurrent Neural Networks|SC-07]].
