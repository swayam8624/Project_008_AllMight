# Held diagram panel review

Review evidence for 11 source graphs and 29 panels is retained here. The canonical panel PDFs are copied to `Assets/Diagrams/Print-v1/`, and the active override manifest points to their ordered panel arrays. `Tools/convert.py` and `Tools/diagram-panels.lua` emit the panels with a single source alt label and Panel n/N continuation labels. The active full-book build has not yet been rerun against this integration.

- `panel-provenance.json` records the source graph hash, exact Mermaid source, exact source lines assigned to each panel, duplicate sequence control lines, cross-panel aliases/relations, display-only line breaks, and review asset paths.
- `render-report.json` records Mermaid renderer status and SVG hashes.
- `panel-print-projection.json` estimates minimum label point size under the existing `BookDiagramPanel` dimensions. It is a geometric projection; actual LaTeX page proofs still need review before acceptance.
- `contact-sheet.png` provides the visual overview; every panel also has individual PDF, SVG, and PNG outputs. `sample/held-panel-print-proof.tex` compiles all panels through the production `BookDiagramPanel` macro; see `panel-review-result.json` for the compiled proof hash and visual review record.

Flowcharts with long left-to-right source paths are shown top-to-bottom in their panels. Original node/edge declaration text and IDs remain in the panels. SVG roots set `xml:space="preserve"` so SVG/PDF conversion retains spaces between adjacent text spans. The state diagram review copies use a 12 px label CSS size to keep the small transition labels legible.
