# Reciprocal bases and coordinate measurements

Reciprocal basis measurements recover coefficients from a nonorthogonal reconstruction basis. Covectors are the scalar measurement rules; a chosen inner product represents them by arrows.

[[Continuous Notes/00 - From Vectors to Images - Integrated Mathematics for Visual Computing#Recover a component with the \right measuring rule|Full definitions, derivation and worked trace]].

## Mastery expansion — coordinates are measurements by a reciprocal basis

For a non-orthonormal basis (b_1,\ldots,b_n), dotting a vector directly with (b_i) does not recover its coordinates. The reciprocal basis (b^i) satisfies (b^i\cdot b_j=\delta^i_j), so the coordinate is (x^i=b^i\cdot x).

If (B) stores basis vectors as columns, the coordinate vector is (c=B^{-1}x). The rows of (B^{-1}) are the reciprocal covectors. In an orthonormal basis the reciprocal basis coincides with the original basis, which is why the distinction is easy to miss.

Near linearly dependent bases make the reciprocal vectors large; coordinate recovery then becomes ill-conditioned.

**Executable witness:** `basis_measurement_lab.cpp`.

**Required next witness:** 3-D randomized tests comparing reciprocal-basis measurements, linear solve and residual/condition estimates.

