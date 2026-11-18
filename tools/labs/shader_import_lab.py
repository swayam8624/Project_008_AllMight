"""Serial stdlib model of shader import inputs, ABI gate, cache checks, and publication."""
from __future__ import annotations

import hashlib
import json
import os
import re
import shutil
import tempfile
import unittest
from pathlib import Path

INCLUDE = re.compile(r'^\s*#include\s+"([^"]+)"\s*$', re.MULTILINE)
BINDING = re.compile(r'^\s*//\s*@binding\s+(\d+)\s+(\w+)\s*$', re.MULTILINE)
IMPORTER_POLICY_VERSION = 1


class ImportErrorDetail(RuntimeError):
    pass


def sha(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def source_closure(root: Path, entry: Path) -> tuple[str, list[dict[str, str]]]:
    """Resolve a strict include graph; bytes are snapshotted once per canonical path."""
    root = root.resolve()
    visiting: list[Path] = []
    visited: set[Path] = set()
    snapshots: dict[Path, bytes] = {}
    records: list[dict[str, str]] = []

    def expand(path: Path) -> str:
        path = path.resolve()
        if root not in path.parents and path != root:
            raise ImportErrorDetail(f"include escapes source root: {path}")
        if path in visiting:
            cycle = " -> ".join(p.name for p in (*visiting, path))
            # This toy resolver has no macro/include-guard evaluation; its strict policy
            # rejects every recursive include. A real importer asks its preprocessor.
            raise ImportErrorDetail(f"strict include cycle: {cycle}")
        if path not in snapshots:
            try:
                snapshots[path] = path.read_bytes()
            except OSError as exc:
                raise ImportErrorDetail(f"missing include/source: {path}") from exc
        raw = snapshots[path]
        if path not in visited:
            records.append({"path": path.relative_to(root).as_posix(), "sha256": sha(raw)})
        visiting.append(path)
        text = raw.decode("utf-8")

        def replace(match: re.Match[str]) -> str:
            # Resolve relative includes from their requesting file, like an includer callback.
            return expand(path.parent / match.group(1))

        expanded = INCLUDE.sub(replace, text)
        visiting.pop()
        visited.add(path)
        return expanded

    preprocessed = expand(entry)
    # Stable paths and snapshotted bytes define the closure, never mtimes or checkout roots.
    records.sort(key=lambda item: item["path"])
    return preprocessed, records


def reflection_from(preprocessed: str) -> list[dict[str, object]]:
    """Mock reflection reads annotations; no shader compiler or GPU ABI is involved."""
    rows = [{"binding": int(n), "kind": kind} for n, kind in BINDING.findall(preprocessed)]
    if len({row["binding"] for row in rows}) != len(rows):
        raise ImportErrorDetail("duplicate reflected binding index")
    return sorted(rows, key=lambda row: int(row["binding"]))


def recipe_for(records: list[dict[str, str]], settings: dict[str, object],
               expected_abi: list[dict[str, object]]) -> dict[str, object]:
    # Keep user settings nested: a field named source_closure cannot replace file evidence.
    return {
        "source_closure": records,
        "settings": settings,
        "abi_contract": sorted(expected_abi, key=lambda row: int(row["binding"])),
        "abi_schema_version": settings["reflection_schema"],
        "importer_policy_version": IMPORTER_POLICY_VERSION,
    }


def variant_key(records: list[dict[str, str]], settings: dict[str, object],
                expected_abi: list[dict[str, object]]) -> str:
    recipe = recipe_for(records, settings, expected_abi)
    encoded = json.dumps(recipe, sort_keys=True, separators=(",", ":")).encode()
    return sha(encoded)


def begin_request(cache: Path, revision: int) -> None:
    """Record the newest requested revision before scheduling work for it."""
    cache.mkdir(parents=True, exist_ok=True)
    latest = cache / "latest-request"
    if latest.exists() and revision <= int(latest.read_text()):
        raise ImportErrorDetail("request revision must increase")
    temp = cache / ".latest-request.tmp"
    temp.write_text(f"{revision}\n")
    os.replace(temp, latest)


def validate_cache_entry(entry: Path, key: str, recipe: dict[str, object]) -> None:
    """Never trust a content-addressed directory solely because its name matches."""
    try:
        manifest = json.loads((entry / "manifest.json").read_text())
        binary = (entry / "shader.bin").read_bytes()
    except (OSError, json.JSONDecodeError) as exc:
        raise ImportErrorDetail(f"unreadable cache entry: {key}") from exc
    if not isinstance(manifest, dict):
        raise ImportErrorDetail(f"cache manifest is not an object: {key}")
    if manifest.get("key") != key or manifest.get("recipe") != recipe:
        raise ImportErrorDetail(f"cache manifest mismatch: {key}")
    if manifest.get("reflection") != recipe["abi_contract"]:
        raise ImportErrorDetail(f"cache reflected interface mismatch: {key}")
    if manifest.get("binary_sha256") != sha(binary):
        raise ImportErrorDetail(f"cache binary digest mismatch: {key}")


def publish_current(cache: Path, key: str, request_revision: int) -> bool:
    """Activate only if this result still belongs to the newest requested revision."""
    # Serial model: the caller must serialize this check and replacement.
    # An atomic file rename alone is NOT a revision compare-and-swap. Real
    # concurrent publication needs one per-asset lock/transaction around both.
    latest = int((cache / "latest-request").read_text())
    if latest != request_revision:
        return False
    pointer_tmp = cache / ".current.tmp"
    pointer_tmp.write_text(json.dumps({"key": key, "request_revision": request_revision}) + "\n")
    os.replace(pointer_tmp, cache / "current")
    return True


def import_shader(root: Path, entry: Path, cache: Path, settings: dict[str, object],
                  expected_abi: list[dict[str, object]], request_revision: int) -> tuple[str, bool]:
    """Build/reuse immutable artifact, then attempt revision-guarded pointer activation."""
    preprocessed, records = source_closure(root, entry)
    recipe = recipe_for(records, settings, expected_abi)
    key = variant_key(records, settings, expected_abi)
    reflected = reflection_from(preprocessed)
    expected = sorted(expected_abi, key=lambda row: int(row["binding"]))
    if reflected != expected:
        raise ImportErrorDetail(f"reflection ABI mismatch: got {reflected}, expected {expected}")
    if "#error" in preprocessed:
        raise ImportErrorDetail("mock compiler rejected #error directive")

    # Deterministic digest stands in for bytecode; it is not compiler output.
    binary = hashlib.sha256((preprocessed + json.dumps(recipe, sort_keys=True)).encode()).hexdigest().encode()
    cache.mkdir(parents=True, exist_ok=True)
    final_entry = cache / key
    if final_entry.exists():
        validate_cache_entry(final_entry, key, recipe)
    else:
        stage = Path(tempfile.mkdtemp(prefix=".candidate-", dir=cache))
        try:
            (stage / "shader.bin").write_bytes(binary)
            manifest = {"key": key, "recipe": recipe, "reflection": reflected,
                        "binary_sha256": sha(binary)}
            (stage / "manifest.json").write_text(json.dumps(manifest, sort_keys=True, indent=2) + "\n")
            # Same-filesystem rename exposes the completed directory as one cache entry.
            os.replace(stage, final_entry)
        finally:
            if stage.exists():
                shutil.rmtree(stage)
        validate_cache_entry(final_entry, key, recipe)
    # A late older job may keep its valid cache result but cannot move the active pointer back.
    return key, publish_current(cache, key, request_revision)


class ShaderImportTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temp = tempfile.TemporaryDirectory()
        self.base = Path(self.temp.name)
        self.src = self.base / "src"
        self.cache = self.base / "cache"
        (self.src / "inc").mkdir(parents=True)
        (self.src / "main.hlsl").write_text('#include "inc/common.hlsli"\nvoid main() {}\n')
        (self.src / "inc/common.hlsli").write_text('// @binding 0 uniform_buffer\n')
        self.settings = {"stage": "vertex", "entry": "main", "defines": {"SKINNED": 0},
                         "compiler": "mock-dxc", "version": "1.0", "backend": "dxil",
                         "profile": "vs_6_6", "optimization": "O2", "reflection_schema": 1,
                         "abi_contract_version": 1}
        self.abi = [{"binding": 0, "kind": "uniform_buffer"}]
        self.revision = 0

    def tearDown(self) -> None:
        self.temp.cleanup()

    def build(self, revision: int | None = None) -> tuple[str, bool]:
        if revision is None:
            self.revision += 1
            revision = self.revision
            begin_request(self.cache, revision)
        return import_shader(self.src, self.src / "main.hlsl", self.cache,
                             self.settings, self.abi, revision)

    def test_closure_settings_and_abi_contract_change_key(self) -> None:
        base, _ = self.build()
        (self.src / "inc/common.hlsli").write_text('// @binding 0 uniform_buffer\n// changed\n')
        self.assertNotEqual(base, self.build()[0])
        (self.src / "inc/common.hlsli").write_text('// @binding 0 uniform_buffer\n')
        stable_base, _ = self.build()
        original = dict(self.settings)
        variants = (("version", "2.0"), ("backend", "spirv"), ("profile", "vs_6_7"),
                    ("optimization", "O0"), ("stage", "fragment"), ("entry", "shade"),
                    ("defines", {"SKINNED": 1}), ("compiler", "mock-dxc-next"),
                    ("abi_contract_version", 2))
        for field, value in variants:
            self.settings = dict(original)
            before = self.build()[0]
            self.settings[field] = value
            self.assertNotEqual(before, self.build()[0], field)
        self.settings = original
        self.assertEqual(stable_base, self.build()[0])

    def test_reserved_setting_cannot_hide_include_change(self) -> None:
        # An ill-named user option remains nested under settings in the recipe.
        self.settings["source_closure"] = [{"path": "forged", "sha256": "0"}]
        base, _ = self.build()
        (self.src / "inc/common.hlsli").write_text('// @binding 0 uniform_buffer\nchanged\n')
        changed, _ = self.build()
        self.assertNotEqual(base, changed)

    def test_abi_or_compile_failure_keeps_last_good_pointer(self) -> None:
        good_key, activated = self.build()
        self.assertTrue(activated)
        pointer = self.cache / "current"
        self.assertEqual(json.loads(pointer.read_text())["key"], good_key)
        self.abi = [{"binding": 1, "kind": "uniform_buffer"}]
        with self.assertRaisesRegex(ImportErrorDetail, "ABI mismatch"):
            self.build()
        self.assertEqual(json.loads(pointer.read_text())["key"], good_key)
        self.abi = [{"binding": 0, "kind": "uniform_buffer"}]
        (self.src / "main.hlsl").write_text('#include "inc/common.hlsli"\n#error fail\n')
        with self.assertRaisesRegex(ImportErrorDetail, "mock compiler rejected"):
            self.build()
        self.assertEqual(json.loads(pointer.read_text())["key"], good_key)

    def test_stale_completion_caches_but_does_not_activate(self) -> None:
        begin_request(self.cache, 1)
        good_key, _ = self.build(revision=1)
        begin_request(self.cache, 2)  # newer import request has superseded revision 1
        older_key, activated = self.build(revision=1)
        self.assertEqual(older_key, good_key)
        self.assertFalse(activated)
        self.assertTrue((self.cache / older_key / "manifest.json").exists())
        self.assertEqual(json.loads((self.cache / "current").read_text())["key"], good_key)

    def test_tampered_cache_is_rejected_without_pointer_change(self) -> None:
        good_key, _ = self.build()
        pointer = self.cache / "current"
        (self.cache / good_key / "shader.bin").write_bytes(b"tampered")
        with self.assertRaisesRegex(ImportErrorDetail, "cache binary digest mismatch"):
            self.build()
        self.assertEqual(json.loads(pointer.read_text())["key"], good_key)

    def test_strict_cycle_and_missing_include_fail_before_publish(self) -> None:
        (self.src / "inc/common.hlsli").write_text('#include "../main.hlsl"\n')
        with self.assertRaisesRegex(ImportErrorDetail, "strict include cycle"):
            self.build()
        (self.src / "inc/common.hlsli").write_text('#include "absent.hlsli"\n')
        with self.assertRaisesRegex(ImportErrorDetail, "missing include"):
            self.build()
        self.assertFalse((self.cache / "current").exists())


if __name__ == "__main__":
    unittest.main(verbosity=2)
