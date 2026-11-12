# Reciprocal bases and coordinate measurements

A basis reconstructs vectors; a reciprocal basis measures their coefficients. Those are different jobs when the basis is not orthonormal.

Let basis vectors $b_1,\ldots,b_n$ form the columns of matrix $B$. Coordinates $c$ reconstruct a vector by

$$
v=Bc.
$$

Recovering the coordinates means solving

$$
Bc=v,
\qquad c=B^{-1}v
$$

when $B$ is square and nonsingular. The rows of $B^{-1}$ are the coordinate-measurement covectors. In Euclidean coordinates, their vector representatives are the columns of

$$
B^{-\mathsf T}.
$$

These reciprocal vectors $b^i$ satisfy

$$
b^i\cdot b_j=\delta^i_j.
$$

For the slanted basis $b_1=(1,0)$, $b_2=(1,1)$ and vector $v=(3,2)$, directly dotting with the basis vectors gives $(3,5)$ and is wrong as a coordinate recovery rule. The reciprocal basis is

$$
b^1=(1,-1),\qquad b^2=(0,1),
$$

so the measurements are $b^1\cdot v=1$ and $b^2\cdot v=2$, exactly the coefficients that reconstruct $v=b_1+2b_2$.

A reciprocal basis is therefore a concrete bridge from vectors to covectors. It also explains why normal transformation uses an inverse transpose: normals are measurements that must continue to annihilate tangent directions after the tangent basis changes.

In numerical code, do not build a matrix inverse merely to solve one coordinate problem. Use a factorization or direct solve appropriate to the matrix structure and condition.

Full derivation and worked trace: [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#Recover a component with the right measuring rule]].

Related: [[Covectors linear functionals and normal transforms]], [[Basis coordinates and dimension]], [[Conditioning and singularity]].
