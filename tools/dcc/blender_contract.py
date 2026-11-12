"""Headless Blender DCC-to-engine publication witness for PROJECTALLMIGHT.

Run inside Blender:
  blender --background --factory-startup --python tools/dcc/blender_contract.py -- /tmp/out
"""
from __future__ import annotations

import json
from pathlib import Path
import struct
import sys

import bpy


def require(condition: bool, message: str) -> None:
    if not condition:
        raise RuntimeError(message)


def output_dir() -> Path:
    if "--" not in sys.argv:
        raise RuntimeError("expected -- <output-dir>")
    args = sys.argv[sys.argv.index("--") + 1 :]
    require(len(args) == 1, "expected exactly one output directory")
    path = Path(args[0]).resolve()
    path.mkdir(parents=True, exist_ok=True)
    return path


def make_asset() -> bpy.types.Object:
    mesh = bpy.data.meshes.new("AllMightCubeMesh")
    vertices = [
        (-1.0, -1.0, -1.0),
        ( 1.0, -1.0, -1.0),
        ( 1.0,  1.0, -1.0),
        (-1.0,  1.0, -1.0),
        (-1.0, -1.0,  1.0),
        ( 1.0, -1.0,  1.0),
        ( 1.0,  1.0,  1.0),
        (-1.0,  1.0,  1.0),
    ]
    faces = [
        (0, 1, 2, 3),
        (4, 7, 6, 5),
        (0, 4, 5, 1),
        (1, 5, 6, 2),
        (2, 6, 7, 3),
        (4, 0, 3, 7),
    ]
    mesh.from_pydata(vertices, [], faces)
    mesh.update()
    require(len(mesh.vertices) == 8 and len(mesh.polygons) == 6, "source cube topology mismatch")

    collection = bpy.data.collections.new("AllMightExport")
    bpy.context.scene.collection.children.link(collection)

    obj = bpy.data.objects.new("AllMightCube", mesh)
    collection.objects.link(obj)
    obj["project_allmight_asset_id"] = "cube.reference.v1"
    obj["publication_schema"] = 1
    obj.location = (1.25, -2.0, 0.5)
    obj.rotation_euler = (0.0, 0.0, 0.25)
    return obj


def validate_glb(path: Path) -> None:
    data = path.read_bytes()
    require(len(data) >= 20, "GLB is too short")
    magic, version, declared_length = struct.unpack_from("<4sII", data, 0)
    require(magic == b"glTF", f"GLB magic mismatch: {magic!r}")
    require(version == 2, f"expected glTF 2 GLB, got version {version}")
    require(declared_length == len(data), "GLB declared length does not match file size")

    chunk_length, chunk_type = struct.unpack_from("<II", data, 12)
    require(chunk_type == 0x4E4F534A, "first GLB chunk is not JSON")
    start = 20
    end = start + chunk_length
    require(end <= len(data), "GLB JSON chunk exceeds file length")
    payload = data[start:end].decode("utf-8").rstrip(" \t\r\n\x00")
    doc = json.loads(payload)
    require(doc.get("asset", {}).get("version") == "2.0", "glTF asset.version is not 2.0")
    require(len(doc.get("meshes", [])) == 1, "expected one exported mesh")
    require(len(doc.get("nodes", [])) >= 1, "expected at least one exported node")


def export_and_reimport(out: Path) -> None:
    source = make_asset()

    for obj in bpy.context.selected_objects:
        obj.select_set(False)
    source.select_set(True)
    bpy.context.view_layer.objects.active = source

    blend_path = out / "allmight_source.blend"
    glb_path = out / "allmight_cube.glb"
    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path), check_existing=False)
    require(blend_path.is_file() and blend_path.stat().st_size > 0, "Blend publication failed")

    result = bpy.ops.export_scene.gltf(
        filepath=str(glb_path),
        export_format="GLB",
        use_selection=True,
        export_extras=True,
        check_existing=False,
    )
    require("FINISHED" in result, f"glTF export failed: {result}")
    validate_glb(glb_path)

    bpy.ops.wm.read_factory_settings(use_empty=True)
    result = bpy.ops.import_scene.gltf(filepath=str(glb_path))
    require("FINISHED" in result, f"glTF re-import failed: {result}")

    imported_meshes = [o for o in bpy.context.scene.objects if o.type == "MESH"]
    require(len(imported_meshes) == 1, f"expected one mesh after re-import, got {len(imported_meshes)}")
    imported = imported_meshes[0]
    require(len(imported.data.vertices) == 8, "re-imported cube vertex count changed")
    require(imported.get("project_allmight_asset_id") == "cube.reference.v1",
            "glTF extras did not preserve logical asset identity")
    require(int(imported.get("publication_schema", -1)) == 1,
            "glTF extras did not preserve publication schema")

    manifest = {
        "schema": 1,
        "source_blend": blend_path.name,
        "published_glb": glb_path.name,
        "logical_asset_id": imported["project_allmight_asset_id"],
        "mesh_vertices": len(imported.data.vertices),
        "blender_version": ".".join(map(str, bpy.app.version)),
    }
    (out / "blender_contract_receipt.json").write_text(
        json.dumps(manifest, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )


def main() -> None:
    out = output_dir()
    export_and_reimport(out)
    print("blender_contract: PASS")
    print("proved: background bpy execution, datablock/collection/object creation, custom publication metadata, .blend save, GLB export structure, GLB re-import, geometry and extras identity")
    print("not proved: Maya/Houdini/Nuke adapters, modifiers/armatures/material parity, texture packaging, unit/up-axis policy for arbitrary scenes, render-farm behavior, or production Kairo import parity")


if __name__ == "__main__":
    main()
