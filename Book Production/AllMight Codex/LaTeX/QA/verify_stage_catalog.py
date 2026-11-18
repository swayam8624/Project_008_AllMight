#!/usr/bin/env python3
"""Independently audit 7-stage chapter and companion first-use assignment."""
import argparse
import collections
import json
from pathlib import Path


def read(path):
    return json.loads(Path(path).read_text(encoding="utf-8"))


def main():
    here = Path(__file__).resolve().parent
    production = here.parent.parent / "production-manifest.json"
    catalog_path = here.parent / "Tools/stage-catalog.json"
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--production-manifest", type=Path, default=production)
    parser.add_argument("--catalog", type=Path, default=catalog_path)
    parser.add_argument("--plan", type=Path, required=True, help="approved companion assignment plan")
    parser.add_argument("--link-targets", type=Path, required=True, help="chapter/source link-target ledger")
    parser.add_argument("--output", type=Path, default=here / "stage-catalog-audit.json")
    args = parser.parse_args()

    pm, catalog, plan, links_doc = map(read, (args.production_manifest, args.catalog, args.plan, args.link_targets))
    chapters = {ch["id"]: ch for vol in pm["volumes"] for ch in vol["chapters"]}
    stages = catalog.get("stages", [])
    stage_by_chapter, order = {}, {}
    issues = []
    for stage_index, stage in enumerate(stages):
        for chapter_index, chapter_id in enumerate(stage.get("chapters", []), 1):
            if chapter_id not in chapters:
                issues.append({"kind": "catalog_unknown_chapter", "stage": stage.get("id"), "chapter_id": chapter_id})
            if chapter_id in stage_by_chapter:
                issues.append({"kind": "chapter_assigned_more_than_once", "chapter_id": chapter_id,
                               "stages": [stage_by_chapter[chapter_id], stage.get("id")]})
            stage_by_chapter[chapter_id] = stage.get("id")
            order[chapter_id] = chapter_index
    for chapter_id in sorted(set(chapters) - set(stage_by_chapter)):
        issues.append({"kind": "chapter_unassigned", "chapter_id": chapter_id})
    if len(stages) != 7:
        issues.append({"kind": "stage_count_mismatch", "expected": 7, "actual": len(stages)})

    stage_rank = {stage["id"]: index for index, stage in enumerate(stages)}
    target_to_assignment = collections.defaultdict(set)
    assignments = plan.get("assignments", [])
    for index, assignment in enumerate(assignments):
        for target in assignment.get("link_targets", []):
            for target_id in (target.get("target_id"), target.get("base_target_id")):
                if target_id:
                    target_to_assignment[target_id].add(index)
    collisions = {target: sorted(indices) for target, indices in target_to_assignment.items() if len(indices) > 1}
    for target, indices in collisions.items():
        issues.append({"kind": "companion_target_id_collision", "target_id": target, "assignment_indices": indices})

    source_to_chapter = {str(Path(ch["source"]).resolve()): ch["id"] for ch in chapters.values()}
    first_use = {}
    chapter_link_counts = collections.Counter()
    for record in links_doc.get("records", []):
        chapter_id = source_to_chapter.get(str(Path(record.get("source", "")).resolve()))
        if chapter_id is None:
            continue
        assignment_indices = target_to_assignment.get(record.get("target_id"), set())
        if not assignment_indices:
            continue
        key = (stage_rank.get(stage_by_chapter[chapter_id], 10**6), order[chapter_id])
        for assignment_index in assignment_indices:
            chapter_link_counts[chapter_id] += 1
            candidate = (key, chapter_id)
            if assignment_index not in first_use or candidate[0] < first_use[assignment_index][0]:
                first_use[assignment_index] = candidate

    legacy_chapters = collections.defaultdict(list)
    for volume in pm["volumes"]:
        for chapter in volume["chapters"]:
            if chapter["id"] in stage_by_chapter:
                legacy_chapters[volume["id"]].append(chapter["id"])
    companion_records = []
    fallback_count = 0
    assigned_paths = collections.Counter(a.get("source_path") for a in assignments)
    for index, assignment in enumerate(assignments):
        method = "direct_chapter_first_use"
        if index in first_use:
            chapter_id = first_use[index][1]
        else:
            method = "assigned_volume_fallback"
            fallback_count += 1
            candidates = legacy_chapters.get(assignment.get("assigned_volume"), [])
            if not candidates:
                issues.append({"kind": "companion_without_first_use_or_fallback", "source": assignment.get("source_path")})
                chapter_id = None
            else:
                chapter_id = min(candidates, key=lambda c: (stage_rank[stage_by_chapter[c]], order[c]))
        if assigned_paths[assignment.get("source_path")] != 1:
            issues.append({"kind": "companion_source_not_unique", "source": assignment.get("source_path"),
                           "count": assigned_paths[assignment.get("source_path")]})
        companion_records.append({
            "source_path": assignment.get("source_path"), "source_sha256": assignment.get("source_sha256"),
            "title": assignment.get("title"), "appendix_id": "appendix-" + __import__("hashlib").sha256(assignment.get("source_path", "").encode()).hexdigest()[:12],
            "assigned_volume": assignment.get("assigned_volume"), "first_use_chapter": chapter_id,
            "stage_id": stage_by_chapter.get(chapter_id), "assignment_method": method,
        })
    if len(assignments) != 439:
        issues.append({"kind": "companion_count_mismatch", "expected": 439, "actual": len(assignments)})
    if len(first_use) + fallback_count != len(assignments):
        issues.append({"kind": "companion_first_use_coverage_mismatch", "direct": len(first_use),
                       "fallback": fallback_count, "total": len(assignments)})

    output = {
        "schema": "allmight-stage-catalog-audit-v1",
        "scope": "logical 7-stage chapter assignment and companion first-use only; physical part/PDF coverage is separate",
        "inputs": {"production_manifest": str(args.production_manifest.resolve()), "catalog": str(args.catalog.resolve()),
                   "plan": str(args.plan.resolve()), "link_targets": str(args.link_targets.resolve())},
        "stage_count": len(stages), "chapter_count": len(chapters),
        "catalog_chapter_rows": sum(len(s.get("chapters", [])) for s in stages),
        "unique_catalog_chapters": len(stage_by_chapter), "chapter_counts_by_stage": {
            s["id"]: len(s.get("chapters", [])) for s in stages},
        "companion_count": len(assignments), "direct_first_use_count": len(first_use),
        "assigned_volume_fallback_count": fallback_count,
        "companions_by_first_use_stage": dict(collections.Counter(r["stage_id"] for r in companion_records)),
        "companions": companion_records,
        "issues": issues, "status": "pass" if not issues else "fail",
    }
    args.output.write_text(json.dumps(output, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"status": output["status"], "stages": len(stages), "chapters": len(chapters),
                      "direct_companions": len(first_use), "fallback_companions": fallback_count,
                      "issues": len(issues), "report": str(args.output.resolve())}, indent=2))
    return 0 if not issues else 1


if __name__ == "__main__":
    raise SystemExit(main())
