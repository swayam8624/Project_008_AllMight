#!/usr/bin/env python3
"""Verify source fidelity, SVG text, PDF bounds, and hashes for held panel candidates."""
from __future__ import annotations
import collections
import hashlib
import json
import re
import subprocess
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BASE = ROOT / "Assets" / "Diagrams" / "Panels-Review-next"

def sha(data: bytes | str) -> str:
    if isinstance(data, str):
        data = data.encode("utf-8")
    return hashlib.sha256(data).hexdigest()

def cls(node: ET.Element) -> str:
    return node.attrib.get("class", "")

def visible_rows(node: ET.Element) -> list[str]:
    rows = [item for item in node.iter()
            if item.tag.endswith("tspan") and "text-outer-tspan" in cls(item)]
    if rows:
        return ["".join(item.itertext()) for item in rows]
    return ["".join(node.itertext())]

def visible_text(node: ET.Element) -> str:
    return re.sub(r"\s+", " ", " ".join(visible_rows(node))).strip()

def ancestors(node: ET.Element, parents: dict[ET.Element, ET.Element]):
    while node in parents:
        node = parents[node]
        yield node

def assert_equal(actual, expected, message: str) -> None:
    if actual != expected:
        raise AssertionError(f"{message}: expected {expected!r}; got {actual!r}")

def audit_source_partition(graph: dict) -> None:
    lines = graph["source_code_exact"].splitlines()
    indices = [index for panel in graph["panels"]
               for index in panel["source_line_indices_zero_based"]]
    repeated = [index for panel in graph["panels"]
                for index in panel["repeated_participant_line_indices_zero_based"]]
    expected = set(range(1, len(lines))) - set(repeated)
    assert_equal(set(indices), expected, f"{graph['source_asset_id']} source line coverage")
    counts = collections.Counter(indices)
    allowed = {item["zero_based_line_index"]
               for item in graph["source_control_line_duplicates"]}
    for index, count in counts.items():
        wanted = 3 if index in allowed else 1
        assert_equal(count, wanted,
                     f"{graph['source_asset_id']} source line {index} multiplicity")
    for panel in graph["panels"]:
        assert_equal([lines[i] for i in panel["source_line_indices_zero_based"]],
                     panel["source_lines_exact"],
                     f"{panel['variant_id']} exact source lines")
        repeated_indices = panel["repeated_participant_line_indices_zero_based"]
        assert_equal([lines[i] for i in repeated_indices],
                     panel["repeated_participant_lines_exact"],
                     f"{panel['variant_id']} repeated participant lines")

def audit_render_job(graph: dict, panel: dict, job: dict) -> None:
    code = job["code"]
    assert_equal(sha(code), panel["rendered_code_sha256"],
                 f"{panel['variant_id']} authored renderer job hash")
    body = [line for line in code.splitlines()
            if not line.startswith("%%{init:")]
    rendered_header = body.pop(0).strip()
    source_header = graph["source_kind"]
    if source_header.startswith(("flowchart LR", "graph LR")):
        assert_equal(rendered_header, "flowchart TB",
                     f"{panel['variant_id']} approved flowchart orientation")
    else:
        assert_equal(rendered_header, source_header,
                     f"{panel['variant_id']} graph kind")
    expected = panel["repeated_participant_lines_exact"] + panel["source_lines_exact"]
    normalize = lambda value: re.sub(r"<br\s*/?>", " ", value)
    assert_equal(list(map(normalize, body)), list(map(normalize, expected)),
                 f"{panel['variant_id']} rendered source body")

def audit_visible_labels(graph: dict) -> None:
    kind = graph["source_kind"]
    if kind.startswith(("flowchart", "graph")):
        expected_nodes, expected_edges = [], []
        for line in graph["source_code_exact"].splitlines()[1:]:
            for match in re.finditer(r"\b([A-Za-z][\w]*)\s*(\[([^\]]*)\]|\{([^}]*)\})", line):
                label = (match.group(3) if match.group(3) is not None
                         else match.group(4)).strip()
                if len(label) >= 2 and label[0] in "\"'" and label[-1] == label[0]:
                    label = label[1:-1]
                expected_nodes.append(re.sub(r"\s+", " ", label).strip())
            expected_edges += [match.group(1).strip() for match in
                               re.finditer(r"--\|([^|]+)\|", line)]
            expected_edges += [match.group(1).strip() for match in
                               re.finditer(r"--\s*[\"']([^\"']+)[\"']\s*-->", line)]
        actual_nodes, actual_edges = [], []
        for panel in graph["panels"]:
            root = ET.parse(panel["svg"]).getroot()
            parents = {child: node for node in root.iter() for child in node}
            for node in root.iter():
                if not node.tag.endswith("text"):
                    continue
                visible = visible_text(node)
                path = list(ancestors(node, parents))
                if any("node" in cls(item).split() and "-flowchart-" in item.attrib.get("id", "")
                       for item in path):
                    actual_nodes.append(visible)
                elif any("edgeLabel" in cls(item) for item in path) and visible:
                    actual_edges.append(visible)
        actual_node_counts = collections.Counter(actual_nodes)
        actual_edge_counts = collections.Counter(actual_edges)
        for label, count in collections.Counter(expected_nodes).items():
            if actual_node_counts[label] < count:
                raise AssertionError(f"{graph['source_asset_id']} missing or altered node label {label!r}")
        for label, count in collections.Counter(expected_edges).items():
            if actual_edge_counts[label] < count:
                raise AssertionError(f"{graph['source_asset_id']} missing or altered edge label {label!r}")
    elif kind == "sequenceDiagram":
        expected_actors, expected_messages, expected_notes, expected_controls = [], [], [], []
        for panel in graph["panels"]:
            for line in panel["repeated_participant_lines_exact"]:
                match = re.match(r"\s*participant\s+\w+(?:\s+as\s+(.*))?$", line)
                label = match.group(1) or re.search(r"participant\s+(\w+)", line).group(1)
                expected_actors.extend([label.strip()] * 2)
            for line in panel["source_lines_exact"]:
                match = re.match(r"\s*Note.*?:\s*(.*)$", line, re.I)
                if match:
                    expected_notes.append(match.group(1).strip())
                    continue
                match = re.match(r"\s*(?:alt|else)\s+(.*)$", line)
                if match:
                    expected_controls.append(match.group(1).strip())
                    continue
                match = re.match(r"\s*[A-Za-z]\w*(?:-->>|-->|->>|->)[A-Za-z]\w*:\s*(.*)$", line)
                if match:
                    expected_messages.append(match.group(1).strip())
        actual_actors, actual_messages, actual_notes, actual_controls = [], [], [], []
        for panel in graph["panels"]:
            root = ET.parse(panel["svg"]).getroot()
            for node in root.iter():
                if not node.tag.endswith("text"):
                    continue
                visible = visible_text(node)
                style = cls(node)
                if "actor-box" in style:
                    actual_actors.append(visible)
                elif "messageText" in style:
                    actual_messages.append(visible)
                elif "noteText" in style:
                    actual_notes.append(visible)
                elif "loopText" in style or "sectionTitle" in style:
                    actual_controls.append(visible)
        assert_equal(collections.Counter(actual_actors), collections.Counter(expected_actors),
                     f"{graph['source_asset_id']} participant labels")
        for label, expected, actual in (
            ("messages", expected_messages, actual_messages),
            ("notes", expected_notes, actual_notes),
        ):
            assert_equal(" ".join(expected), " ".join(actual),
                         f"{graph['source_asset_id']} {label}")
        strip_brackets = lambda items: re.sub(r"\s+", " ",
                                              " ".join(items).replace("[", "").replace("]", "")).strip()
        assert_equal(strip_brackets(expected_controls), strip_brackets(actual_controls),
                     f"{graph['source_asset_id']} alternative/control labels")
    elif kind.startswith("stateDiagram"):
        for panel in graph["panels"]:
            expected_states, expected_transitions = set(), []
            for line in panel["source_lines_exact"]:
                match = re.match(r"\s*(.*?)\s*-->\s*(.*?)(?::\s*(.*))?$", line)
                if not match:
                    continue
                for state in (match.group(1).strip(), match.group(2).strip()):
                    if state != "[*]":
                        expected_states.add(state)
                if match.group(3):
                    expected_transitions.append(match.group(3).strip())
            root = ET.parse(panel["svg"]).getroot()
            actual_states = []
            for node in root.iter():
                if node.tag.endswith("g") and "statediagram-state" in cls(node):
                    label = next((item for item in node.iter() if item.tag.endswith("text")), None)
                    if label is not None:
                        actual_states.append(visible_text(label))
            assert_equal(collections.Counter(actual_states),
                         collections.Counter(expected_states),
                         f"{panel['variant_id']} state labels")
            visible = [visible_text(node) for node in root.iter()
                       if node.tag.endswith("text")]
            for label in expected_transitions:
                if label not in visible:
                    raise AssertionError(f"{panel['variant_id']} missing transition label {label!r}")

def audit_pdf_and_projection(panel: dict, metric: dict) -> None:
    svg = Path(panel["svg"]); pdf = Path(panel["pdf"])
    if not svg.is_file() or not pdf.is_file():
        raise AssertionError(f"missing review asset for {panel['variant_id']}")
    svg_bytes, pdf_bytes = svg.read_bytes(), pdf.read_bytes()
    assert_equal(sha(svg_bytes), panel["svg_sha256"], f"{panel['variant_id']} SVG hash")
    assert_equal(sha(pdf_bytes), panel["pdf_sha256"], f"{panel['variant_id']} PDF hash")
    assert_equal(sha(svg_bytes), metric["svg_sha256"], f"{panel['variant_id']} projection SVG hash")
    assert_equal(sha(pdf_bytes), metric["pdf_sha256"], f"{panel['variant_id']} projection PDF hash")
    root = re.search(r"<svg\b[^>]*>", svg_bytes.decode("utf-8")).group()
    width = float(re.search(r'\bwidth="([0-9.]+)px"', root).group(1))
    height = float(re.search(r'\bheight="([0-9.]+)px"', root).group(1))
    info = subprocess.check_output(["pdfinfo", str(pdf)], text=True)
    dims = tuple(map(float, re.search(r"Page size:\s+([\d.]+) x ([\d.]+) pts", info).groups()))
    assert_equal(round(dims[0], 2), round(width * 0.75, 2), f"{panel['variant_id']} PDF width")
    assert_equal(round(dims[1], 2), round(height * 0.75, 2), f"{panel['variant_id']} PDF height")
    font_sizes = [float(value) for value in
                  re.findall(r"font-size:\s*([0-9.]+)px", svg_bytes.decode("utf-8"))]
    font_sizes += [float(value) for value in
                   re.findall(r'font-size="([0-9.]+)px"', svg_bytes.decode("utf-8"))]
    if not font_sizes:
        raise AssertionError(f"{panel['variant_id']} SVG has no measurable font size")
    scale = min(1.0, 438.88 / dims[0], 341.74 / dims[1])
    projected = min(font_sizes) * 0.75 * scale
    if projected < 8:
        raise AssertionError(f"{panel['variant_id']} projects below 8 pt: {projected:.2f} pt")
    if dims[0] > 438.88 or dims[1] > 341.74:
        raise AssertionError(f"{panel['variant_id']} exceeds BookDiagramPanel bounds")

def main() -> int:
    provenance = json.loads((BASE / "panel-provenance.json").read_text())
    jobs_data = json.loads((BASE / "jobs.json").read_text())
    jobs = {item["id"]: item for item in jobs_data["assets"]}
    projection_data = json.loads((BASE / "panel-print-projection.json").read_text())
    metrics = {item["id"]: item for item in projection_data["metrics"]}
    graphs = provenance["source_panels"]
    if len(graphs) != 11 or sum(item["panel_count"] for item in graphs) != 29:
        raise AssertionError("expected exactly 11 held source graphs and 29 panels")
    for graph in graphs:
        assert_equal(sha(graph["source_code_exact"]), graph["source_graph_sha256"],
                     f"{graph['source_asset_id']} source graph hash")
        assert_equal(graph["source_graph_sha256"], graph["expected_source_graph_sha256"],
                     f"{graph['source_asset_id']} source manifest hash")
        audit_source_partition(graph)
        audit_visible_labels(graph)
        for panel in graph["panels"]:
            audit_render_job(graph, panel, jobs[panel["variant_id"]])
            audit_pdf_and_projection(panel, metrics[panel["variant_id"]])
    print("PASS: 11 source graphs, 29 panels; exact source partitions and visible labels; SVG/PDF hashes, geometry, and >=8 pt print projection")
    return 0

if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)
