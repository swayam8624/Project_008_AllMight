# Book 03 — Pass 02 primary manuscript inspection and integration map

**Purpose:** prevent the new reader's friendly voice from replacing existing deep derivations. These notes record **actual inspected source chapter openings**, their repository blob identities and the new reader sections to which they are relevant. They do not falsely label the full long chapters “reviewed.” All files below already live in the restructured Notes/ tree; no source file was moved or rewritten.

## A precise question first

The original computing master and graphics master contain a much richer camera, geometry and image-formation narrative than a new introductory chapter can reasonably reproduce in one pass. Should Chapter 02 become a short rewrite of the old chapters? **No.** Its job is a clean entrance, then verified inclusion and rigorous revision of those pre-existing source spans in their correct reader locations. Old source remains authoritative until source-unit reconciliation establishes a safe integration.

## Examined originals and verified first-use facts

| Old source path relative to Notes/ | Observed Git blob SHA | Original master span from chapter header | Fact inspected and destination |
|---|---|---|---|
| Chapter Edition/01 - Computing Foundations/32 - Geometry Becomes an Image and an Inverse Problem.md | 2bb93ac0b57df60f2b7e7655ad196373215a30bf | Computing Foundations lines 78505–80940; SHA256 340c3f3eab22b702f8d250fbf0fdb8bd0a1e0b2e848681aa23ee77df8dacef80 | Right-handed camera looks down local -Z, column-vector PVM, separate matrix storage, near-plane/depth and pixel origins. Chapter 02 introduces only planar affine frames; the original 3D proof will route to Chapters 11–16. |
| Chapter Edition/01 - Computing Foundations/33 - Projection, Clipping, and Raster Sampling.md | ef481955be60941713582f1e5600191498f4bd8d | Computing Foundations lines 80941–84338; SHA256 fcf61fdb48daa14bb38e7cbb8d87cc82df71979648d4c3d5d7fa16fca6ec1b8b | Orthographic-versus-perspective forward geometry, homogeneous clipping, NDC and sample-center conventions. Book 03 Chapters 13–21 must preserve full derivation rather than repeat an early two-dimensional map. |
| Chapter Edition/01 - Computing Foundations/34 - Robust Predicates and Camera Sensitivity.md | c75a9727e22b771187f941ddfebe0042bb8ee9ba | Computing Foundations lines 84339–87111; SHA256 f824915709a11c867778d38638510a33752e0269c1d2d5233cf5caca04f05d7c | Orientation determinant, discrete sign decisions, near-degenerate triangle, camera sensitivities; authoritative development for Chapters 03, 05 and 26. |
| Chapter Edition/01 - Computing Foundations/35 - Camera Extrinsics and Projection Geometry.md | ae269463e966c7c8e19dfa18954d2a4552438833 | Computing Foundations lines 87112–90166; SHA256 d05c86caf710d5dae5ba53d28b3e1e882519751530a0c533621a24830008f64c | World-to-camera extrinsics use X_C=R_CW X_W + t_CW with t_CW = -R_CW C_W; Chapter 02 uses named 2D frames but defers this camera-center issue to Chapters 11–15. |
| Chapter Edition/01 - Computing Foundations/36 - Exposure, Physical Optics, and Depth of Field.md | 1391fdcc5adbf5c320ff5bde278a76d912189564 | Computing Foundations lines 90167–93036; SHA256 85da0b62a78b033757909481b5fd79089a16a18ac238ea0a3caf1745b6ce438f | Exposure changes sensor/display interpretation rather than changing source radiance. Route optical assumptions to Book 03 Chapters 22–25, deeper radiative physics to Book 04. |
| Chapter Edition/01 - Computing Foundations/37 - Camera Contracts and Transform-Chain Debugging.md | 957f1b0ddd3c620cf28e5289b606687c8f32d71c | Computing Foundations lines 93037–95790; SHA256 ff9fb23aab9ebc413a2ce9ee1acf22cb01bdfc4a2c0af26d14b3f670299495fb | Camera pose/projection/depth/raster/optics/temporal/calibration contracts are different entities. Later camera and full-pipeline tests must preserve each contract separately. |
| Chapter Edition/01 - Computing Foundations/38 - Reconstruct the Complete Geometry Pipeline.md | 94e1114642639aef5914bc4c53952bb61d78f39b | Computing Foundations lines 95791–97432; SHA256 6248bbc6f44946b2e0faa9869bf7ac6e9ebc30a7945ed4cc75ec8c9ecd4db96e | Original integrated example already walks imported geometry through frame, normals, interpolation, collision, projection and buffer encoding. Route nonduplicated components and source evidence into Chapters 21 and 31–32. |
| Chapter Edition/02 - Graphics Vision and XR/13 - From View Space to Clip Space.md | c99505f5c26da93b53ffb68c0ac8c38261eddd2a | Graphics master lines 28351–31800; SHA256 cfb093b9ae71f87be0582e488bddba421a963189da090b4c068283090e5435cd | It distinguishes clip space from NDC and follows a Kairo vertex path. Chapter 16 must rederive this independently and check exact renderer source before making any current-repo claim. |
| Chapter Edition/02 - Graphics Vision and XR/23 - Mesh Topology and Connectivity.md | 991dfa264100be10e1df3efc801966ae1d383da1 | Graphics master lines 57090–59762; SHA256 527e67cd9bc0edb8249c4bdc2df4b7befbdafe310af89d9dd833bbcf50cac8c8 | Mesh indices, shared edges, disconnected duplicates and winding are not the same as mesh coordinates. Route full treatment to Chapter 07 and mesh-debugging project, not Chapter 02. |

## Explicit convention bridge

The newly drafted Chapter 02 uses **world X right / world Y up**, 2D affine transforms, local-coordinate metres, and a teaching world-to-image map with image u right / v down and positive pixels/metre. It does not contradict the old right-handed **3D** camera convention looking along camera -Z: that is a *different frame and dimensionality* that must be introduced by Chapter 11 before projection. In Chapter 14, explicitly map the 3D view frame through projection to clip/NDC and then convert to the agreed image origin. Never equate a two-dimensional orthographic world-to-image drawing map with camera extrinsic/intrinsic calibration.

The old 3D chapter's camera center warning must survive: a camera's world-space center C_W is not usually identical to the translation column of its world-to-camera extrinsic transform. The new 2D frame header already calls its object origin an explicit world point; do not hide it when upgrading to 3D.

## Source custody boundary

The blob SHAs above identify the chapter-reading files inspected *at their openings* on 23 November 2026. They do not certify full body inspection, tested snippets, licensing, citations or exact per-paragraph provenance of a final reader. Original master span SHA256 values are copied from preserved source-derivation headers and must be rechecked against the actual long masters before source integration.

No old chapter or long continuous note should be deleted, replaced with a condensed rewrite or mechanically pasted wholesale into a new Book 03 chapter. First build a per-source-unit map, then explicitly distinguish new instruction, relocated legacy explanation, and duplicate material whose only duplication is editorial. Source-preserving composition needs actual hashes and tests.

## Next audit and technical work

1. Inspect the *full* source bodies for Chapters 32–38 and relevant Graphics/Vision sections, recording section-level destination and unique code/figures.
2. Verify equation conventions and the C++23 frame model with actual compiler/CTest results; do not infer success from source presence.
3. Continue Chapter 03 with robust triangle/oriented-area geometry; connect Shewchuk-type exact predicate motivation to the frozen Book 02 orientation material without copying its whole implementation.
4. Build reader composition only after the source map can demonstrate full preservation and coherent ordering.
