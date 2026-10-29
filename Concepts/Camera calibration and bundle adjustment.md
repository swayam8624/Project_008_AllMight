---
tags:
  - concept
  - university-series
  - independent-series
---

# Camera calibration and bundle adjustment

**Curriculum depth:** Developed · **First seen:** University courses · **Latest development:** University courses

## Definition and mechanism

Camera calibration estimates parameters that map 3D points to image coordinates. Intrinsics describe focal scale, principal point, and lens distortion; extrinsics describe camera pose relative to the scene. Known calibration targets provide 3D-to-2D correspondences, and a nonlinear least-squares objective minimizes reprojection error. Bundle adjustment jointly refines many camera poses and 3D feature points by minimizing all observed image residuals. The problem is sparse because each image sees only a subset of points. It also has gauge freedoms: translating or scaling the entire reconstruction may leave projections unchanged, so fix a reference pose or scale. Robust loss functions reduce the effect of mismatched features. Good initialization, parameter scaling, and Jacobian structure determine whether optimization converges.

## Prerequisites

- [[Homogeneous coordinates and perspective division]]
- [[Singular value decomposition]]

## Used by

- [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography|Worlds, Images and Inference]]
- [[Stereo correspondence and epipolar geometry]]

- [[Radiance fields and Gaussian splatting]]
- [[Off-axis stereo projection and disparity]]
- [[Omnidirectional camera projections]]
- [[Position reconstruction from depth]]
- [[Camera gauge freedoms and observability]]

## Full teaching explanation

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Cameras, projection, and calibration|Cameras, projection, and calibration]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Pose estimation, transforms, and robust fitting|Pose estimation, transforms, and robust fitting]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Stereo, relative orientation, and reconstruction|Stereo, relative orientation, and reconstruction]]

## Phase 11 camera development

Linearize reprojection residuals as r(θ+δ)≈r(θ)+Jδ; the weighted information matrix $J^{\mathsf T}WJ$ reveals parameter combinations that change observations. Null directions are local ambiguities, while small eigenvalues indicate weak constraints. In calibrated monocular structure from motion, global frame choice and common scene scale are gauge freedoms. Fix a pose and scale convention, or add a metric measurement, before interpreting covariance. Repeated nearly identical views may leave focal length, depth, and pose coupled even with many pixels. Link: [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#A camera model must close the loop|A camera model must close the loop]]; research: [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Four deeper links from the TUM lecture review|Four deeper links from the TUM lecture review]].
