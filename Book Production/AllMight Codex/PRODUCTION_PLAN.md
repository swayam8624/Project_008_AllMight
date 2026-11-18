# The AllMight Codex — staged production plan

This plan produces three books in two volumes each from the current manuscripts. It preserves the masters, all original teaching, and recoverable earlier production artifacts. Existing source-vault changes are outside this task. No commit, push, or publication is authorized.

## Current decision

Build an original vintage technical-book interior inspired by the reference direction, using original typography, rules, and vector ornaments. Do not acquire or reproduce the Adobe Stock template. The previous novel-derived template is a historical prototype, not the final design. The active vault is the Obsidian iCloud container, not the retired CloudDocs location.

## Why another layout pass is required

Earlier import code used uniform graphic fitting and equal table-column widths. Those rules can prevent gross overflow while leaving poor orientation, crowded inline formulas, badly allocated columns, weak captions, and inappropriate page breaks. Valid XML, an existing PDF, and zero overset are separate from a readable finished page. The revised pipeline must prove both structural preservation and actual layout.

## Work sequence and gates

1. **Inventory and freeze.** Fingerprint the three masters, chapter edition, generated ASTs, all required assets, design profile, and generators. Detect stale absolute paths. Keep the 129 chapters assigned once across the six volumes. Never regenerate from retired manifests.
2. **Resolve references.** Map original-master topic links into real chapter anchors; disambiguate repeated headings using source identity and location. Add missing anchors only at the exact retained teaching passage. Keep private artifact references visible but distinguish them from reader navigation. A successful stage has zero unresolved publication links; warning exclusions remain listed.
3. **Design the original interior.** Start with a 7 × 10 inch technical trim, mirrored margins, a readable serif body, clear hierarchy, modest chapter ornament, running headers, monospaced code, restrained table rules, and numbered/captioned figures where identifiers can be stable. Build reusable native styles and parent pages without modifying the earlier template.
4. **Create a difficult sample.** Use actual material from all three books: prose, inline fractions/subscripts, display matrices and derivations, narrow and wide tables, code-heavy examples, horizontal flows, tall flows, and sequence diagrams. Record source IDs for every sample. Render and inspect the resulting pages before bulk composition.
5. **Compose with checkpoints.** A Python controller invokes one controlled InDesign queue. Separate input preparation, native import, linking, export, and audits. Reuse a chapter only when source, design, generator, and placed-asset fingerprints match. Retry failed stages and chapters only. Parallel workers may parse/render/check independent inputs; they do not drive the same native application concurrently.
6. **Export both reading models.** Produce native INDD/IDML chapter documents and six INDB/PDF volumes. Generate six semantic reflowable EPUBs and a collection EPUB for direct cross-volume topic navigation. Preserve code as text and equations as MathML in EPUB, with vector print equations. Individual-volume cross-book entries identify the destination volume/topic instead of pretending every reader can open a sibling EPUB.
7. **Audit and repair exceptions.** Validate source preservation, code whitespace, rich table cells, anchors, resources, fonts, image geometry, native overflow, minimum effective base type size, and EPUB conformance. Review rendered samples from each layout class and every flagged page. Do not declare final books complete while any required gate remains open.

## Layout acceptance rules

| Element | Required behavior | Failure evidence |
|---|---|---|
| Inline equation | Align using its actual baseline; keep surrounding spaces; provide enough line height without making every line in the paragraph oversized | Collision, changed word boundary, baseline error, clipped fraction/subscript |
| Display equation | Own paragraph, consistent spacing, centered or aligned derivation as appropriate; exact tokens and operators retained | Run-on prose, clipped matrix, unreadable scale, changed expression |
| Diagram | Choose direction by meaning and page geometry; preserve every node, relationship, label, and order. Use panels or a dedicated figure page when needed | Blanket direction change, tiny labels, detached explanation, broken flow |
| Diagram orientation | Prefer horizontal only when readable. Consider vertical, panels, dedicated landscape page, or spread deliberately | Rotated body text, shrinking instead of a layout decision, missing panel continuity |
| Table | Allocate columns from contents; preserve header associations and rich cells; repeat headers on continued pages; manage row breaks | Equal widths causing squeeze, overset cells, cut rows, equation outside a cell |
| Wide table | Prefer a deliberate full-width/landscape treatment or labelled row layout that retains relationships; preserve semantic tables in EPUB | Blindly flattened cells, lost headers, tiny type, silent omission |
| Caption and teaching | Keep figure, caption, and explanation connected; preserve stable anchors/backlinks | Caption stranded on another page, meaningless alt text, wrong reference target |
| Code | Preserve spaces, tabs, identifiers, comments and line endings semantically; keep selectable text and readable wrapping | Lost indentation, missing blocks, clipped long lines, unexplained fragments |

Target effective base text in figures is at least 7.5 pt at final print size; superscripts and mathematical script sizes are evaluated as part of normal notation, not falsely treated as base-label failures. Normal body and code sizes are higher. File geometry checks are followed by rendered-page review.

## Supervision and ownership

The smallest agents audit references, layout cases, and build/EPUB contracts in bounded tasks. The supervisor reviews their evidence, owns the native application queue and design integration, and resolves cross-stage decisions. Each stage records commands, actual outcomes, and unresolved cases. An agent completion message alone is not approval evidence.

## First concrete milestone

An original native template plus a rendered representative technical sample, accompanied by the source/asset inventory and an explicit defect report. The full six-volume build follows only after this internal acceptance gate passes; it does not require a new permission request.
