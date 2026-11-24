# Pass 13 — Original-source custody: pinhole camera and calibrated projection

**Date:** 24 November 2026. **Branch:** curriculum/book03-geometry-images-pass01.

This is an additive reader/laboratory pass. Original continuous notes, derived reading copies, legacy typesetting stage, published Books 01–02 and final distribution directories remain read-only.

## Directly inspected source identities

- `Notes/Chapter Edition/01 - Computing Foundations/32 - Geometry Becomes an Image and an Inverse Problem.md`, Git blob `2bb93ac0b57df60f2b7e7655ad196373215a30bf`. Header-declared original master span 78505–80940, SHA256 `340c3f3eab22b702f8d250fbf0fdb8bd0a1e0b2e848681aa23ee77df8dacef80`. Located passages at reading-copy lines 23 and 55 explicitly distinguish graphics local -Z forward from optical +Z forward, positive axial depth, and normalized pinhole coordinates. Located line 1700 identifies the positive optical Z convention for the projection derivation.
- `Notes/Chapter Edition/01 - Computing Foundations/37 - Camera Contracts and Transform-Chain Debugging.md`, blob `957f1b0ddd3c620cf28e5289b606687c8f32d71c`. Header-declared master span 93037–95790, SHA256 `ff9fb23aab9ebc413a2ce9ee1acf22cb01bdfc4a2c0af26d14b3f670299495fb`. Located passages at reading-copy lines 626–627 distinguish calibration and lens fault boundaries; 729–737 describe the original KairoRayTracer pinhole path; 1512–1534 identify frame/ray mapping; 1712–1750 discuss intrinsics; 1834 identifies the projection Jacobian; 2135–2167 describe calibration validity and the original Vulkax pinhole observation model.
- Existing P12 `rotation3.hpp`, blob `bd61b3765a18eb9678dcb3582295e58eada1adea`, and P06 `geometry3d.hpp`, blob `b386703e351ccb6bf8730754949f2683903628e7`, were inspected for actual RigidPose, Rotation3, Point3, Vector3 and Ray3 APIs before writing P13.

Header-declared source-span hashes were recorded, not independently recomputed against the full continuous master. These targeted located passages do not establish complete source-unit reconciliation.

## Coverage and outstanding source work

| Subject | P13 treatment | Outstanding |
|---|---|---|
| Graphics and optical frames | Explicit D=diag(1,-1,-1), camera-to-world pose, center-relative inverse, extrinsic translation | Full engine API adapters and import conventions |
| Pinhole and calibration | Similar triangles, fx/fy/skew/principal point, homogeneous K[R|t], sensor units and FOV | Lens distortion and physical lens modeling |
| Image coordinates | Continuous top-left edge coordinates, half-integer sample centers, half-open image membership, crop/resize | Filtering, raster coverage, image reconstruction |
| Inverse geometry | Unit world ray and axial-depth unprojection; depth/range distinction | Multiview triangulation, depth-buffer inversion and calibration estimation |
| Sensitivity | 2x3 optical Jacobian, radial null direction, first-order covariance explanation, finite-difference tests | Pose Jacobians, distortion Jacobians, estimation conditioning and uncertainty propagation |
| Projective camera versus GPU projection | Explicit distinction, near-plane and visibility not hidden | Four-component clip coordinates, orthographic and perspective depth mappings, reverse-Z |

No full camera renderer, optics simulator, calibration solver, GPU performance result or native test pass is claimed. Original material remains intact.

**Next:** Chapter 14 — From a 3D Point to a Screen Address.
