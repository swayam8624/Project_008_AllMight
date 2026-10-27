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
## Full teaching explanation

[[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Cameras, projection, and calibration|Cameras, projection, and calibration]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Pose estimation, transforms, and robust fitting|Pose estimation, transforms, and robust fitting]] · [[Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography#Stereo, relative orientation, and reconstruction|Stereo, relative orientation, and reconstruction]]
