#!/usr/bin/env python3
"""Read-only structural audit of staged Book01 revision, NOT a quality verdict.

The sixteen staged additions and their original source anchors must be
individually identifiable. This does not compile C++, validate hardware,
verify papers, or assess the truth of narrative explanations. No source files
are changed. --allow-applied is for the later authorized integration stage.
"""
from __future__ import annotations
import argparse
from dataclasses import asdict, dataclass
import hashlib
import importlib.util
import json
from pathlib import Path
import re
import statistics
import sys

ROOT=Path(__file__).resolve().parent.parent
MASTER=ROOT/"Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md"
EDITION=ROOT/"Chapter Edition/01 - Computing Foundations"
MAP=EDITION/"00 - Reading Map.md"
CMAKE=ROOT/"Book Production/Seven-Book Curriculum/Book01/CMakeLists.txt"
EXPECTED_BLOB="d3202af31e1eb644a74565eeb63e7a3352438823"

def read_module(filename: str, module_name: str):
    path=ROOT/"tools"/filename
    spec=importlib.util.spec_from_file_location(module_name,path)
    if spec is None or spec.loader is None:
        raise RuntimeError("Cannot import "+str(path))
    module=importlib.util.module_from_spec(spec)
    sys.modules[module_name]=module
    spec.loader.exec_module(module)
    return module

def git_blob(data: bytes)->str:
    return hashlib.sha1(("blob "+str(len(data))+"\0").encode()+data).hexdigest()

def measure_fences(text: str)->int:
    return sum(line.startswith(chr(96)*3) for line in text.splitlines())

def audit(allow_applied: bool=False)->dict:
    faults=[]
    warnings=[]
    records=[]
    try:
        patcher=read_module("apply_cpp23_project_inserts.py","allmight_audit_patcher")
        evidence=read_module("nes_course_evidence.py","allmight_audit_evidence")
        original_bytes=MASTER.read_bytes()
        original_bytes.decode("utf-8")
        map_text=MAP.read_text("utf-8")
        cmake=CMAKE.read_text("utf-8")
    except (Exception) as e:
        return {"ok":False,"faults":["Unreadable audit input: "+str(e)],"warnings":[]}
    source_sha=git_blob(original_bytes)
    specs=list(patcher.INSERTS)
    targets=list(evidence.EXPECTED)
    mapped=re.findall(r"\[\[Chapter Edition/01 - Computing Foundations/(\d{2} - [^\]|]+)\|",map_text)
    executables=re.findall(r"(?m)^\s*add_executable\(\s*([^\s()]+)",cmake)
    ctests=re.findall(r"(?m)^\s*add_test\(\s*NAME\s+([^\s()]+)",cmake)
    if len(mapped)!=39: faults.append("Reading map does not have 39 chapters")
    if len(specs)!=16 or len(targets)!=16:
        faults.append("Staged insertion and source evidence inventories must both have 16")
    if len(executables)!=16 or len(ctests)!=16:
        faults.append("CMake executable and CTest registration counts must both have 16")
    if len(set(executables))!=len(executables) or len(set(ctests))!=len(ctests):
        faults.append("Duplicate executable/CTest name")
    if [x.name for x in specs]!=[x[0] for x in targets]:
        faults.append("Insertion IDs/order disagree with source evidence")
    if not allow_applied and source_sha!=EXPECTED_BLOB:
        faults.append("Canonical master Git blob changed during staged authoring")
    if allow_applied:
        try:
            staged,_=patcher.process(original_bytes)
            if staged!=original_bytes:
                warnings.append("Authorized lessons are not all applied")
        except Exception as e:
            faults.append("Invalid or modified applied insertion: "+str(e))
    for lab_id,target,source in targets:
        if target not in executables:
            faults.append(lab_id+" executable missing: "+target)
        if not (ROOT/source).is_file():
            faults.append(lab_id+" source missing: "+source)
    for path in evidence.EXTRA_SOURCES:
        if not (ROOT/path).is_file():
            faults.append("Provenance source missing: "+path)
    for critical in (ROOT/".github/workflows/allmight-cpp23-nes-course.yml",
                     ROOT/"Book Production/Seven-Book Curriculum/Book01/CMakePresets.json"):
        if not critical.is_file():
            faults.append("Missing build/CI configuration "+str(critical))
    for i in range(min(16,len(specs),len(mapped))):
        entry=mapped[i]
        lesson=specs[i]
        if not entry.startswith(f"{i+1:02d} - "):
            faults.append("Reading chapter order wrong at "+str(i+1))
            continue
        chapter_path=EDITION/(entry+".md")
        lesson_path=ROOT/lesson.source
        if not chapter_path.is_file() or not lesson_path.is_file():
            faults.append("Missing chapter/lesson "+entry+" / "+lesson.source)
            continue
        ch=chapter_path.read_text("utf-8")
        lab=lesson_path.read_text("utf-8")
        if not lab.startswith("### "+lesson.name+" "):
            faults.append("Lesson heading mismatch: "+lesson.name)
        if ch.count("<!-- CHAPTER-SOURCE-BEGIN -->")!=1 or ch.count("<!-- CHAPTER-SOURCE-END -->")!=1:
            faults.append("Derived source markers mismatch: "+entry)
        anchor=lesson.anchor.decode("utf-8").strip()
        anchor_count=sum(line==anchor for line in ch.splitlines())
        if anchor_count!=1:
            faults.append("Original chapter anchor not unique: "+lesson.name)
        if original_bytes.count(lesson.anchor)!=1:
            faults.append("Canonical master anchor not unique: "+lesson.name)
        cf=measure_fences(ch)
        lf=measure_fences(lab)
        if cf%2 or lf%2:
            warnings.append("Review uneven fenced Markdown: "+lesson.name)
        if lf==0:
            warnings.append("Lesson has no embedded fenced code, despite separate lab: "+lesson.name)
        widths=[len(line) for line in ch.splitlines() if line.strip()]
        median=statistics.median(widths) if widths else 0
        if median<20:
            warnings.append(f"{lesson.name} original has {median:g}-character median nonempty line: inspect narrative fragmentation")
        records.append({
            "chapter":i+1,
            "title":entry,
            "derived_blob":git_blob(ch.encode("utf-8")),
            "original_lines":len(ch.splitlines()),
            "original_chars":len(ch),
            "median_nonempty_line_width":median,
            "chapter_fences":cf,
            "lesson_id":lesson.name,
            "lesson_chars":len(lab),
            "lesson_lines":len(lab.splitlines()),
            "lesson_fences":lf,
            "lesson_url_occurrences":len(re.findall(r"https?://",lab)),
            "exact_source_anchor_count":anchor_count,
        })
    return {
        "ok":not faults,
        "scope":"structural source-only; never a technical completeness certificate",
        "canonical_git_blob":source_sha,
        "expected_unchanged_blob":EXPECTED_BLOB,
        "reading_map_chapters":len(mapped),
        "staged_additions":len(specs),
        "registered_executables":len(executables),
        "registered_ctests":len(ctests),
        "audited_chapters":len(records),
        "faults":faults,
        "warnings":warnings,
        "chapters":records,
        "compiler_results_verified":False,
        "native_gpu_results_verified":False,
        "editorial_or_research_completeness_verified":False,
    }

def main()->int:
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--json",action="store_true")
    parser.add_argument("--allow-applied",action="store_true")
    args=parser.parse_args()
    result=audit(allow_applied=args.allow_applied)
    if args.json:
        print(json.dumps(result,indent=2,ensure_ascii=False))
    else:
        print("Book01 source structure:", "PASS" if result["ok"] else "FAIL",
              "| chapters",result.get("audited_chapters",0),
              "| staged",result.get("staged_additions",0),
              "| CTests",result.get("registered_ctests",0))
        for fault in result.get("faults",[]):
            print("FAIL:",fault)
        for warning in result.get("warnings",[]):
            print("REVIEW:",warning)
        print("No compiler, native GPU, research or editorial-completeness claim")
    return 0 if result["ok"] else 1

if __name__=="__main__":
    raise SystemExit(main())
