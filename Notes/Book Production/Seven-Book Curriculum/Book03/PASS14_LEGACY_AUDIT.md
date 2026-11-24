# Pass 14 — Original-source custody: homogeneous projection and depth

**Date:** 24 November 2026. **Branch:** curriculum/book03-geometry-images-pass01.

This is an additive pass. Original continuous notes, derived reading copies, legacy typesetting material and published Books 01–02 remain unchanged.

## Original sources directly inspected

- `Notes/Chapter Edition/01 - Computing Foundations/33 - Projection, Clipping, and Raster Sampling.md`, blob `ef481955be60941713582f1e5600191498f4bd8d`. Header-declared master range 80941–84338 and source-span SHA256 `fcf61fdb48daa14bb38e7cbb8d87cc82df71979648d4c3d5d7fa16fca6ec1b8b`. Located reading-copy material: lines 20–92 orthographic geometry; 234–329 orthographic and reversed depth witnesses; 514–776 homogeneous division and w; 827–934 zero-to-one depth; 1180–1520 FOV/near/far and precision; 1592–1845 reciprocal depth, reversed depth and floating storage concerns.
- `Notes/Chapter Edition/01 - Computing Foundations/34 - Robust Predicates and Camera Sensitivity.md`, blob `c75a9727e22b771187f941ddfebe0042bb8ee9ba`. Header-declared master range 84339–87111, SHA256 `f824915709a11c867778d38638510a33752e0269c1d2d5233cf5caca04f05d7c`. Located lines 352–392 identify NDC frustum corners and reversed depth assignment; 656–870 address homogeneous planes and the redundant infinite far inequality; 1451–1453 warn against mixing API projection conventions; 1606 identifies screen-space depth interpolation.
- Existing `include/book03/pinhole_camera.hpp`, blob `4a980a1fb282d350747d3b9078c09c417b373a0e`: directly inspected intrinsics, pixel conventions, optical mapping and projection API before writing the calibrated homogeneous matrix.

These are targeted passages and recorded header metadata, not complete original source reconciliation or independently recomputed master hashes.

## Scope

P14 derives off-axis calibrated perspective including skew, orthographic projection, zero-to-one homogeneous clip membership, positive-w division, viewport mapping, forward/reverse finite/infinite perspective depth, inverse depth and a controlled floating-storage experiment. Original discussions of API-specific conventions, general frustum extraction, oblique projection, robust clipping, complete primitive coverage and interpolation remain owed in later passes.

The new code is a scalar reference, not a Metal/Vulkan/OpenGL renderer. No native build or CI pass has been observed. No PDF/EPUB is generated.

**Next:** Chapter 15 — Two Eyes Looking at One House.
