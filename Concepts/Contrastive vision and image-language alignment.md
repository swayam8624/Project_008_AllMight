---
tags:
  - concept
  - university-series
  - independent-series
aliases:
  - "CLIP"
  - "InfoNCE"
  - "image-text contrastive learning"
  - "image-language alignment"
  - "zero-shot image classification"
  - "contrastive vision-language model"
  - "contrastive learning"
---

# Contrastive vision and image-language alignment

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** University courses

## Definition and mechanism

Contrastive image-language learning trains an image encoder and a text encoder to place matching descriptions near their images in a shared feature space. For a batch of paired examples, cosine similarities between normalized embeddings form a score matrix; a temperature scales the scores, and an InfoNCE-style cross-entropy treats the matching pair as the positive while other batch items act as negatives. The objective makes retrieval and zero-shot label comparison possible by encoding class names as text. It learns statistical alignment, not a guarantee that every phrase is grounded in the pixels. Batch composition, duplicate or semantically similar negatives, captions, and web-data biases all shape the representation. A model may associate a prompt with correlated context rather than the requested object, and similarity scores are not calibrated probabilities. Evaluation must distinguish retrieval, classification, localization, and reasoning rather than treating one score as general visual understanding.

## Prerequisites

- [[Probability density and change of measure]]
- [[Research validity and ablation]]
- [[Perceptual metrics and display calibration]]
- [[Learned image features and CNNs]]
- [[Attention and visual transformers]]

## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Self-supervision and evaluation|Self-supervision and evaluation]]
- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Vision-language models and grounded tools|Vision-language models and grounded tools]]
- [[Perception feedback and robot learning]]

## Full teaching explanation

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Self-supervision and evaluation|Self-supervision and evaluation]]
- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Vision-language models and grounded tools|Vision-language models and grounded tools]]

## Recall and next depth

- Identify one positive pair and the negatives in a batch-level InfoNCE objective.
- Why does a high image-text similarity score not prove that the named object is grounded?

## Source and navigation

Teaching narrative: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Self-supervision and evaluation|open the main explanation]]. Browse the [[Keyword Index#University graphics vision and photography - independent series|University graphics and vision index]]. Additional chapter connections: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Recognition, templates, and industrial matching|Recognition, templates, and industrial matching]]; [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Edges, contours, and subpixel localization|Edges, contours, and subpixel localization]]; [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#A complete renderer makes the forward model executable|A complete renderer makes the forward model executable]].

Caption/source records: [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#SC-16 - Stanford CS231N Deep Learning for Computer Vision Spring 2025 Lecture 16: Vision and Language|SC-16]].
