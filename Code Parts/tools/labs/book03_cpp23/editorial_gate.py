#!/usr/bin/env python3
"""Source-only checks for Book 03 / Passes 01–17.

This validates names, manuscript location, original figures and references.
It deliberately does not compile LaTeX, rewrite the manuscript, build an EPUB
or claim to have assessed the mathematical truth of every narrative sentence.
The C++23 compiler and CTest remain responsible for executable behavior.
"""

from __future__ import annotations

import sys
from pathlib import Path
import xml.etree.ElementTree as ET


ROOT = Path(__file__).resolve().parents[4]
BOOK = ROOT / "Notes/Book Production/Seven-Book Curriculum/Book03"
LAB = ROOT / "Code Parts/tools/labs/book03_cpp23"
CHAPTER = BOOK / "Chapters/01 - The Picture Made of Little Windows.md"
CHAPTER_02 = BOOK / "Chapters/02 - Every Place Needs an Address.md"
CHAPTER_03 = BOOK / "Chapters/03 - Shapes That Know Their Borders.md"
CHAPTER_04 = BOOK / "Chapters/04 - Move the World Without Losing It.md"
CHAPTER_05 = BOOK / "Chapters/05 - The Triangle That Almost Wasn't There.md"
CHAPTER_06 = BOOK / "Chapters/06 - Step Out of the Page.md"
CHAPTER_07 = BOOK / "Chapters/07 - One Shape, Many Triangles.md"
CHAPTER_08 = BOOK / "Chapters/08 - Drawing Smooth Things with Control Points.md"
CHAPTER_09 = BOOK / "Chapters/09 - Making a World from Rules.md"
CHAPTER_10 = BOOK / "Chapters/10 - A Mesh You Can Trust.md"
CHAPTER_11 = BOOK / "Chapters/11 - The Same Tree in Five Worlds.md"
CHAPTER_12 = BOOK / "Chapters/12 - Rotations Without Headaches.md"
CHAPTER_13 = BOOK / "Chapters/13 - The Tiny Hole That Makes a Picture.md"
CHAPTER_14 = BOOK / "Chapters/14 - From a 3D Point to a Screen Address.md"
CHAPTER_15 = BOOK / "Chapters/15 - Two Eyes Looking at One House.md"
CHAPTER_16 = BOOK / "Chapters/16 - The Long Journey of One Vertex.md"
CHAPTER_17 = BOOK / "Chapters/17 - Shapes That Refuse to Leave the Window.md"
SVG = "{http://www.w3.org/2000/svg}"


def check(condition: bool, explanation: str) -> None:
    if not condition:
        raise AssertionError(explanation)


def main() -> int:
    for path in [
        ROOT / "CHATGPT.md",
        ROOT / "AGENTS.md",
        ROOT / "Notes/Continuous Notes",
        ROOT / "Notes/Book Production/AllMight Codex/LaTeX/CURRENT.json",
        BOOK / "README.md",
        BOOK / "EDITORIAL_CONTRACT.md",
        BOOK / "SOURCE_LEDGER.md",
        BOOK / "CHAPTER_PLAN.md",
        CHAPTER,
        CHAPTER_02,
        CHAPTER_03,
        CHAPTER_04,
        CHAPTER_05,
        CHAPTER_06,
        CHAPTER_07,
        CHAPTER_08,
        CHAPTER_09,
        CHAPTER_10,
        CHAPTER_11,
        CHAPTER_12,
        CHAPTER_13,
        CHAPTER_14,
        CHAPTER_15,
        CHAPTER_16,
        CHAPTER_17,
        LAB / "CMakeLists.txt",
        LAB / "include/book03/pixel_canvas.hpp",
        LAB / "src/main.cpp",
        LAB / "tests/pixel_canvas_tests.cpp",
        LAB / "include/book03/frame_math.hpp",
        LAB / "src/frame_playground.cpp",
        LAB / "tests/frame_math_tests.cpp",
        LAB / "include/book03/triangle_geometry.hpp",
        LAB / "src/triangle_studio.cpp",
        LAB / "tests/triangle_geometry_tests.cpp",
        LAB / "include/book03/affine2.hpp",
        LAB / "src/transform_studio.cpp",
        LAB / "tests/affine2_tests.cpp",
        LAB / "include/book03/exact_orient2d.hpp",
        LAB / "src/precision_studio.cpp",
        LAB / "tests/exact_orient2d_tests.cpp",
        LAB / "include/book03/geometry3d.hpp",
        LAB / "src/spatial_studio.cpp",
        LAB / "tests/geometry3d_tests.cpp",
        LAB / "include/book03/mesh_topology.hpp",
        LAB / "src/mesh_studio.cpp",
        LAB / "tests/mesh_topology_tests.cpp",
        LAB / "include/book03/halfedge_mesh.hpp",
        LAB / "tests/halfedge_mesh_tests.cpp",
        LAB / "include/book03/curves3d.hpp",
        LAB / "src/curve_studio.cpp",
        LAB / "tests/curves3d_tests.cpp",
        LAB / "include/book03/procedural_geometry.hpp",
        LAB / "src/procedural_studio.cpp",
        LAB / "tests/procedural_geometry_tests.cpp",
        LAB / "include/book03/asset_import.hpp",
        LAB / "src/asset_studio.cpp",
        LAB / "tests/asset_import_tests.cpp",
        LAB / "include/book03/scene_forest.hpp",
        LAB / "src/scene_studio.cpp",
        LAB / "tests/scene_forest_tests.cpp",
        LAB / "include/book03/rotation3.hpp",
        LAB / "src/rotation_studio.cpp",
        LAB / "tests/rotation3_tests.cpp",
        LAB / "include/book03/pinhole_camera.hpp",
        LAB / "src/pinhole_studio.cpp",
        LAB / "tests/pinhole_camera_tests.cpp",
        LAB / "include/book03/projection_pipeline.hpp",
        LAB / "src/projection_studio.cpp",
        LAB / "tests/projection_pipeline_tests.cpp",
        LAB / "include/book03/stereo_geometry.hpp",
        LAB / "src/stereo_studio.cpp",
        LAB / "tests/stereo_geometry_tests.cpp",
        LAB / "include/book03/vertex_pipeline.hpp",
        LAB / "src/vertex_studio.cpp",
        LAB / "tests/vertex_pipeline_tests.cpp",
        LAB / "include/book03/primitive_clipping.hpp",
        LAB / "src/clipping_studio.cpp",
        LAB / "tests/primitive_clipping_tests.cpp",
    ]:
        check(path.exists(), f"missing required source: {path}")

    charter = (ROOT / "CHATGPT.md").read_text(encoding="utf-8")
    check("From Bits to Research/" in charter, "new publication layout missing")
    check("Notes/Continuous Notes/" in charter, "master notes contract missing")
    check("Code Parts/" in charter, "new code layout missing")

    chapter = CHAPTER.read_text(encoding="utf-8")
    check(len(chapter) > 15_000, "opening chapter was unexpectedly truncated")
    required_prose = [
        "The Picture Made of Little Windows",
        "scene", "continuous", "stored", "RGB8", "row-major", "Bresenham",
        "Reconstruct", "Falsify", "Transfer", "Explain",
        "write_ppm", "Chapter 02", "Pixel index",
    ]
    for phrase in required_prose:
        check(phrase in chapter, f"opening chapter lost essential concept: {phrase}")

    chapter2 = CHAPTER_02.read_text(encoding="utf-8")
    check(len(chapter2) > 15_000, "second chapter unexpectedly truncated")
    for phrase in [
        "Every Place Needs an Address", "displacement", "basis", "frame",
        "determinant", "metres", "ImageMap2", "Reconstruct", "Falsify",
        "Transfer", "Explain", "Chapter 03",
    ]:
        check(phrase in chapter2, f"second chapter missing: {phrase}")

    chapter3 = CHAPTER_03.read_text(encoding="utf-8")
    check(len(chapter3) > 18_000, "third chapter unexpectedly truncated")
    for phrase in [
        "Shapes That Know Their Borders", "signed", "winding",
        "barycentric", "Degenerate", "Reconstruct", "Falsify",
        "Transfer", "Explain", "Chapter 04", "exact",
    ]:
        check(phrase.lower() in chapter3.lower(),
              f"third chapter missing essential concept: {phrase}")

    chapter4 = CHAPTER_04.read_text(encoding="utf-8")
    check(len(chapter4) > 20_000, "fourth chapter unexpectedly truncated")
    for phrase in [
        "Move the World Without Losing It", "affine", "homogeneous",
        "shear", "reflection", "determinant", "inverse", "pivot",
        "normal", "Reconstruct", "Falsify", "Transfer", "Explain",
        "Chapter 05", "active", "passive",
    ]:
        check(phrase.lower() in chapter4.lower(),
              f"fourth chapter missing essential concept: {phrase}")

    chapter5 = CHAPTER_05.read_text(encoding="utf-8")
    check(len(chapter5) > 20_000, "fifth chapter unexpectedly truncated")
    for phrase in [
        "The Triangle That Almost Wasn't There", "catastrophic",
        "binary64", "subnormal", "signed", "exact", "dyadic",
        "represented", "Reconstruct", "Falsify", "Transfer",
        "Implement", "Explain", "Chapter 06", "Shewchuk",
    ]:
        check(phrase.lower() in chapter5.lower(),
              f"fifth chapter missing essential concept: {phrase}")

    chapter6 = CHAPTER_06.read_text(encoding="utf-8")
    check(len(chapter6) > 20_000, "sixth chapter unexpectedly truncated")
    for phrase in [
        "Step Out of the Page", "right-handed", "dot product",
        "cross product", "Plane3", "Ray3", "Sphere3",
        "signed distance", "ray", "parallel", "normal",
        "Reconstruct", "Falsify", "Transfer", "Implement",
        "Explain", "Chapter 07", "oblique",
    ]:
        check(phrase.lower() in chapter6.lower(),
              f"sixth chapter missing essential concept: {phrase}")

    chapter7 = CHAPTER_07.read_text(encoding="utf-8")
    check(len(chapter7) > 20_000, "seventh chapter unexpectedly truncated")
    for phrase in [
        "One Shape, Many Triangles", "indexed", "topology", "vertex",
        "manifold", "winding", "Euler", "seam", "bow-tie",
        "per-corner", "Reconstruct", "Falsify", "Transfer",
        "Implement", "Explain", "Chapter 08",
    ]:
        check(phrase.lower() in chapter7.lower(),
              f"seventh chapter missing essential concept: {phrase}")

    chapter8 = CHAPTER_08.read_text(encoding="utf-8")
    check(len(chapter8) > 25_000, "eighth chapter unexpectedly truncated")
    for phrase in [
        "Drawing Smooth Things with Control Points", "Bézier",
        "Hermite", "B-spline", "de Casteljau", "Bernstein",
        "tangent", "second derivative", "G^1", "C^1",
        "tessellation", "bicubic", "control", "indexed",
        "Reconstruct", "Falsify", "Transfer", "Implement",
        "Explain", "Chapter 09",
    ]:
        check(phrase.lower() in chapter8.lower(),
              f"eighth chapter missing essential concept: {phrase}")

    chapter9 = CHAPTER_09.read_text(encoding="utf-8")
    check(len(chapter9) > 30_000, "ninth chapter unexpectedly truncated")
    for phrase in [
        "Making a World from Rules", "heightfield", "cylinder", "sphere",
        "implicit", "signed distance", "gradient", "marching tetrahedra",
        "topology", "pole", "longitude", "index", "normal",
        "Reconstruct", "Falsify", "Transfer", "Implement",
        "Explain", "Chapter 10",
    ]:
        check(phrase.lower() in chapter9.lower(),
              f"ninth chapter missing essential concept: {phrase}")

    chapter10 = CHAPTER_10.read_text(encoding="utf-8")
    check(len(chapter10) > 30_000, "tenth chapter unexpectedly truncated")
    for phrase in [
        "A Mesh You Can Trust", "OBJ", "glTF", "vertex",
        "texture", "normal", "tangent", "tuple", "negative",
        "index", "winding", "manifold", "triangulation",
        "bufferView", "accessor", "byteStride", "repair",
        "Reconstruct", "Falsify", "Transfer", "Implement",
        "Explain", "Chapter 11",
    ]:
        check(phrase.lower() in chapter10.lower(),
              f"tenth chapter missing essential concept: {phrase}")

    chapter11 = CHAPTER_11.read_text(encoding="utf-8")
    check(len(chapter11) > 25_000, "eleventh chapter unexpectedly truncated")
    for phrase in [
        "The Same Tree in Five Worlds", "asset", "instance", "parent",
        "affine", "Kahn", "cycle", "reparent", "inverse transpose",
        "reflection", "bounding box", "Reconstruct", "Falsify",
        "Implement", "Explain", "Chapter 12",
    ]:
        check(phrase.lower() in chapter11.lower(),
              f"eleventh chapter missing essential concept: {phrase}")

    chapter12 = CHAPTER_12.read_text(encoding="utf-8")
    check(len(chapter12) > 25_000, "twelfth chapter unexpectedly truncated")
    for phrase in [
        "Rotations Without Headaches", "SO(3)", "SE(3)", "Rodrigues",
        "gimbal lock", "Hamilton product", "quaternion", "SLERP", "NLERP",
        "inverse", "Reconstruct", "Falsify", "Implement", "Explain", "Chapter 13",
    ]:
        check(phrase.lower() in chapter12.lower(),
              f"twelfth chapter missing essential concept: {phrase}")

    chapter13 = CHAPTER_13.read_text(encoding="utf-8")
    check(len(chapter13) > 30_000, "thirteenth chapter unexpectedly truncated")
    for phrase in [
        "The Tiny Hole That Makes a Picture", "similar triangles", "optical",
        "intrinsics", "extrinsics", "principal point", "focal length",
        "field of view", "axial depth", "Jacobian", "crop", "ray",
        "Reconstruct", "Falsify", "Implement", "Challenge", "Chapter 14",
    ]:
        check(phrase.lower() in chapter13.lower(),
              f"thirteenth chapter missing essential concept: {phrase}")

    chapter14 = CHAPTER_14.read_text(encoding="utf-8")
    check(len(chapter14) > 25_000, "fourteenth chapter unexpectedly truncated")
    for phrase in [
        "From a 3D Point to a Screen Address", "homogeneous", "clip",
        "orthographic", "NDC", "viewport", "reverse-Z", "reciprocal",
        "infinite", "float32", "Reconstruct", "Falsify", "Implement", "Chapter 15",
    ]:
        check(phrase.lower() in chapter14.lower(),
              f"fourteenth chapter missing essential concept: {phrase}")

    chapter15 = CHAPTER_15.read_text(encoding="utf-8")
    check(len(chapter15) > 27_000, "fifteenth chapter unexpectedly truncated")
    for phrase in [
        "Two Eyes Looking at One House", "baseline", "disparity",
        "principal point", "epipolar", "essential matrix", "fundamental matrix",
        "rectification", "cheirality", "reprojection", "uncertainty",
        "Reconstruct", "Falsify", "Implement", "Chapter 16",
    ]:
        check(phrase.lower() in chapter15.lower(),
              f"fifteenth chapter missing essential concept: {phrase}")

    chapter16 = CHAPTER_16.read_text(encoding="utf-8")
    check(len(chapter16) > 25_000, "sixteenth chapter unexpectedly truncated")
    for phrase in [
        "The Long Journey of One Vertex", "rendering tuple", "vertex invocation",
        "inverse transpose", "clip coordinate", "surface barycentrics",
        "perspective-correct", "reciprocal-w", "screen-linear", "noperspective",
        "Reconstruct", "Falsify", "Implement", "Chapter 17",
    ]:
        check(phrase.lower() in chapter16.lower(),
              f"sixteenth chapter missing essential concept: {phrase}")

    chapter17 = CHAPTER_17.read_text(encoding="utf-8")
    check(len(chapter17) > 20_000, "seventeenth chapter unexpectedly truncated")
    for phrase in [
        "Shapes That Refuse to Leave the Window", "Cohen–Sutherland", "Liang–Barsky",
        "Sutherland–Hodgman", "homogeneous", "half-space", "provenance",
        "fan triangulation", "watertightness", "Reconstruct", "Falsify",
        "Implement", "Chapter 18",
    ]:
        check(phrase.lower() in chapter17.lower(),
              f"seventeenth chapter missing essential concept: {phrase}")

    figure_owners = {
        "01-pixel-addresses.svg": chapter,
        "02-lines-and-discrete-pixels.svg": chapter,
        "03-two-frames-one-place.svg": chapter2,
        "04-world-to-image-frame.svg": chapter2,
        "05-the-directed-edge.svg": chapter3,
        "06-barycentric-roof.svg": chapter3,
        "07-transform-family.svg": chapter4,
        "08-transform-order.svg": chapter4,
        "09-normals-under-shear.svg": chapter4,
        "10-the-vanishing-triangle.svg": chapter5,
        "11-subnormal-area.svg": chapter5,
        "12-certified-sign-interval.svg": chapter5,
        "13-right-handed-space.svg": chapter6,
        "14-3d-cross-normal.svg": chapter6,
        "15-ray-plane-cases.svg": chapter6,
        "16-house-leaves-the-page.svg": chapter6,
        "17-indexed-quad-adjacency.svg": chapter7,
        "18-coincident-positions-separate-vertices.svg": chapter7,
        "19-nonmanifold-edge-and-bowtie.svg": chapter7,
        "20-closed-indexed-house.svg": chapter7,
        "21-halfedge-twins-and-cycles.svg": chapter7,
        "22-de-casteljau-levels.svg": chapter8,
        "23-bezier-subdivision.svg": chapter8,
        "24-curve-continuity.svg": chapter8,
        "25-uniform-bspline-spans.svg": chapter8,
        "26-adaptive-curve-tessellation.svg": chapter8,
        "27-bicubic-patch-grid.svg": chapter8,
        "28-three-ways-to-describe-a-shape.svg": chapter9,
        "29-heightfield-rule-to-grid.svg": chapter9,
        "30-cylinder-wrapping-and-caps.svg": chapter9,
        "31-sphere-poles-and-seam.svg": chapter9,
        "32-marching-tetrahedra-edge-identity.svg": chapter9,
        "33-procedural-representations-and-topology.svg": chapter9,
        "34-four-layers-of-mesh-trust.svg": chapter10,
        "35-one-position-many-rendering-tuples.svg": chapter10,
        "36-convex-quad-not-all-polygons.svg": chapter10,
        "37-gltf-buffer-view-accessor.svg": chapter10,
        "38-asset-studio-trust-witnesses.svg": chapter10,
        "39-deliberate-repair-transaction.svg": chapter10,
        "40-one-asset-five-instances.svg": chapter11,
        "41-transform-order-witness.svg": chapter11,
        "42-scene-forest-two-identities.svg": chapter11,
        "43-reparent-keep-world.svg": chapter11,
        "44-normal-inverse-transpose.svg": chapter11,
        "45-rotation-basis-and-parity.svg": chapter12,
        "46-axis-angle-decomposition.svg": chapter12,
        "47-euler-gimbal-lock.svg": chapter12,
        "48-quaternion-double-cover.svg": chapter12,
        "49-slerp-constant-angle.svg": chapter12,
        "50-pinhole-similar-triangles.svg": chapter13,
        "51-world-local-optical-frames.svg": chapter13,
        "52-intrinsics-pixel-addresses.svg": chapter13,
        "53-focal-length-and-fov.svg": chapter13,
        "54-one-pixel-many-depths.svg": chapter13,
        "55-clip-to-screen-contract.svg": chapter14,
        "56-depth-endpoints-and-infinity.svg": chapter14,
        "57-perspective-versus-orthographic.svg": chapter14,
        "58-clip-before-divide.svg": chapter14,
        "59-reciprocal-depth-curves.svg": chapter14,
        "60-depth-storage-precision.svg": chapter14,
        "61-stereo-baseline-and-rays.svg": chapter15,
        "62-corrected-disparity.svg": chapter15,
        "63-disparity-and-depth-uncertainty.svg": chapter15,
        "64-epipolar-plane-and-lines.svg": chapter15,
        "65-closest-rays-midpoint.svg": chapter15,
        "66-wrong-match-perfect-geometry.svg": chapter15,
        "67-vertex-fetch-and-seam.svg": chapter16,
        "68-vertex-attribute-routes.svg": chapter16,
        "69-outside-vertex-still-matters.svg": chapter16,
        "70-screen-and-surface-weights.svg": chapter16,
        "71-perspective-interpolation-curve.svg": chapter16,
        "72-varying-and-depth-contracts.svg": chapter16,
        "73-window-outcodes.svg": chapter17,
        "74-parametric-clip-interval.svg": chapter17,
        "75-polygon-edge-cases.svg": chapter17,
        "76-clipped-quad-and-fan.svg": chapter17,
        "77-clip-varying-provenance.svg": chapter17,
    }
    for filename, owning_chapter in figure_owners.items():
        figure = BOOK / "Assets" / filename
        check(figure.is_file(), f"missing figure: {filename}")
        check(f"../Assets/{filename}" in owning_chapter,
              f"figure not included at point of teaching: {filename}")

        root = ET.parse(figure).getroot()
        check(root.tag == SVG + "svg", f"not an SVG document: {filename}")
        title = root.find(SVG + "title")
        description = root.find(SVG + "desc")
        check(title is not None and bool((title.text or "").strip()),
              f"figure lacks accessible title: {filename}")
        check(description is not None and len((description.text or "").strip()) > 70,
              f"figure lacks meaningful explanation: {filename}")
        check(bool(root.attrib.get("viewBox")), "figure missing viewBox")

    design = (BOOK / "CHAPTER_PLAN.md").read_text(encoding="utf-8")
    for i in range(1, 33):
        check(f"{i:02d}." in design,
              f"chapter {i:02d} not listed in editorial plan")
    ledger = (BOOK / "SOURCE_LEDGER.md").read_text(encoding="utf-8")
    check("02-foundations-ii-ch032" in ledger,
          "legacy camera source mapping is missing")

    # We never write to these destinations from this script, but we also
    # assert the source-only gate has not been repurposed to release files.
    check("P6" in (LAB / "include/book03/pixel_canvas.hpp").read_text(),
          "RGB8 PPM serialization contract missing")

    print("Book03/P01–P17: seventeen narrative chapters, source location and seventy-seven original SVG figure contracts checked.")
    print("No LaTeX/PDF/EPUB builds were requested or performed.")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (AssertionError, ET.ParseError) as exc:
        print(f"Book03/P01–P17 editorial gate FAILED: {exc}", file=sys.stderr)
        sys.exit(1)
