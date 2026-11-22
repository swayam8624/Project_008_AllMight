#!/usr/bin/env python3
"""Source-preserving figure, math, and KDP checks for Book 1."""
from __future__ import annotations
from hashlib import sha256
from pathlib import Path
import os
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
FENCE = re.compile(r"(?ms)^(?P<marker>`{3,}|~{3,})[^\n]*\n"
                   r".*?^(?P=marker)[ \t]*$")
THEMATIC_BOLD = re.compile(
    r"(?m)^[ \t]*---[ \t]*\n(?:[ \t]*\n)*\*\*([^\n*]{3,120})\*\*[ \t]*\n"
)
THEMATIC_REST = re.compile(r"(?m)^[ \t]*---[ \t]*$")



def verify_fenced_blocks_unchanged(
    original: str, edited: str, chapter: int, stage: str,
    *, allow_pseudocode_labels: bool = False,
    allowed_new_shell_listings: int = 0,
) -> None:
    """Fail if any source code or diagram changes, disappears or moves.

    Reader editing may introduce a shell *listing* by promoting a long
    inline build command into a copyable block. That is an intentional,
    counted addition. Existing blocks still have to match in order, with
    identical payload hashes. An unexpected new Mermaid or C++ fence is
    not silently accepted.
    """
    def unpack(source: str) -> list[tuple[str, str]]:
        blocks = []
        for match in FENCE.finditer(source):
            raw = match.group(0)
            header, _, rest = raw.partition("\n")
            payload, delimiter, _ = rest.rpartition("\n" + match.group("marker"))
            if not delimiter:
                raise ValueError("Malformed closing code-fence marker")
            blocks.append((header, digest(payload.encode("utf-8"))))
        return blocks

    before = unpack(original)
    after = unpack(edited)
    if len(after) != len(before) + allowed_new_shell_listings:
        raise ValueError(
            f"Chapter {chapter}: fenced-block count changed during {stage}; "
            f"{len(before)} source blocks, {len(after)} reader blocks, "
            f"{allowed_new_shell_listings} authorized shell additions"
        )
    consumed = 0
    new_shell = 0
    for new_head, new_hash in after:
        matched = False
        if consumed < len(before):
            old_head, old_hash = before[consumed]
            permitted_label = (
                allow_pseudocode_labels
                and old_head.startswith(chr(96) * 3 + "cpp")
                and new_head == old_head.replace(chr(96) * 3 + "cpp",
                                                 chr(96) * 3 + "text", 1)
            )
            matched = (old_hash == new_hash and
                       (old_head == new_head or permitted_label))
        if matched:
            consumed += 1
            continue
        if (new_shell < allowed_new_shell_listings
                and new_head == chr(96) * 3 + "sh"):
            new_shell += 1
            continue
        raise ValueError(
            f"Chapter {chapter}: original fenced-block {consumed + 1} "
            f"altered, reordered or replaced during {stage}; unexpected "
            f"reader header {new_head!r}"
        )
    if consumed != len(before) or new_shell != allowed_new_shell_listings:
        raise ValueError(
            f"Chapter {chapter}: missing original fenced block or "
            f"unaccounted shell conversion during {stage}"
        )


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
    """Apply the *minimal* TeX text-mode escapes needed by source math.

    The canonical source is never edited. Display-math prose may contain a
    literal percent (99%) or a code identifier in \texttt{simd_sum}. In TeX,
    percent starts a comment; underscore inside \texttt is invalid text mode.
    Escaping only the text-mode contents preserves mathematical subscripts
    such as x_{i}, command semantics, and author-owned formulas.
    """
    sanitized = re.sub(r"(?<!\\)%", lambda _: r"\%", body)
    for macro in ("texttt", "text", "textbf", "textit", "textrm", "textsf"):
        # This handles simple text-mode macro arguments. Nested arguments are
        # never silently rewritten: an explicit regression test and the TeX
        # compiler must validate genuinely complex display expressions.
        pattern = re.compile(r"\\" + macro + r"\{([^{}\n]*)\}")
        def escape_text(match: re.Match[str]) -> str:
            text = match.group(1)
            text = re.sub(r"(?<!\\)_", lambda _: r"\_", text)
            text = re.sub(r"(?<!\\)&", lambda _: r"\&", text)
            text = re.sub(r"(?<!\\)#", lambda _: r"\#", text)
            return "\\" + macro + "{" + text + "}"
        sanitized = pattern.sub(escape_text, sanitized)
    return sanitized


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



def check_math_tokens(text: str, equations: list[dict], chapter: int,
                      stage: str) -> None:
    """Verify that editing never loses, repeats or invents a math placeholder.

    This check intentionally does not require source formulas to be valid TeX.
    It proves a one-to-one mapping between all source math displays and the
    exact restoration sites *before* Pandoc consumes the derived Markdown.
    """
    expected = {entry["token"] for entry in equations}
    actual = re.findall(r"ALLMIGHTMATHCH\d+INDEX\d+", text)
    missing = [entry["index"] for entry in equations
               if actual.count(entry["token"]) != 1]
    unexpected = sorted(set(actual) - expected)
    if missing or unexpected or len(actual) != len(equations):
        raise ValueError(
            f"Chapter {chapter}: display math not preserved after {stage}; "
            f"expected {len(equations)} source displays, found "
            f"{len(actual)} markers; missing or duplicate indices "
            f"{missing[:25]}; unexpected markers {unexpected[:8]}"
        )

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
        # Validate the marker before accessing figure layout metadata. A
        # missing/duplicated token is an integrity failure, not a KeyError.
        pattern = r"(?m)^" + re.escape(token) + r"$"
        if len(re.findall(pattern, tex)) != 1:
            raise ValueError(f"Missing/duplicate diagram marker: {token}")
        required = ("part", "image", "caption", "chapter", "index")
        missing = [field for field in required if field not in item]
        if missing:
            raise ValueError(f"Incomplete diagram metadata for {token}: {missing}")
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
        # Do not print internal source labels such as "Figure B1" again
        # after LaTeX has already assigned the public Figure 1.2 counter.
        title = re.sub(r"^Figure\s+(?:B\d+|CPP[- ]\d+)\s*[-:]\s*",
                       "", title)
        latex = (
            "\\par\\begin{center}\n"
            "\\includegraphics[width=.94\\linewidth,height=.34\\textheight,"
            "keepaspectratio]{" + path + "}\n"
            "\\captionof{figure}{" + title + "}"
            "\\label{fig:book01:" + str(item["chapter"]) + ":" +
            str(item["index"]) + "}\n"
            "\\end{center}\\smallskip\n"
        )
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
        args = [binary, "-i", str(src), "-o", str(dst),
                "-s", "3", "-t", "neutral", "-b", "white"]
        # Linux CI may need sandbox arguments for its Chromium installation;
        # macOS uses the Mermaid CLI's standard configuration unchanged.
        puppeteer_config = os.environ.get("ALLMIGHT_MERMAID_PUPPETEER_CONFIG")
        if puppeteer_config:
            cfg = Path(puppeteer_config)
            if not cfg.is_file():
                raise FileNotFoundError("Missing Puppeteer config: " + str(cfg))
            args.extend(["-p", str(cfg)])
        result = subprocess.run(args, capture_output=True, text=True)
        if result.returncode or not dst.is_file() or dst.stat().st_size < 500:
            raise RuntimeError(f"Mermaid failed: chapter {item['chapter']} "
                               f"diagram {item['index']}\n" +
                               (result.stderr or result.stdout)[-4000:])
        item["render_sha256"] = digest(dst.read_bytes())
    print("Verified rendered diagrams:", len(figures))



def reflow_inline_monospace(tex: str) -> str:
    """Allow line breaks within nested Pandoc \texttt{...} snippets.

    Pandoc emits C++ template syntax with nested \textless{} and
    \textgreater{} commands, e.g. \texttt{std::vector\textless{}T...}.
    The earlier one-level regex skipped those snippets entirely, causing
    cropped text in the PDF. This balanced-brace scanner edits only generated
    TeX prose, not verbatim code or the canonical Markdown. Display math is
    restored *after* this pass and has its own source-protected sanitizer.
    """
    opening = re.compile(r"\\texttt\{")
    verbatim = re.compile(r"\\begin\{verbatim\}.*?\\end\{verbatim\}",
                          re.DOTALL)

    def prose(fragment: str) -> str:
        out: list[str] = []
        consumed = 0
        for match in opening.finditer(fragment):
            if match.start() < consumed:
                continue
            cursor = match.end()
            depth = 1
            while cursor < len(fragment) and depth:
                if (fragment[cursor] == "\\"
                        and cursor + 1 < len(fragment)
                        and fragment[cursor + 1] in "{}"):
                    cursor += 2
                    continue
                if fragment[cursor] == "{":
                    depth += 1
                elif fragment[cursor] == "}":
                    depth -= 1
                cursor += 1
            if depth != 0:
                raise ValueError("Unclosed generated inline \\texttt argument")
            raw = fragment[match.end():cursor - 1]
            fixed = re.sub(r"(?<!\\)_", lambda _: r"\_", raw)
            if len(raw) >= 15:
                fixed = re.sub(
                    r"(?<=[a-z])(?=[A-Z])|(?<=[A-Z])(?=[A-Z][a-z])",
                    lambda _: r"\allowbreak{}", fixed,
                )
            if len(raw) >= 24:
                fixed = re.sub(
                    r"(\\textless\{\}|\\textgreater\{\}|\\_|::|[/.,;=\-])"
                    r"(?!\\allowbreak\{\})",
                    lambda found: found.group(1) + r"\allowbreak{}",
                    fixed,
                )
            out.append(fragment[consumed:match.start()])
            out.append(r"\texttt{" + fixed + "}")
            consumed = cursor
        out.append(fragment[consumed:])
        return "".join(out)

    result: list[str] = []
    cursor = 0
    for match in verbatim.finditer(tex):
        result.append(prose(tex[cursor:match.start()]))
        result.append(match.group(0))
        cursor = match.end()
    result.append(prose(tex[cursor:]))
    return "".join(result)


def reflow_long_cpp_verbatim(tex: str, width: int = 64) -> str:
    """Wrap long *displayed* C++ lines at token-safe boundaries.

    Pandoc's verbatim blocks have no semantic line-break policy. LaTeX
    listings otherwise splits long expressions in the middle of brackets,
    identifiers or a final semicolon. This changes only generated TeX:
    source Markdown and all checksum/provenance comparisons remain intact.
    Other languages, preprocessor directives, and ambiguous raw strings or
    comments are left for the standard listings fallback.
    """
    blocks = re.compile(r"(?s)(\\begin\{verbatim\}\n)(.*?)(\n\\end\{verbatim\})")

    def wrap(line: str) -> list[str]:
        if (len(line.expandtabs(4)) <= width
                or line.lstrip().startswith(("#", "//"))
                or "//" in line or 'R"' in line
                or line.rstrip().endswith("\\") or "\t" in line):
            return [line]
        baseindent = len(line) - len(line.lstrip())
        continuation = " " * min(baseindent + 4, 20)
        out: list[str] = []
        current = line
        for _ in range(5):
            if len(current) <= width:
                break
            candidates: list[tuple[int, int]] = []
            quote: str | None = None
            escape = False
            for i, character in enumerate(current):
                if quote is not None:
                    if escape:
                        escape = False
                    elif character == "\\":
                        escape = True
                    elif character == quote:
                        quote = None
                    continue
                if character in ('"', "'"):
                    quote = character
                    continue
                if i < baseindent + 10 or i > width - 3:
                    continue
                if character in ("(", ","):
                    candidates.append((i + 1, 2))
                elif character == " " and i + 1 < len(current):
                    candidates.append((i + 1, 1))
            if not candidates:
                break
            # Later legal breaks are preferable to token-breaking at the
            # margin. A small bonus favours punctuation over bare spaces.
            cut = max(candidates, key=lambda item: item[0] + 2 * item[1])[0]
            head, tail = current[:cut].rstrip(), current[cut:].lstrip()
            if not head or not tail:
                break
            out.append(head)
            current = continuation + tail
        out.append(current)
        return out

    def fix(match: re.Match[str]) -> str:
        code = match.group(2)
        if not re.search(
            r"(#include\s*<|std::|\b(?:struct|class|template)\s+[A-Za-z_])",
            code,
        ):
            return match.group(0)
        expanded = [part for line in code.split("\n") for part in wrap(line)]
        return match.group(1) + "\n".join(expanded) + match.group(3)

    return blocks.sub(fix, tex)


def reflow_long_textual_displays(tex: str) -> str:
    """Avoid clipping long prose and pipeline math in generated TeX only.

    The manuscript's exact math-body and boxed-equation checks already ran
    before this function. No source Markdown is changed. We alter only
    visual presentation of long labeled chains, additive plans, enumerated
    properties, or prose-only boxes. Genuine short formulae are unchanged.
    """
    display = re.compile(r"\\\[\s*(.*?)\s*\\\]", re.DOTALL)

    def top_level_parts(text: str, delimiter: str) -> list[str]:
        """Split only between math terms, never inside TeX curly braces."""
        parts: list[str] = []
        depth = 0
        beginning = 0
        pos = 0
        while pos < len(text):
            if text[pos] == "{":
                depth += 1
            elif text[pos] == "}":
                depth -= 1
            if depth == 0 and text.startswith(delimiter, pos):
                parts.append(text[beginning:pos].strip())
                pos += len(delimiter)
                beginning = pos
                continue
            pos += 1
        parts.append(text[beginning:].strip())
        return parts

    def wrap_terms(parts: list[str], operator: str, max_chars: int) -> str | None:
        rows: list[list[str]] = []
        row: list[str] = []
        row_length = 0
        for term in parts:
            width = len(term) + 4
            if row and row_length + width > max_chars:
                rows.append(row)
                row = []
                row_length = 0
            row.append(term)
            row_length += width
        if row:
            rows.append(row)
        if len(rows) < 2:
            return None
        written: list[str] = []
        for number, terms in enumerate(rows):
            body = operator.join(terms)
            if number and operator != ", ":
                body = operator.strip() + " " + body
            if operator == ", " and number < len(rows) - 1:
                body += ","
            written.append("& " + body)
        return "\\begin{aligned}\n" + (" \\\\\n").join(written) + "\n\\end{aligned}"

    def correct(match: re.Match[str]) -> str:
        raw = match.group(1).strip()
        if r"\begin{aligned}" in raw:
            return match.group(0)
        body = " ".join(raw.split())
        boxed = body.startswith(r"\boxed{") and body.endswith("}")
        inner = body[len(r"\boxed{"):-1].strip() if boxed else body

        # \boxed{\text{an entire English sentence or engineering claim}}
        # is a non-breaking mathematical box, not a readable proposition.
        if boxed and len(inner) > 75:
            visible = re.sub(
                r"\\text\{([^{}]*)\}", lambda found: found.group(1), inner
            )
            if "\\" not in visible and len(visible) >= 65:
                return (
                    "\\begin{center}\n"
                    "\\fbox{\\parbox{0.82\\linewidth}{\\centering\\small\\itshape "
                    + visible + "}}\n\\end{center}"
                )

        if inner.count(r"\rightarrow") >= 2 and len(inner) > 76:
            parts = top_level_parts(inner, r"\rightarrow")
            if len(parts) >= 3:
                arranged = wrap_terms(parts, r"\rightarrow ", 62)
                if arranged:
                    if boxed:
                        arranged = r"\boxed{" + arranged + "}"
                    return "\\[\n" + arranged + "\n\\]"

        if inner.count(r"\land") >= 2 and len(inner) > 85:
            parts = top_level_parts(inner, r"\land")
            if len(parts) >= 3:
                arranged = wrap_terms(parts, r"\land ", 54)
                if arranged:
                    return "\\[\n" + arranged + "\n\\]"

        if inner.count(" + ") >= 3 and len(inner) > 80:
            parts = top_level_parts(inner, " + ")
            if len(parts) >= 4:
                arranged = wrap_terms(parts, " + ", 78)
                if arranged:
                    if boxed:
                        arranged = r"\boxed{" + arranged + "}"
                    return "\\[\n" + arranged + "\n\\]"

        if not boxed and inner.count(",") >= 5 and len(inner) >= 80:
            # A bare list of sampler options is not a scalar equation.
            if "\\" not in inner:
                parts = top_level_parts(inner, ",")
                arranged = wrap_terms(parts, ", ", 57)
                if arranged:
                    return "\\[\n" + arranged + "\n\\]"

        return match.group(0)

    return display.sub(correct, tex)


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
