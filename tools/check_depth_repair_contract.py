#!/usr/bin/env python3
"""Structural smoke checks for the PROJECTALLMIGHT depth-repair layer.

This is intentionally weaker than semantic mastery. It prevents obvious
regressions: disappearing evidence artifacts, tiny placeholder concept cards,
or master volumes losing the reconstruction checkpoints added by the repair.
"""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CONCEPTS = ROOT / "Concepts"
CONTINUOUS = ROOT / "Continuous Notes"

CANONICAL = {
    "foundations": CONTINUOUS / "Computing Foundations - From Bits to C++ and Systems APIs.md",
    "engine": CONTINUOUS / "Game Engine - From First Frame to Living Worlds.md",
    "graphics": CONTINUOUS / "University - Worlds, Images and Inference - Graphics, Vision and Photography.md",
}

REQUIRED_ARTIFACTS = [
    "tools/labs/phase29_ml_runtime_lab.cpp",
    "tools/labs/ecs_reconstruction_lab.cpp",
    "tools/labs/spatial_index_lab.cpp",
    "tools/labs/raster_reference_lab.cpp",
    "tools/labs/pbr_reference_lab.cpp",
    "tools/labs/monte_carlo_transport_lab.cpp",
    "tools/labs/realtime_rendering_reference_lab.cpp",
    "tools/labs/gaussian_projection_reference_lab.cpp",
    "tools/labs/mesh_topology_lab.cpp",
    "tools/labs/dual_quaternion_skinning_lab.cpp",
    "tools/labs/crypto_primitives_lab.cpp",
    "tools/labs/network_loopback_lab.cpp",
    "tools/labs/posix_vm_file_lab.cpp",
    "tools/labs/production_pipeline_contract_lab.cpp",
    "tools/labs/ecs_scheduler_hierarchy_lab.cpp",
    "tools/labs/gaussian_tile_conic_lab.cpp",
    "tools/labs/qem_edge_collapse_lab.cpp",
    "tools/labs/path_transport_scene_lab.cpp",
    "tools/labs/raster_pipeline_state_lab.cpp",
    "tools/labs/pbr_extended_reference_lab.cpp",
    "tools/labs/bvh_sah_refit_lab.cpp",
    "tools/labs/constraint_friction_island_lab.cpp",
    "tools/labs/gguf_metadata_tensor_lab.cpp",
    "tools/native/vulkan_sync_smoke.cpp",
    "tools/native/metal_compile_smoke.metal",
    "Concepts/Mastery evidence levels.md",
]

REQUIRED_MASTER_MARKERS = {
    "foundations": [
        "## From model bytes to a validated compressed-inference runtime",
        "tools/labs/phase29_ml_runtime_lab.cpp",
        "tools/labs/network_loopback_lab.cpp",
    ],
    "engine": [
        "### Reconstruction checkpoint — prove identity survives relocation",
        "### Native-API evidence is a separate layer from the logical renderer",
        "### Publication reconstruction — make DCC output pass through one engine contract",
    ],
    "graphics": [
        "### Reconstruction checkpoint — make the coverage rule executable",
        "### Reconstruction checkpoint — test the microfacet equations, not only the picture",
        "### Reconstruction checkpoint — project a Gaussian before optimizing the rasterizer",
    ],
}


def main() -> None:
    missing = [p for p in REQUIRED_ARTIFACTS if not (ROOT / p).is_file()]
    if missing:
        raise RuntimeError("missing required depth-repair artifacts: " + ", ".join(missing))

    tiny = []
    for path in sorted(CONCEPTS.glob("*.md")):
        size = path.stat().st_size
        if size < 1000:
            tiny.append((path.name, size))
    if tiny:
        details = ", ".join(f"{name}={size}B" for name, size in tiny)
        raise RuntimeError(
            "concept cards below the 1000-byte smoke floor; deepen or explicitly "
            f"redesign the contract instead of padding blindly: {details}"
        )

    live = sorted(p.name for p in CONTINUOUS.glob("*.md"))
    expected = sorted(p.name for p in CANONICAL.values())
    if live != expected:
        raise RuntimeError(f"canonical continuous-note set changed: {live}")

    for key, path in CANONICAL.items():
        text = path.read_text(encoding="utf-8")
        for marker in REQUIRED_MASTER_MARKERS[key]:
            if marker not in text:
                raise RuntimeError(f"{path.name}: missing repair marker {marker!r}")

    print(f"PASS: {len(list(CONCEPTS.glob('*.md')))} concept notes; none below 1000 bytes")
    print(f"PASS: {len(REQUIRED_ARTIFACTS)} required depth-repair artifacts present")
    print("PASS: reconstruction checkpoints remain in all three canonical volumes")
    print("LIMIT: structural depth floor is not a semantic mastery certificate")


if __name__ == "__main__":
    main()
