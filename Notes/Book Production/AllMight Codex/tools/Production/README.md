# Production tooling and provenance

This package contains reusable production contracts and records for the AllMight Codex. The authoritative production source remains the six-volume tree at this folder's parent. The original continuous Master sources remain the editorial sources.

## Stable tooling

- `../../tools/render_assets.cjs` renders MathJax and Mermaid assets. It loads Playwright through standard Node module resolution, or from the module specifier in `PLAYWRIGHT_MODULE`. Install or configure the production runtime outside this package; no developer-specific runtime path is embedded here.
- `../../tools/compose_fcbea_panels.py` composes the approved two-panel fcbea diagram.
- `../../tools/run_composite_jobs.py` runs the versioned post-render composite stage from the production root using `composite-jobs.json`.
- `../source-promotion/` contains the accepted source inventory, source-only resource audit, merged print-map summary, and live asset/font provenance.
- `provenance/computed-special-svg-fonts.json` contains durable font measurement evidence referenced by the font metadata map.

## Source promotion checkpoint

The source-only promotion passed hash-guarded apply and destination verification. The six volume AST, Appendix, XML, and XHTML trees plus additive assets and three generated root manifests were promoted. Markdown sources were hash-verified unchanged; no files were deleted. `../source-promotion/source-promotion-verification.json` records the accepted live manifest hashes and media audit. The source inventory records 129 chapters and 439 unique companion sources relative to the Master vault.

The merged font map is `svg-base-font-metadata.json`. It covers 25,735 MathJax SVGs and 95 Mermaid SVGs. Every SVG SHA matches its print-map input hash. The durable print map contains the same 25,832 production rows as the accepted source map; only one cache-only nonproduction fixture exclusion record is represented as a sanitized summary. Six EPUB outputs and native InDesign deliverables have separate validation gates.

Recheck the post-render assets from the production root with the packaged verifier:

```sh
python3 Tools/Production/verify_postrender_assets.py --production-root . --print-assets Tools/Production/merged-print-asset-manifest.json --font-metadata Tools/Production/svg-base-font-metadata.json --report QA/source-promotion/postpromotion-postrender-verification.json
```

## Production controller recovery

- `production_controller.py` is the restored, tested checkpoint controller source. `provenance/production-controller-recovery.json` records its source hash, recovery evidence, and fixture results. The package keeps the Python source only; it has no bytecode or private-cache runtime dependency.

## Reproduction and recovery

Run from the production root with Node, Chrome, and Playwright available to the process. Render canonical assets using the checked-in `jobs.json`; then run the versioned post-render composite stage before font measurement, print-PDF transcode, native import, or EPUB validation:

```sh
node tools/render_assets.cjs jobs.json
python3 tools/run_composite_jobs.py composite-jobs.json --job-id fcbea06ba67e4dcc48784ddf --publish
```

Keep `sharedAssets/`, `Design/Diagram Sources/`, `composite-jobs.json`, and `composite-render-report.json` together. The original, panel, and recovery SVGs are packaged under `Design/Diagram Sources/`; their hashes are recorded in `composite-jobs.json`. Re-run the asset hash and source inventory checks before restoring from an older snapshot. Do not use a private staging cache as a runtime dependency.

The import builder and native scripts are maintained in their respective active work streams; this package does not replace them.
