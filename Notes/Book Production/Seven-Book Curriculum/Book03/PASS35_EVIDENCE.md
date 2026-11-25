# Pass 35 — Graphics, vision and XR reconciliation evidence

## Provenance

Pinned source revision 0afe1daf1efb5b1952b78a6476b3a37edde912cb. PASS35_GRAPHICS_SOURCE_MAP.md records all fourteen derived Graphics Vision and XR sources named by the original ledger: 28,118 routed body lines, 502 mechanically extracted heading entries and 1,421,821 retrieved characters. With P34, the two maps cover 21 derived files, 47,046 routed lines and 1,129 heading entries. The headings may include fenced example headings; this is an inventory, not a count of unique concepts. Source destinations are provisional, and master-span hashes are source-declared, not recomputed from the continuous masters.

## Actual reader additions

| Reader | Selected source passages | New material |
|---|---|---|
| 06 | Graphics Vision and XR/24, 14–462 | Möller–Trumbore derivation, exact hit and finite triangle miss, front/back policy, non-unit direction, numerical and shared-edge limitations |
| 07 | /41, 1424–1505; /30, 108–179 and 214–295 | Cotangent weights and stiffness/mass signs, square diagonal witness, explicit diffusion stability and boundary interpretation |
| 20 | /19, 14–330; /04, 76–114 | Bilinear interpolation, negative indices, repeat/clamp/mirror, seam example, footprint singular values |
| 29 | /32, 68–110 | Centered rigid alignment, SVD proper rotation, noncollinear support, reflection rejection, symmetric vs one-sided scale objectives |

The selected passages were inspected in detail. Original files and earlier chapter content are retained; two obsolete sentences in Chapter 06 were updated to reflect the new derivation.

## Independent numerical checks

Run from the lab directory with Python and NumPy installed:

~~~sh
python3 scripts/verify_source_reconciliation.py
~~~

Exact tested and committed blob 53fe0c39efee77fe0e48e92cd1abb56ce0dbc08a. Observed local execution:
- 500 seeded ray/triangle linear systems with condition number below 100, compared against NumPy direct solve: maximum absolute component difference 1.7053025658242404e-13. Explicit inside, outside-supporting-plane-hit and parallel fixtures.
- Square cotangent stiffness: zero diagonal-edge weight, boundary edge weights 0.5, constant null vector. Mass-scaled eigenvalues approximately 0,3,6,9. Forward Euler stable step limit 2/9 = 0.2222222222 for unit diffusivity; 1.01 times the limit has amplification greater than one.
- Texture seam sample [0.5,0,0] for repeat, clamp and mirrored addressing at u=0. 2,187 address-bound checks (9 dimensions, 81 indices, 3 modes).
- Exact planar quarter-turn rigid fit and translation; noncoplanar reflected asymmetric cloud gives proper-rotation RMSE 0.671302390501482 coordinate units and determinant +1 within tolerance.

These are separate Python/NumPy numerical witnesses, not new production C++23 implementations or registered CTests. The original native labs are unchanged, with the 34-test and workshop acceptance recorded at the exact earlier CI revision in PASS34_EVIDENCE.md. The new script requires NumPy and is not yet part of the cumulative CI workflow. No new PDF or EPUB, no claim of complete math, source or visual audit. Full comparison with the continuous masters, source asset and code custody, remaining specialized passages and final editorial acceptance remain open.
