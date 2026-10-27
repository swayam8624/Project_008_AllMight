---
tags:
  - concept
  - university-series
  - independent-series
aliases:
  - "self-attention"
  - "QKV"
  - "queries keys values"
  - "multi-head attention"
  - "vision transformer"
  - "ViT"
---

# Attention and visual transformers

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** University courses

## Definition and mechanism

Self-attention lets each token build a context-dependent mixture of other tokens. A learned linear map produces queries $Q$, keys $K$, and values $V$; scaled dot products compare each query with all keys, softmax converts scores to weights, and those weights combine values. Multiple heads learn different relationships, while positional information tells the model where patches or words came from. Vision transformers split an image into patches, embed them as tokens, and process the sequence with attention and feed-forward blocks. Unlike a small convolution, global attention can relate distant regions in one layer, but its pairwise score matrix grows roughly as the square of token count. Patch size trades spatial detail for compute; positional encodings and training data affect transfer. Attention weights are a mechanism for mixing information, not an explanation of causal importance. A transformer still needs a task objective, data, optimization, and validation, and can fail under distribution shift or poor spatial grounding.

## Prerequisites

- [[Convolution and filter footprints]]
- [[Probability density and change of measure]]
- [[Research validity and ablation]]
- [[Automatic differentiation and backpropagation]]
- [[Learned image features and CNNs]]

## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Sequence models and attention|Sequence models and attention]]
- [[Contrastive vision and image-language alignment]]
- [[Perception feedback and robot learning]]

## Full teaching explanation

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Sequence models and attention|Sequence models and attention]]

## Recall and next depth

- Compute the role of Q, K, and V in one self-attention layer.
- What makes global attention expensive as the image patch count grows?

## Source and navigation

Teaching narrative: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Sequence models and attention|open the main explanation]]. Browse the [[Keyword Index#University graphics vision and photography - independent series|University graphics and vision index]]. Additional chapter connections: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Image sampling, filtering, and efficient reconstruction|Image sampling, filtering, and efficient reconstruction]]; [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Optical flow and motion from images|Optical flow and motion from images]]; [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#A complete renderer makes the forward model executable|A complete renderer makes the forward model executable]].

Caption/source records: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#SC-08 - Stanford CS231N Spring 2025 Lecture 8: Attention and Transformers|SC-08]], [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#SC-16 - Stanford CS231N Deep Learning for Computer Vision Spring 2025 Lecture 16: Vision and Language|SC-16]].
