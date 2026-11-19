#!/usr/bin/env python3
"""Source-preserving figure, math, and KDP checks for Book 1."""
from __future__ import annotations
from hashlib import sha256
from pathlib import Path
import re
import shutil
import subprocess

MERMAID = re.compile(r"(?m)^```mermaid(?:[ \t]+[^\n]*)?\n(?P<body>[\s\S]*?)^```[ \t]*$")
DISPLAY_MATH = re.compile(r"(?s)\$\$(.*?)\$\$")
EMPTY_BOX = re.compile(r"\\boxed\s*\{\s*\}", re.MULTILINE)
KDP_MAX_WHITE_BW = 828


def digest(payload: bytes) -> str:
    return sha256(payload).hexdigest()



# Earlier drafts used hundreds of Markdown thematic breaks as chapter dividers.
# Pandoc turns them into drawn rules plus white space. In the reader edition
# only, promote a following single-line bold concept to a compact heading;
# otherwise convert a thematic break to a very small, deliberate space.
# Fenced source/diagram blocks must remain bit-for-bit unchanged.
ACADEMIC_HEADINGS = {
    "GPU EXECUTION ARCHITECTURE": "GPU Execution Architecture",
    "MEMORY, DIVERGENCE, SUBGROUPS, ATOMICS & ORDERING":
        "GPU Memory, Divergence, Subgroups, Atomics, and Ordering",
    "GENERAL COMPUTE & PARALLEL ALGORITHMS":
        "General-Purpose Compute and Parallel Algorithms",
    "THE BUILD GRAPH ITSELF": "The Build Dependency Graph",
    "REPRODUCIBILITY, CI, TESTING, DIAGNOSTICS, PROFILING AND THE COMPLETE ENGINE FLOW":
        "Reproducibility, Continuous Integration, Testing, and Profiling",
    "GPU-RESIDENT DATA STRUCTURES & GPU-DRIVEN WORK":
        "GPU-Resident Data Structures and Device-Driven Work",
    "COMMAND EXECUTION, ASYNC WORK & MEMORY LIFETIMES":
        "Asynchronous Execution and Memory Lifetimes",
    "PERFORMANCE MODELING & PRINCIPLED GPU REDESIGN":
        "Performance Models and Evidence-Guided GPU Design",
    "VULKAN PHILOSOPHY, INSTANCE, DEVICES, QUEUES, SURFACE & SWAPCHAIN":
        "Vulkan: Instances, Devices, Queues, Surfaces, and Swapchains",
    "BUFFERS, IMAGES, MEMORY & UPLOADS":
        "Buffers, Images, Memory, and Uploads",
    "DESCRIPTORS, PIPELINE LAYOUTS, SHADERS & RENDERING STATE":
        "Descriptors, Pipeline Layouts, Shaders, and Render State",
    "COMMAND POOLS, COMMAND BUFFERS, SUBMISSION & SEMAPHORES":
        "Command Pools, Submission, and Semaphores",
    "VULKAN MEMORY MODEL, BARRIERS & MULTI-FRAME LIFETIMES":
        "Vulkan Memory Dependencies and Multi-Frame Lifetimes",
    "TIMESTAMPS, VALIDATION, DEVICE LOSS, MOLTENVK & EXPLICIT RAII":
        "Timestamps, Validation, Device Loss, MoltenVK, and RAII",
    "METAL'S CORE EXECUTION MODEL": "The Metal Execution Model",
    "STORAGE MODES, TILE MEMORY, DRAWABLES, FRAMES IN FLIGHT, HEAPS & SYNCHRONIZATION":
        "Storage, Tile Memory, Frame Resources, and Synchronization",
    "PROFILING, OBJECTIVE-C++ OWNERSHIP, SWIFTUI & THREAD BOUNDARIES":
        "Profiling and Apple Platform Ownership Boundaries",
}
FENCE = re.compile(r"(?ms)^" + re.escape(chr(96) * 3) +
                   r"[^\n]*\n.*?^" + re.escape(chr(96) * 3) + r"[ \t]*$")
THEMATIC_BOLD = re.compile(
    r"(?m)^[ \t]*---[ \t]*\n(?:[ \t]*\n)*\*\*([^\n*]{3,120})\*\*[ \t]*\n"
)
THEMATIC_REST = re.compile(r"(?m)^[ \t]*---[ \t]*$")


def prepare_reading_structure(source: str) -> tuple[str, dict]:
    """Format-only reflow with provenance; never change code or math text."""
    report = {"promoted_concept_headings": 0, "compacted_separators": 0,
              "academic_heading_edits": 0}

    def prose(segment: str) -> str:
        def rename(match: re.Match[str]) -> str:
            title = match.group(2)
            replacement = ACADEMIC_HEADINGS.get(title)
            if replacement:
                report["academic_heading_edits"] += 1
                return match.group(1) + replacement
            return match.group(0)

        segment = re.sub(r"(?m)^(#{2,5} +)([^\n]+)$", rename, segment)
        def promote(match: re.Match[str]) -> str:
            text = match.group(1).strip()
            if text.endswith(".") or len(text) > 105:
                return "\\smallskip\n\n**" + text + "**\n"
            report["promoted_concept_headings"] += 1
            return "\n\n#### " + text + "\n\n"

        segment = THEMATIC_BOLD.sub(promote, segment)
        count = len(THEMATIC_REST.findall(segment))
        report["compacted_separators"] += count
        segment = THEMATIC_REST.sub(r"\\smallskip", segment)
        return segment

    pieces: list[str] = []
    offset = 0
    for match in FENCE.finditer(source):
        pieces.append(prose(source[offset:match.start()]))
        pieces.append(match.group(0))
        offset = match.end()
    pieces.append(prose(source[offset:]))
    return "".join(pieces), report


def math_preflight(source: str, label: str) -> dict:
    no_code = re.sub(r"(?ms)^```[^\n]*\n.*?^```[ \t]*$", "", source)
    blocks = DISPLAY_MATH.findall(no_code)
    empty = [i for i, b in enumerate(blocks, 1) if not b.strip()]
    broken = [i for i, b in enumerate(blocks, 1) if EMPTY_BOX.search(b)]
    if empty or broken:
        raise ValueError(f"{label}: empty math {empty}; empty boxed expressions {broken}")
    return {"display_math": len(blocks),
            "boxed_math": sum(len(re.findall(r"\\boxed\s*\{", b)) for b in blocks),
            "_original_math_bodies": [b.strip() for b in blocks]}



FENCE_LANG_ID = re.compile(
    r'(?m)^' + re.escape(chr(96) * 3)
    + r'(?P<lang>[A-Za-z][\w.+-]*)[ \t]+id="[^"\n]+"[ \t]*$'
)


def normalize_code_fence_metadata(source: str) -> tuple[str, int]:
    """Normalize Obsidian code-fence IDs in derived Markdown only."""
    result, count = FENCE_LANG_ID.subn(
        lambda match: chr(96) * 3 + match.group("lang"),
        source,
    )
    remaining = re.findall(
        r'(?m)^' + re.escape(chr(96) * 3)
        + r'[^\n]*[ \t]+id="[^"\n]+"[ \t]*$',
        result,
    )
    if remaining:
        raise ValueError("Unsupported code-fence metadata: " + repr(remaining[:3]))
    return result, count


def sanitize_source_math_for_tex(body: str) -> str:
    """Escape unescaped Markdown percentage characters for valid LaTeX math.

    Without this transform, a literal percentage such as 99% is a LaTeX
    comment marker and may swallow the closing braces of a boxed equation.
    Already escaped \% remains unchanged. Original Markdown stays intact.
    """
    return re.sub(r'(?<!\\)%', lambda _: r'\%', body)


def plan_math(source: str, chapter: int) -> tuple[str, list[dict]]:
    """Protect every display formula from Pandoc's Markdown math heuristics.

    A source display is replaced by one unique plain-text marker during
    Markdown-to-TeX conversion. The exact formula is reinserted as raw TeX
    afterward, before any source-preserving TeX safety escaping. This prevents
    implicit Markdown interpretation from dropping a boxed expression.
    Fenced code examples remain untouched.
    """
    equations: list[dict] = []

    def protect_prose(segment: str) -> str:
        def replace(match: re.Match[str]) -> str:
            body = match.group(1)
            if not body.strip():
                raise ValueError(f"Chapter {chapter}: empty display expression")
            index = len(equations) + 1
            token = f"ALLMIGHTMATHCH{chapter:02d}INDEX{index:04d}"
            equations.append({
                "index": index,
                "token": token,
                "body": body,
                "sha256": digest(body.encode("utf-8")),
            })
            return "\n\n" + token + "\n\n"
        return DISPLAY_MATH.sub(replace, segment)

    pieces: list[str] = []
    offset = 0
    for match in FENCE.finditer(source):
        pieces.append(protect_prose(source[offset:match.start()]))
        pieces.append(match.group(0))
        offset = match.end()
    pieces.append(protect_prose(source[offset:]))
    expected = len(DISPLAY_MATH.findall(FENCE.sub("", source)))
    if len(equations) != expected:
        raise ValueError(
            f"Chapter {chapter}: expected {expected} source math displays, "
            f"protected {len(equations)}. No math may be omitted."
        )
    return "".join(pieces), equations


def replace_math_tokens(tex: str, equations: list[dict]) -> str:
    """Restore original math text exactly once at its original position."""
    for entry in equations:
        token = entry["token"]
        if digest(entry["body"].encode("utf-8")) != entry["sha256"]:
            raise ValueError("Math source identity changed: " + token)
        regex = r"(?m)^" + re.escape(token) + r"$"
        count = len(re.findall(regex, tex))
        if count != 1:
            raise ValueError(
                f"Math marker {token} occurred {count} times in Pandoc TeX"
            )
        original = "\\[\n" + sanitize_source_math_for_tex(entry["body"].strip()) + "\n\\]"
        tex = re.sub(regex, lambda _: original, tex, count=1)
    if "ALLMIGHTMATHCH" in tex:
        raise ValueError("Unresolved display-math marker in TeX")
    return tex


def plan_figures(source: str, chapter: int, part: int, directory: Path, output: Path,
                 first_index: int = 1):
    """Plan every Mermaid figure without changing the canonical chapter."""
    figure_dir = directory / "content" / "figures"
    figure_dir.mkdir(parents=True, exist_ok=True)
    entries = []

    def translate(match):
        body = match.group("body").strip() + "\n"
        if not body.strip():
            raise ValueError(f"Chapter {chapter}: empty Mermaid code")
        idx = first_index + len(entries)
        basename = f"chapter-{chapter:02d}-diagram-{idx:02d}"
        mmd = figure_dir / (basename + ".mmd")
        png = figure_dir / (basename + ".png")
        token = f"ALLMIGHTFIGURECH{chapter:02d}INDEX{idx:03d}"
        headings = re.findall(r"(?m)^#{2,5} +([^\n]+)$", source[:match.start()])
        lead = headings[-1] if headings else "Computational data flow"
        lead = re.sub(r"[`*_\[\]]", "", lead).strip()
        lead = re.sub(r"\s+", " ", lead)[:96]
        mmd.write_text(body, encoding="utf-8")
        entries.append({
            "chapter": chapter, "part": part, "index": idx, "token": token,
            "diagram_sha256": digest(body.encode("utf-8")),
            "source": mmd.relative_to(output).as_posix(),
            "image": png.relative_to(output).as_posix(),
            "caption": f"Figure {chapter}.{idx}: {lead}",
        })
        return "\n\n" + token + "\n\n"

    revised, matched = MERMAID.subn(translate, source)
    declared = len(re.findall(r"(?m)^```mermaid(?:[ \t]|$)", source))
    if matched != declared:
        raise ValueError(f"Chapter {chapter}: {declared} Mermaid fences; {matched} matched")
    return revised, entries


def replace_figure_tokens(tex: str, figures: list[dict]) -> str:
    for item in figures:
        token = item["token"]
        path = f'../book01-expanded-p{item["part"]:02d}/content/figures/' + Path(item["image"]).name
        caption = item["caption"].replace("\\", "")
        for reserved, escaped in (("&", r"\&"), ("%", r"\%"), ("#", r"\#"),
                                  ("_", r"\_"), ("{", r"\{"), ("}", r"\}"),
                                  ("$", r"\$"), ("~", r"\textasciitilde{}"),
                                  ("^", r"\textasciicircum{}")):
            caption = caption.replace(reserved, escaped)
        # Use the LaTeX figure counter, not a hand-printed "Figure 1.2:"
        # prefix. This gives correct references and a genuine list of figures.
        title = re.sub(r"^Figure\s+\d+\.\d+\s*:\s*", "", caption)
        latex = (
            "\\par\\begin{center}\n"
            "\\includegraphics[width=.94\\linewidth,height=.34\\textheight,"
            "keepaspectratio]{" + path + "}\n"
            "\\captionof{figure}{" + title + "}"
            "\\label{fig:book01:" + str(item["chapter"]) + ":" +
            str(item["index"]) + "}\n"
            "\\end{center}\\smallskip\n"
        )
        pattern = r"(?m)^" + re.escape(token) + r"$"
        if len(re.findall(pattern, tex)) != 1:
            raise ValueError(f"Missing/duplicate diagram marker: {token}")
        tex = re.sub(pattern, lambda _: latex, tex, count=1)
    if "ALLMIGHTFIGURECH" in tex:
        raise ValueError("Unresolved Mermaid marker in TeX")
    return tex


def check_math_rendered(source_stats: dict, tex: str, chapter: int):
    """Require every original display formula's TeX body to survive Pandoc."""
    generated = len(re.findall(r"\\boxed\s*\{", tex))
    if generated < source_stats["boxed_math"]:
        raise ValueError(
            f"Chapter {chapter}: source has {source_stats['boxed_math']} boxed "
            f"equations but generated TeX only {generated}"
        )
    def normalize(s: str) -> str:
        # Underscores inside generated texttt are escaped for compilability.
        return re.sub(r"\s+", "", s).replace(r"\_", "_").replace(r"\%", "%")
    output = normalize(tex)
    missing = [index for index, body in
               enumerate(source_stats.get("_original_math_bodies", []), 1)
               if normalize(body) not in output]
    if missing:
        raise ValueError(
            f"Chapter {chapter}: {len(missing)} display equations were "
            f"altered, split, or omitted in generated TeX; display-math "
            f"block numbers: {missing[:20]}"
        )


def render_figures(manifest: dict, output: Path):
    figures = manifest.get("diagrams", [])
    if not figures:
        print("No Mermaid diagrams declared")
        return
    binary = shutil.which("mmdc")
    if not binary:
        raise RuntimeError(
            f"{len(figures)} diagrams must be rendered. Install Mermaid CLI: "
            "brew install node; npm install -g @mermaid-js/mermaid-cli"
        )
    for item in figures:
        src, dst = output / item["source"], output / item["image"]
        if not src.is_file() or digest(src.read_bytes()) != item["diagram_sha256"]:
            raise ValueError("Changed or missing diagram source: " + str(src))
        if dst.is_file() and dst.stat().st_size > 500 and item.get("render_sha256") == digest(dst.read_bytes()):
            continue
        result = subprocess.run([binary, "-i", str(src), "-o", str(dst),
                                 "-s", "3", "-t", "neutral", "-b", "white"],
                                capture_output=True, text=True)
        if result.returncode or not dst.is_file() or dst.stat().st_size < 500:
            raise RuntimeError(f"Mermaid failed: chapter {item['chapter']} "
                               f"diagram {item['index']}\n" +
                               (result.stderr or result.stdout)[-4000:])
        item["render_sha256"] = digest(dst.read_bytes())
    print("Verified rendered diagrams:", len(figures))


def pdf_preflight(pdf: Path, *, paperback: bool) -> int:
    try:
        from pypdf import PdfReader
    except ImportError as exc:
        raise RuntimeError("PDF preflight needs: python3 -m pip install pypdf") from exc
    pages = PdfReader(str(pdf)).pages
    count = len(pages)
    if count < 1:
        raise RuntimeError("PDF has no pages")
    if paperback:
        if count > KDP_MAX_WHITE_BW:
            raise RuntimeError(f"KDP paperback REFUSED: {count} > {KDP_MAX_WHITE_BW} pages")
        for index, page in enumerate(pages, 1):
            width, height = float(page.mediabox.width), float(page.mediabox.height)
            if abs(width - 504) > .5 or abs(height - 720) > .5:
                raise RuntimeError(f"KDP trim mismatch at PDF page {index}")
    return count
