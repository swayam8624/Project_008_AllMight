---
tags: [visual-recall, phase-7, matrices, transforms, quaternions]
coverage: "Received 601–634 and 651–700; 635–650 source gap"
---

# Matrices, Transforms and Quaternions — Visual Recall

Companion to [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Keep the promises visible while moving between representations|the Phase 7 teaching story]]. Start with the short route, reconstruct a worked figure, then follow its link when an assumption or definition is missing. Six excerpts supply 84 numbered lessons; the affine bridge is reconstruction, and Parts 635–650 remain source gaps.

## The short route

```mermaid
%%{init: {'theme':'base','themeVariables':{'primaryColor':'#e4f1f6','primaryTextColor':'#173746','lineColor':'#527589'}}}%%
flowchart LR
 A["Basis images<br/>define the map"] --> B["Compose and invert<br/>track information"]
 B --> C["Add origins<br/>name coordinate frames"]
 C --> D["Store and execute<br/>declare layout"]
 D --> E["Compose orientations<br/>unit quaternions"]
 E --> F["Animate and cache<br/>respect representation limits"]
 style F fill:#fff1d5,stroke:#bc892a
```

**Convention card:** active right-handed rotations; column vectors; $y=Mx$; rightmost operation first; code angles in radians; Hamilton quaternion components $(w,x,y,z)$. Row-major storage does not choose any of those mathematical conventions.

| Topic | Shortest useful rule | Figure or explanation |
|---|---|---|
| Meaning | A rectangular table becomes a map only after input/output bases and roles are fixed. Columns are input-basis images. | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Figure M1 - Coordinates weight the new basis directions\|M1: weighted columns]] |
| Shape | $(m\times k)(k\times n)$ yields $m\times n$; shared dimensions must match. Addition needs equal shape. | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Combining rules explains the multiplication law\|Derive the product]] |
| Composition | $ABx$ applies $B$ first. Noncommutative order is separate from associative grouping; floating-point regrouping can change bits. | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Figure M2 - The same operations give different results in different orders\|M2: two orders]] |
| Rotations | Derive columns from rotated axes. For $R_y$, rotate the oriented pair $(z,x)$, then restore $(x,y,z)$ order. | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#The oriented plane explains the Y-rotation signs\|Axis derivation and values]] |
| Structure | $(AB)^T=B^TA^T$; $A=(A+A^T)/2+(A-A^T)/2$. Trace is similarity-invariant, not a full map identifier. | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Transpose lets us compare measurements and structure\|Transpose, symmetry and trace]] |
| Determinant | Magnitude = volume scale; sign = orientation; zero = rank loss for square maps. It does not measure conditioning alone. | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Figure M3 - Three different questions answered by one determinant\|M3: determinant questions]] |
| Inversion | Cofactors explain a formula; pivoted elimination explains an algorithm. Solve directly when the inverse itself is unnecessary. | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Row reduction makes the inverse observable\|Full elimination trace]] |
| Sensitivity | $\kappa(A)=\|A\|\|A^{-1}\|$ in a compatible norm. Stable code cannot restore information lost to input uncertainty. | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Figure M4 - Uncertainty can be amplified by a reversible map\|M4: perturbed observation]] |
| Point/vector | Affine points receive $Ap+t$; displacements receive $Av$. Homogeneous $w=1$ and $w=0$ expose the distinction. | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Figure M5 - The same frame moves a point and redirects a displacement\|M5: two roles]] |
| Frames | $F_C^{-1}F_B$ converts B-coordinates to C-coordinates. The camera view is the inverse camera pose. | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Coordinates can change while the geometric object stays still\|Active/passive and frame conversion]] |
| Projection | Clip retains $w$; NDC divides by valid $w$. Clipping, depth range and backend conventions remain explicit. | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Figure M6 - Follow one point through named spaces\|M6: complete point walk]] |
| Normals | $n'=A^{-T}n$ preserves zero action on transformed tangents. Normalize only after a valid transformation. | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#A normal transforms as a measurement that must still vanish on tangents\|Proof and counterexample]] |
| Layout | Logical width is not physical stride. Real arrays provide array bounds; views borrow both layout and lifetime. | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Figure M7 - A submatrix keeps the parent's physical stride\|M7: subview addresses]] |
| Performance | SIMD, loop order and tiling change reuse and rounding. Measure hardware/workload; algebra alone supplies no speedup. | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#SIMD and loop order change the execution schedule, not the matrix question\|Reuse and loop trace]] |
| Quaternion | Hamilton product = scalar/dot part + weighted vectors/cross part. Unit $q$ permits inverse-by-conjugate. | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Figure M8 - One orientation, several checkable actions\|M8: axis to action]] |
| Half-angle | $q=(\cos(\theta/2),a\sin(\theta/2))$ for unit axis. Two-sided action produces the full angle; $q$ and $-q$ share orientation. | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Derive the axis-angle mapping by deriving its action\|Sandwich derivation]] |
| Conversion | Quaternion↔matrix tests compare basis actions; near $180^\circ$ recover a large component. Reject scale/shear/reflection as rotation inputs. | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Recover a large component instead of dividing by a nearly-zero one\|Stable branches]] |
| Interpolation | Sign-align endpoints. NLERP normalizes the chord; SLERP advances on the great circle. Small-angle fallback needs a policy. | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Figure M9 - Choose a representative before choosing a path\|M9: path selection]] |
| Euler | Axis sequence is part of the type contract. Gimbal lock is a singular coordinate chart; original angle history cannot be recovered. | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Euler coordinates remain useful when their chart is explicit\|Substitute the singular pitch]] |
| Rigid dual | $q_d=(0,t)q_r/2$; require $\|q_r\|=1$ and $q_r\cdot q_d=0$. Do not normalize the two halves independently. | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Two coupled constraints make the eight coefficients a rigid motion\|Study constraint and recovery]] |
| TRS/cache | Local TRS is useful authoring data; full affine world products may contain shear. Dirty flags record dependency invalidation. | [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Figure M10 - Representation follows the permitted operation\|M10: choose representation]] · [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Figure M11 - Invalidate the values that actually depend on the edit\|M11: cache dependencies]] |

## Four experiments to reconstruct without the chapter

1. Send $(4,2)$ through columns $(2,1)$ and $(-1,3)$. Obtain $(6,10)$ by column contributions and row sums. Explain why changing storage cannot change that answer.
2. Compare observations $(2,2.001)$ and $(2,2.002)$ for $A=\begin{bmatrix}1&1\\1&1.001\end{bmatrix}$. Recover $(1,1)$ versus $(0,2)$ and identify the source of uncertainty.
3. For $q_Z=(\sqrt2/2,0,0,\sqrt2/2)$, rotate X to Y. Then use $2q_Z$: true inverse still gives Y, conjugate sandwich gives four times Y, and the unit-optimized formula is outside its contract.
4. Compare the columns of $S R_{45^\circ}$ with $S=\operatorname{diag}(2,1)$. Their dot is $-3/2$, proving that the result is not an orthogonal rotation times diagonal scale.

**Teaching check:** explain every coefficient, space and failure result before running [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#A complete scalar laboratory makes the contracts observable|the standalone C++ laboratory]]. Written coverage and successful tests do not measure personal mastery.

[[Supplementary/Foundations#Phase 7 - Parts 601-700|Source ledger and gap]] · [[Supplementary/Derivations#Phase 7 - Transform reconstruction proofs|Derivations]] · [[Supplementary/Worked Traces#Phase 7 - The moving-panel tracebook|Tracebook]] · [[Dependency Map]]

## Added retrieval prompt

Project onto y=x: standard matrix [[1/2,1/2],[1/2,1/2]], aligned-basis matrix diag(1,0); explain why P squared equals P. [[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#A linear rule is determined by its basis images|Rebuild both descriptions]].

![[Visual Recall/Math Foundations/M07-1_rotated-coordinate-chart.png]]

**M07-1. A point expressed in a rotated local chart.** For the quarter-turn basis at (1,1), point (3,2) has local coordinates (1,-2) and reconstructs the original point.
