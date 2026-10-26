# Derivations

## Phase 8 - Linear systems and decompositions

These compact reconstructions support [[Continuous Notes/08 - From Constraints to Spectra - Systems, Decompositions and Matrix Functions|the full Phase 8 narrative]].

### The solution set is one solution plus the null space

If $A\mathbf x_0=\mathbf b$, then $A\mathbf x=\mathbf b$ exactly when $A(\mathbf x-\mathbf x_0)=0$. Hence

$$
\{\mathbf x:A\mathbf x=\mathbf b\}=\mathbf x_0+\operatorname{Null}(A).
$$

Rank-nullity follows by choosing a basis of the null space, extending it to a basis of the input space, and observing that the images of the added basis vectors form a basis of the column space. Therefore $\dim\operatorname{Null}(A)+\operatorname{rank}(A)=n$.

### The least-squares residual is perpendicular to the reachable output

Let $\widehat x$ minimize $\|Ax-b\|_2^2$. For any direction $h$, differentiate along $x(t)=\widehat x+th$:

$$
\frac{d}{dt}\|A(\widehat x+th)-b\|_2^2\bigg|_{t=0}
=2h^\mathsf T A^\mathsf T(A\widehat x-b)=0.
$$

Since this holds for every $h$, $A^\mathsf T(A\widehat x-b)=0$. Thus the residual is orthogonal to every column of $A$. With thin QR $A=QR$ and full column rank, $Q^TQ=I$ preserves the norm and reduces the minimizer to $R\widehat x=Q^Tb$.

### Why normal equations square the 2-norm condition number

For full-column-rank $A$, the symmetric eigenvalues of $A^TA$ are the squared singular values of $A$. Therefore

$$
\kappa_2(A^TA)=\frac{\sigma_{\max}^2}{\sigma_{\min}^2}=\kappa_2(A)^2.
$$

This is an algebraic identity and a numerical warning: forming the Gram matrix can make small singular directions harder to distinguish.

### The pseudoinverse conditions select least-squares then minimum norm

For $A=U\Sigma V^T$, define $A^+=V\Sigma^+U^T$ by reciprocating only retained positive singular values. The coordinates of $b$ along left-singular vectors with positive retained values determine $x$; components orthogonal to $\operatorname{Col}(A)$ remain residual. Null-space components of $x$ do not affect $Ax$, and choosing them as zero gives the minimum Euclidean norm. The numerical threshold controls which near-null directions are treated as absent; it belongs to the stated problem scale and precision.

### PCA through covariance and through the centered data SVD

For centered sample rows $X\in\mathbb R^{N\times d}$ and $X=U\Sigma V^T$,

$$
C=\frac{X^TX}{N-1}
=V\frac{\Sigma^T\Sigma}{N-1}V^T.
$$

Thus the right singular vectors of $X$ are covariance eigenvectors, and covariance eigenvalues equal $\sigma_i^2/(N-1)$. The identity explains PCA; computing SVD directly can avoid explicitly forming a Gram matrix with squared conditioning.

### Right polar decomposition from the SVD

For nonsingular square $A=U\Sigma V^T$, set $Q=UV^T$ and $H=V\Sigma V^T$. Then $Q$ is orthogonal, $H$ is symmetric positive definite, and

$$
QH=UV^TV\Sigma V^T=U\Sigma V^T=A.
$$

Also $H^2=V\Sigma^2V^T=A^TA$, so $H=(A^TA)^{1/2}$. If $A$ is singular, the positive-semidefinite square root still exists but the orthogonal completion may not be unique.

### Scaling and squaring identity

For any square matrix and integer $s\ge0$, the exponential's composition law gives

$$
e^A=\left(e^{A/2^s}\right)^{2^s}.
$$

The method chooses $s$ so a rational or polynomial approximation is accurate for the smaller-norm matrix, then squares. In floating point the identity motivates an algorithm; approximation and repeated rounding still need to be bounded and tested.

## Phase 7 - Transform reconstruction proofs

Essential proofs are in [[Continuous Notes/07 - From Basis Images to Moving Worlds - Matrices, Transforms and Quaternions|the continuous chapter]]. These exercises deepen the same contracts rather than introduce prerequisites hidden in this supplement.

### A change of basis changes a map on both sides

Let $A$ represent one endomorphism in a reference basis, and let the columns of invertible $P$ be a new basis expressed in that reference. A new-coordinate input $x'$ represents $x=Px'$. Applying the map gives $y=APx'$, and expressing its output in the new basis gives $y'=P^{-1}APx'$. Thus its new matrix is $A'=P^{-1}AP$. This is a similarity transform, not a spatial motion of the object.

The determinant is invariant under this change because

$$
\det(P^{-1}AP)=\det(P)^{-1}\det(A)\det(P)=\det(A).
$$

The trace is invariant because cyclically moving the first factor gives $\operatorname{tr}(P^{-1}AP)=\operatorname{tr}(APP^{-1})=\operatorname{tr}(A)$. These invariants do not identify the entire map: many distinct maps share trace and determinant. A row-major/column-major storage conversion is not a similarity transform; it changes address mapping, not the chosen basis.

### A scaled quaternion separates the three rotation formulas

For nonzero $q=m\widehat q$ with $\|\widehat q\|=1$ and $m>0$, the true-inverse sandwich is

$$
q(0,v)q^{-1}=\widehat q(0,v)\widehat q^*.
$$

Its scale cancels. The conjugate sandwich instead equals $m^2\widehat q(0,v)\widehat q^*$. For $q=2q_Z$, where $q_Z$ is the positive-Z quarter turn, the actions on X are respectively $(0,1,0)$ and $(0,4,0)$. The optimized unit-only expression $v+w(2u\times v)+u\times(2u\times v)$ gives $(-3,4,0)$ on that same invalid input. This demonstrates why describing every nonunit failure as uniform scaling is inaccurate: it depends on the actual formula used.

### Why a restricted TRS representation is not closed

The columns of $RD$ are pairwise perpendicular when $R$ is orthogonal and $D$ diagonal: $(RD)^T(RD)=D^2$ is diagonal. Let $S=\operatorname{diag}(2,1)$ and let $R$ rotate by $45^\circ$. The column dot product of $SR$ is $-3/2$, which is nonzero. Therefore no orthogonal $R'$ and diagonal $D'$ can satisfy $SR=R'D'$. Embedding this example in three dimensions proves the same limitation for general nonuniform-scale transform hierarchies. The affine matrix still exists; only the restricted storage promise fails.

### Coupled normalization of a rigid dual quaternion

Let a finite blended pair be $(r,d)$ with $a=\|r\|>0$. Define $r'=r/a$, $d_0=d/a$ and $d'=d_0-r'(r'\cdot d_0)$. Then

$$
\|r'\|=1,\qquad r'\cdot d'=r'\cdot d_0-(r'\cdot r')(r'\cdot d_0)=0.
$$

The recovered $2d'r'^*$ has zero scalar part, as a translation pure quaternion requires. This is projection onto the rigid constraints, not a claim that every weighted blend has the desired animation path. Sign-align both halves together before blending, handle cancellation of the real part, and keep scale/shear outside the rigid representation.

## Phase 6 - Geometry reconstruction proofs

These extensions belong to [[Continuous Notes/06 - From Movement to Meaning - Vectors, Geometry and Reliable Code|the Phase 6 story]]. Essential derivations remain in that volume. This section adds reconstruction exercises, not prerequisites hidden outside the teaching narrative.

### Why projection is an orthogonal decomposition

For a nonzero target vector $\mathbf b$, define $\alpha=(\mathbf a\cdot\mathbf b)/(\mathbf b\cdot\mathbf b)$ and $\mathbf r=\mathbf a-\alpha\mathbf b$. Then

$$
\mathbf r\cdot\mathbf b=\mathbf a\cdot\mathbf b-
\frac{\mathbf a\cdot\mathbf b}{\mathbf b\cdot\mathbf b}(\mathbf b\cdot\mathbf b)=0.
$$

For any candidate coefficient $t$, the difference is $\mathbf r+(\alpha-t)\mathbf b$. Its squared length is

$$
\|\mathbf r\|^2+(\alpha-t)^2\|\mathbf b\|^2,
$$

since its cross term vanishes. The minimum occurs uniquely at $t=\alpha$. Thus perpendicular residual, least-squares projection, and closest line point are one result.

For a unit axis $\mathbf u$, define the projection matrix $P=\mathbf u\mathbf u^\mathsf T$. Matrix multiplication reconstructs $(\mathbf a\cdot\mathbf u)\mathbf u$. Since $\mathbf u^\mathsf T\mathbf u=1$, we obtain $P^2=P$. The complementary rejection matrix $I-P$ is also idempotent. The plane reflection matrix is $I-2P$ and squares to identity:

$$
(I-2P)^2=I-4P+4P^2=I.
$$

This explains why reflecting twice restores a vector in exact arithmetic.

### Gram determinants connect length and area

For two generators, their Gram matrix is

$$
G=\begin{pmatrix}\mathbf a\cdot\mathbf a&\mathbf a\cdot\mathbf b\\
\mathbf a\cdot\mathbf b&\mathbf b\cdot\mathbf b\end{pmatrix}.
$$

Its determinant is $\|\mathbf a\|^2\|\mathbf b\|^2-(\mathbf a\cdot\mathbf b)^2$, the squared parallelogram area. For a basis matrix $B$ with three columns, $G=B^\mathsf TB$. Therefore

$$
\det(G)=\det(B^\mathsf T)\det(B)=\det(B)^2.
$$

This is why the Gram determinant loses the sign of orientation. Reflecting the complete frame changes $\det(B)$ but not its square. A zero determinant signals dependence; a small dimensional value by itself is not a complete conditioning estimate.

### Affine weights and translation

For points with coordinates $p_i$ and weights $w_i$, move every contributor by $t$. Their weighted result becomes

$$
\sum_iw_i(p_i+t)=\sum_iw_ip_i+\left(\sum_iw_i\right)t.
$$

Weights summing to one produce the translation of a point. Weights summing to zero produce an origin-independent displacement. Other sums do not transform as either canonical role without further interpretation. Nonnegative weights summing to one produce convex combinations. The sum condition and sign condition solve different problems.

### The normal transformation preserves a scalar pairing

Let a vector transform actively as $v'=Av$, and represent a linear measurement by column coefficients $n$, so it returns $n^\mathsf Tv$. Preserving every measured result requires $n'^\mathsf TAv=n^\mathsf Tv$ for every $v$, hence $n'=A^{-\mathsf T}n$ when $A$ is invertible.

For a passive basis description $v=Bc$, the same fixed measurement has coordinate coefficients $\omega_B=B^\mathsf Tn$. Notice that this is a different question from actively deforming a surface. Both formulas follow from the same pairing rather than contradictory transformation rules.

In nonorthonormal coordinates, a vector's coordinate column $c$ determines its metric measurement row $c^\mathsf TG$. Componentwise dotting assumes $G=I$. This connects basis changes, dual measurements and normals.

### Why perspective interpolation uses reciprocal weights

Let affine surface weights be $\mu_i$, with sum one, and homogeneous vertex values $H_i$. Before division, the combined representative is $H=\sum_i\mu_iH_i$. Its final coordinate is $W=\sum_i\mu_iw_i$. After division, the screen weights satisfy

$$
\lambda_i=\frac{\mu_iw_i}{W}.
$$

Thus $\mu_i=\lambda_iW/w_i$. Impose $\sum_i\mu_i=1$ to get $W=1/(\sum_i\lambda_i/w_i)$, then

$$
\mu_i=\frac{\lambda_i/w_i}{\sum_j\lambda_j/w_j}.
$$

Applying these weights to an affine attribute gives the perspective-correct formula in the volume. The derivation assumes a valid projected configuration with appropriate nonzero denominators and clipping; it is not an instruction to divide primitives crossing a projective boundary.

### Support queries commute with set addition in a specific way

For nonempty compact sets $A,B$, expand the dot product in the support definition:

$$
h_{A\oplus B}(d)=\max_{a\in A,b\in B}(d\cdot a+d\cdot b)
=h_A(d)+h_B(d).
$$

The maxima separate because the choices of $a$ and $b$ are independent. Reflecting $B$ gives $h_{-B}(d)=h_B(-d)$; a maximizing point is the negative of a maximizer for $B$ in direction $-d$. Thus one valid support point of the collision difference is $s_A(d)-s_B(-d)$. Maximizers need not be unique, so repeatable tie selection is part of an implementation contract.

### Proof, numerical algorithm and application policy

A mathematical identity can be correct while its direct arithmetic evaluation overflows or cancels. A numerically stable calculation can still solve an ill-conditioned problem. A certified sign for stored coordinates can still misrepresent a physically uncertain measurement. Teach these three boundaries separately; neither a tolerance nor a proof replaces all of them.

## XR - Sampling, contrast transfer and temporal evidence

### Why local pixels per degree is not the average

Start with a flat screen. Let its width be $W$, its viewing distance $D$, and its number of pixel columns $N$. The viewer looks at the center.

Pixel pitch is $p=W/N$. For a point at angle $\theta$:

$$
x=D\tan\theta.
$$

The angle is in radians. Differentiating gives the amount of screen distance per angular change:

$$
\frac{dx}{d\theta}=D\sec^2\theta.
$$

Divide by pixel pitch to get pixels per radian. Then multiply by $\pi/180$ to get pixels per degree.

At the center:

$$
\mathrm{PPD}_0=\frac{DN}{W}\frac{\pi}{180}.
$$

The average density uses the entire field of view instead:

$$
\mathrm{PPD}_{\mathrm{average}}
=\frac{N}{(360/\pi)\tan^{-1}(W/(2D))}.
$$

With $W=0.53\,\mathrm{m}$, $D=0.60\,\mathrm{m}$, and $N=1920$, the center has about 37.9362 PPD. The average is about 40.2863 PPD.

These numbers answer different questions. They are not conflicting measurements. A different projection or lens distortion requires its own mapping. See [[Visual angle and calibrated rays]].

### Sampling creates ambiguity before reconstruction begins

Take sample index $n$ and change the original frequency from $f_0$ to $f_0+mf_s$, where $m$ is an integer.

The extra phase is $2\pi mn$. At integer sample positions it is a whole number of cycles, so the samples do not change. For a real cosine, negative-frequency equivalence also matters.

In frequency coordinates, sampling repeats the original spectrum every $f_s$. If that spectrum occupies $[-B,B]$, the copies stay separate when:

$$
f_s>2B.
$$

This is the strict band-limited form used here.

Sinc reconstruction can select the original copy under the ideal assumptions. Once copies overlap, later smoothing cannot generally identify which part came from which original frequency.

The boundary case also matters. At $f_s/2$, a sine can give:

$$
\sin(\pi n)=0
$$

for every integer $n$. The continuous signal is nonzero between those observations.

### A normalized filter preserves constants, not arbitrary details

For convolution:

$$
y[n]=\sum_k h[k]x[n-k],
$$

a constant input $x[n]=a$ gives:

$$
y[n]=a\sum_k h[k].
$$

The filter preserves the constant when its weights sum to one and its boundary rule also preserves constants.

The weights $1/4$, $1/2$, and $1/4$ have frequency response:

$$
H(\omega)=\frac{1+\cos\omega}{2}.
$$

The gain is one at $\omega=0$ and zero at $\omega=\pi$.

A balanced difference of two normalized Gaussians instead has zero DC response. Unequal gains do not necessarily cancel.

For the stated continuous Gaussian blur model:

$$
\mathrm{MTF}(f)=e^{-2\pi^2\sigma^2f^2}.
$$

Use matching units for blur width $\sigma$ and frequency $f$. A finite discrete kernel need not have exactly this response.

### Temporal variance and age are conditional results

Assume a constant blend factor $\alpha$, a stationary target, correct alignment, and independent observations with equal noise variance.

The weight given to an observation $j$ frames old is:

$$
w_j=\alpha(1-\alpha)^j.
$$

The weights sum to one. For independent noise, variance uses the sum of squared weights:

$$
\frac{\operatorname{Var}(H)}{\operatorname{Var}(C)}
=\frac{\alpha^2}{1-(1-\alpha)^2}
=\frac{\alpha}{2-\alpha}.
$$

The mean age is:

$$
\operatorname{meanAge}=\frac{1-\alpha}{\alpha}
\quad\text{frames}.
$$

These formulas explain noise reduction and delayed response. They do not prove valid history after disocclusion or a changing target. Correlated errors add covariance terms, and reducing variance does not remove bias. See [[Temporal reprojection and history validation]].

### Psychometric scale is not itself the chosen threshold

Use the illustrative curve:

$$
F(x)=1-\frac12e^{-(x/\lambda)^k},
\qquad \lambda>0,\quad k>0.
$$

Define the threshold $T$ as the stimulus level giving 75% correct:

$$
F(T)=0.75.
$$

Rearrange:

$$
e^{-(T/\lambda)^k}=\frac12.
$$

Taking logarithms gives:

$$
\left(\frac{T}{\lambda}\right)^k=\ln2.
$$

Therefore:

$$
T=\lambda(\ln2)^{1/k}.
$$

The numerical lab verifies $F(T)=0.75$ for illustrative parameters. It does not fit participant responses or validate a paper's fitted coefficients.

## C++ - Assignment as a resource transaction

The [[Assignment and exception guarantees|strong guarantee]] requires a defined observable state before the operation and an unchanged promised state if preparation fails. Let D0 be the destination's original owned allocation/value and S the source. The teaching byte owner's copy assignment follows:

1. Construct replacement R from S without changing D0. Allocation failure propagates while D0 and S retain their original state.
2. Non-throwing swap transfers R's value into the destination and D0 into the temporary. This is the commit point: no remaining step can fail under the stated byte-copy/release contract.
3. Destroy R, releasing the former destination D0 exactly once.

Self-copy is explicitly guarded; an alternative copy-and-swap without that guard could still be safe but allocate unnecessarily. This proof assumes a non-throwing swap, safe release, and copy preparation that does not mutate its source. It does not establish whole-object strong guarantees for arbitrary memberwise assignment or a copy operation with external side effects.

For the failure-injection witness, initial owners A/B each hold one allocation. An injected exception before the new allocation succeeds leaves two owners/two live allocations and B's old byte9. Moving A into C then leaves A empty while C owns A's allocation. Moving C into B releases B's previous allocation before adoption, leaving one allocation. Destroying/resetting its final owner leaves zero. The source remains a live empty object throughout its moved-from interval.

### Why no-throw properties matter to growth

Suppose source element0 is moved and changed, then moving element1 throws. The partially populated new buffer can be cleaned up, but cleanup alone does not restore element0's old value. Copying that preserves source state can support rollback where available; a genuinely non-throwing move avoids that transfer failure. If copying is unavailable and moving may throw, consult the exact operation's exceptional-effects contract. move_if_noexcept encodes the relevant construction-expression choice, not a universal rollback theorem.

### Isolating sink counts from growth

With spare destination capacity and a Probe type whose copy/move constructors are instrumented: lvalue→value parameter→element gives one copy/one move; xvalue→parameter→element gives two moves; same-type prvalue→parameter→element gives direct parameter construction/one move. Cache growth would add transfer of old elements. Optional named-result NRVO is independent of these parameter routes, so record it separately instead of silently adding it to a fixed portable count.

## C++ - Qualification safety and borrow intervals

### Why nested const needs an intermediate guard

This is a hypothetical proof by contradiction, not legal code to run. Suppose `int**` could convert to `const int**`:

```cpp
const int immutable = 7;
int* mutable_path = nullptr;
int** outer = &mutable_path;
// const int** injected = outer; // actual C++ rejects this
// *injected = &immutable;       // hypothetical mutation of mutable_path
// *mutable_path = 9;            // would write an actually const object
```

Step one establishes a const object. Step two gives the mutable pointer a mutable type. The hypothetical conversion lets a const-object address enter that mutable pointer through an alias. The final write would violate the first promise without an explicit cast. Therefore adding const only at the deepest level is not safe. `const int* const*` makes the intermediate pointer read-only through that path, rejecting the injection assignment. This explains the safety requirement; the general language rule is qualification decomposition, not a special ban against double pointers. See [[Multilevel pointer qualification]].

### A borrow's access interval is an intersection

Let $L_o$ denote times at which the referent is live, $L_s$ times at which its storage/access relationship is valid, and $L_i$ times at which the path still names the intended logical entity. Under the API's mutation and synchronization contract, the admissible access times satisfy:

$$
L_{\mathrm{access}}\subseteq L_o\cap L_s\cap L_i.
$$

These are sets of times, not byte counts or durations to add. Const/type/bounds/permissions still add independent obligations. Reallocation can end the old storage relationship; destruction ends the old object's live interval; slot assignment can preserve the live object while ending its association with a logical entity. A non-null bit pattern establishes none of those intervals.

For an epoch witness, save $e_b$ when borrowing and compare with the live pool epoch $e_p$ before access. Acceptance requires $e_b=e_p$, but that test is meaningful only if the epoch owner remains live, every invalidating mutation advances the counter, equality cannot falsely recur through wrap, and mutation cannot race between check and use. A single-threaded epoch implements a conservative API contract, not a proof of ownership or lock-free safety. See [[Borrow contracts and pool epochs]] and the inline [[Continuous Notes/02 - C++ - From Objects to Reliable Programs#When a valid borrow stops naming the intended thing|slot trace]].

### Forwarding reconstructed from collapsing

For an lvalue argument of type int to deduced `T&&`, deduction chooses `T=int&`; applying `&&` to that substituted reference collapses to `int&`. For an int prvalue, deduction chooses `T=int`; the parameter is `int&&`. In the body the parameter's name is an lvalue in both cases. `std::forward<T>` uses the deduced T to restore the original category; unconditional move discards that distinction. This derivation requires the forwarding pattern, not merely any spelling containing two ampersands.

## C++ - Conventional record layout and identity horizon

This proof extends [[Continuous Notes/02 - C++ - From Objects to Reliable Programs#Bringing the object model back to bytes|the inline layout explanation]], not a universal class-layout algorithm. Assume an ordinary ABI with member size/alignment pairs, no bases, virtual machinery, overlap, packing or stronger requested alignment. For nonnegative offset x and positive required alignment a:

$$
\operatorname{roundUp}(x,a)=a\left\lceil\frac{x}{a}\right\rceil.
$$

Place each member at its rounded-up offset, advance by its size, then round the complete extent to the ABI's class alignment. For char/double/char with sizes 1/8/1 and alignments 1/8/1: first offset0; double roundUp(1,8)=8; final char offset16; extent17; size roundUp(17,8)=24. Reordering double/char/char gives offsets0/8/9, extent10 and size16. Adjacent array elements then begin at aligned multiples of the complete size. This is a target model to measure, not the meaning of standard-layout.

A generation counter of G bits has 2^G possible bit patterns. If one slot is reused r times per second, a wrap horizon under steady reuse is:

$$
T=\frac{2^G}{r}.
$$

T is in seconds when r is reuses/second. This predicts counter wrap, not guaranteed collision freedom: a retained old handle may match again after wrap, and a handle from another registry can match sooner. Reserve invalid states, width choice and reuse/quarantine policy can change the horizon. A validity check must include the owning registry and live slot state.

Neither proof makes a C++ object's physical address its semantic identity. Neither grants pointer arithmetic across adjacent non-array members.

Extended mathematical and logical derivations live here under durable concept headings. Update an existing derivation when a later chunk adds assumptions, a more general form, limiting cases, or corrections.

Each derivation records:

- the main-note chunk that first requires it;
- definitions, domains, units, conventions, and assumptions;
- every non-obvious step;
- dimensional, sign, boundary, and limiting-case checks;
- links to later chunks that deepen or reuse it.

## M001 - Positional value, masks, fields, and De Morgan

**Introduced by:** [[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#From a signal to an interpretation|C001 / M001]]

### Unsigned positional value and base conversion

For bits $b_i\in\{0,1\}$, an unsigned $N$-bit pattern means

$$
V=\sum_{i=0}^{N-1}b_i2^i.
$$

Euclidean division gives $n=2q+r$, with $r\in\{0,1\}$. Therefore $r=b_0$; repeating on $q$ exposes $b_1,b_2,\ldots$, so remainders arrive LSB-first. Decoding left-to-right uses the Horner recurrence $v\leftarrow2v+b$.

Check for `10101101`: $128+32+8+4+1=173$. Repeated division of 173 returns remainders `1,0,1,1,0,1,0,1`, whose reverse is `10101101`.

### Contiguous masks, extraction, and insertion

A width-$w$ low mask is

$$
L_w=2^w-1
$$

because $2^w$ is `1` followed by $w$ zeros and subtracting one borrows through them to produce $w$ ones. Moving the field to offset $s$ gives $M=L_w\ll s$.

Extraction first normalizes, then isolates:

$$
extract(x,s,w)=(x\gg s)\land L_w.
$$

Insertion must preserve every position outside $M$. The term $x\land\neg M$ clears exactly the destination; $(v\land L_w)\ll s$ encodes only values representable by the field. Since their 1-bits cannot overlap outside the destination,

$$
insert(x,v,s,w)=(x\land\neg M)\lor((v\land L_w)\ll s).
$$

Implementation checks: $w>0$, $s+w\le N$, shifts are smaller than the carrier width, and complement is evaluated at carrier width. A semantic API must choose whether an oversized $v$ is rejected or truncated.

### De Morgan by exhaustive truth assignment

For two Boolean inputs there are only four assignments. Evaluating both sides on `(0,0)`, `(0,1)`, `(1,0)`, and `(1,1)` yields identical result columns:

| A | B | not (A and B) | (not A) or (not B) | not (A or B) | (not A) and (not B) |
|---:|---:|---:|---:|---:|---:|
| 0 | 0 | 1 | 1 | 1 | 1 |
| 0 | 1 | 1 | 1 | 0 | 0 |
| 1 | 0 | 1 | 1 | 0 | 0 |
| 1 | 1 | 0 | 0 | 0 | 0 |

Thus

$$
\neg(A\land B)=\neg A\lor\neg B,
\qquad
\neg(A\lor B)=\neg A\land\neg B.
$$

The same identities apply lane-by-lane to fixed-width bit vectors, provided the complement width is fixed.

## M002 - Signed representations, adders, and arithmetic policies

**Introduced by:** [[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Arithmetic becomes a policy|C002 / M002]]

### Finite ranges and signed encodings

An $N$-bit carrier has $2^N$ patterns. Unsigned positional interpretation maps them bijectively to $[0,2^N-1]$. Signed magnitude and one's complement each reserve separate positive and negative zero patterns, leaving range

$$
-(2^{N-1}-1)\le x\le2^{N-1}-1.
$$

Two's complement instead interprets the high bit with weight $-2^{N-1}$, producing

$$
-2^{N-1}\le x\le2^{N-1}-1.
$$

Zero occupies one pattern, so the recovered pattern becomes the extra negative endpoint. Consequently, the positive counterpart of $-2^{N-1}$ cannot fit in the same signed type.

### Two's-complement negation and subtraction

In the residue ring modulo $2^N$, $-x$ is represented by $2^N-x$. Since the all-ones word is $2^N-1$,

$$
2^N-x=(2^N-1-x)+1=\sim x+1.
$$

Therefore

$$
A-B=A+(-B)=A+\sim B+1.
$$

The low $N$ result bits are identical whether interpreted as the unsigned residue or an in-range signed result; flags and the chosen type supply the interpretation.

### Full adder, carry, and signed overflow

For inputs $A,B,C_{in}$, the sum bit is

$$
S=A\oplus B\oplus C_{in}.
$$

Carry occurs when at least two inputs are 1:

$$
C_{out}=(A\land B)\lor(C_{in}\land(A\oplus B)).
$$

For bit position $i$, define generate $G_i=A_i\land B_i$ and propagate $P_i=A_i\oplus B_i$. Then

$$
C_{i+1}=G_i\lor(P_i\land C_i).
$$

Ripple carry evaluates this recurrence sequentially across positions; carry-lookahead or prefix networks expand and combine the terms in parallel.

Signed addition overflows precisely when operand signs match and the result sign differs:

$$
V=\neg(A\oplus B)\land(A\oplus R)\land signBit.
$$

Carry and overflow may disagree because they answer unsigned and signed representability questions about the same result bits.

### Multiplication, division, and policy choices

Binary multiplication decomposes into selected partial products:

$$
A\times B=\sum_i B_i(A\ll i).
$$

Unsigned long division maintains

$$
D=Qd+R,\qquad 0\le R<d,
$$

by shifting each dividend bit into $R$, subtracting $d$ when possible, and setting the corresponding quotient bit.

When a result cannot fit, the correct response is a semantic choice: modulo residue, explicit checked failure, wider intermediate, boundary saturation, or arbitrary precision. Saturation is not generally associative, so parallel reordering can change results.

## Definitions and proof steps behind the compact equations

### Why an N-bit complement subtracts from all ones

Each bit of x contributes either 0 or its weight $2^i$. Complementing replaces that contribution by $2^i-b_i2^i$. Summing over all positions gives:

$$
\operatorname{NOT}_N(x)=\sum_{i=0}^{N-1}(1-b_i)2^i
=\left(\sum_{i=0}^{N-1}2^i\right)-x=(2^N-1)-x.
$$

Adding one produces the residue representing negation. For x=0, the intermediate is $2^N$, so reducing modulo $2^N$ returns zero. This is why the equality for negation must specify modular width instead of pretending every signed positive counterpart fits.

### Why insertion preserves all other bits

Consider one position i. If mask bit $M_i=0$, then $(x\land\operatorname{NOT}_N(M))_i=x_i$ and the encoded new field contributes zero there. The output is therefore $x_i$. If $M_i=1$, clearing contributes zero and the encoded new field supplies its bit. These two cases cover every position, proving preservation outside the field and replacement inside it.

### How two half adders make a full adder

First compute $t=A\oplus B$ and $g=A\land B$. Then add incoming carry c to t: $S=t\oplus c$ and $h=t\land c$. The outgoing carry is $g\lor h$, so:

$$
S=A\oplus B\oplus c,\qquad c'=(A\land B)\lor(c\land(A\oplus B)).
$$

If A and B are both 1, g already supplies carry. If exactly one is 1, c determines whether the total reaches two. If both are 0, a single c cannot create carry. This explains the majority rule without treating the formula as a memorized symbol string.

### C++ common-type decision after promotion

For signed type S and unsigned type U: if U has at least S's rank, use U; otherwise use S only if S represents every U value; otherwise use the unsigned counterpart of S. Integer rank orders conversion preference; it is not merely a count of storage bytes. Consequently, int −1 versus unsigned int 1 usually converts −1 to unsigned, while a sufficiently wide signed long long can retain −1 when compared with unsigned int.

These decisions occur before arithmetic or comparison. They also explain why casting the final result cannot repair an earlier signed overflow. A wider destination is useful only if the operation itself was performed safely.

## M003 - Byte order, layout, and translation

**Introduced by:** [[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Where the bytes live|C003 / M003]].

### Base-256 decomposition and codec inverse

Assume n octets, unsigned $0\le x<256^n$, and digits $0\le d_i<256$. Euclidean division by 256 yields a low digit and quotient. Repeating produces:

$$
d_i=\left\lfloor x/256^i\right\rfloor\bmod256,\qquad
x=\sum_{i=0}^{n-1}d_i256^i.
$$

Little-endian stream element i is $d_i$; big-endian element i is $d_{n-1-i}$. Decoding LE attaches weight $256^i$ to byte i. Decoding BE starts at zero and applies $v_{j+1}=256v_j+b_j$; after j bytes it equals the weighted sum of the prefix. After n bytes, both recover x under their matching order. Fixed n preserves leading zero bytes.

For four octets, swapping sends digit i to position 3−i:

$$
\operatorname{swap}_{32}(x)=\sum_{i=0}^{3}d_i256^{3-i},\qquad
\operatorname{swap}_{32}(\operatorname{swap}_{32}(x))=x.
$$

This is a byte permutation, not intra-byte bit reversal. Host-to-wire conversion is conditional on the format contract.

### Alignment rounding without hiding overflow

Let p be a nonnegative byte offset and A a positive alignment. Write $p=qA+r$, $0\le r<A$. If r=0, p is aligned; otherwise the minimal larger multiple is $(q+1)A$. Define padding $\delta$:

$$
\delta=(A-(p\bmod A))\bmod A,\qquad
\operatorname{alignUp}(p,A)=p+\delta,\qquad 0\le\delta<A.
$$

This proves minimality and idempotence. At finite unsigned maximum M, require $\delta\le M-p$ before adding. For $A=2^k$, the bottom k bits encode r. Adding A−1 and clearing those bits gives:

$$
\operatorname{alignUp}(p,A)=(p+A-1)\mathbin{\&}\operatorname{NOT}_N(A-1).
$$

The mask formula requires nonzero power-of-two A and an addition fitting the N-bit carrier. The delta method can accept a highest aligned p even when p+A−1 would overflow. An aligned numeric offset neither allocates storage nor establishes pointer validity.

### Member offsets and tail padding

For an illustrative record with member sizes $s_j$, alignments $a_j$, no special overlap/base rules, and stated ABI placement convention, let $e_0=0$ be the first unused offset:

$$
o_j=\operatorname{alignUp}(e_j,a_j),\qquad
e_{j+1}=o_j+s_j,\qquad
\text{internal padding}_j=o_j-e_j.
$$

For complete-record alignment A and final member end E:

$$
S=\operatorname{alignUp}(E,A),\qquad
\text{tail padding}=S-E.
$$

Sizes/alignments 1/4/1 give offsets 0/4/8, E=9, A=4, S=12. Array element i starts at base+iS; if base and S are multiples of A, every element remains aligned. This models specified ABI assumptions, not a rule overriding compiler layout. Observe standard-layout records with sizeof, alignof, and offsetof.

### Page decomposition and offset preservation

For page size $P=2^k$, Euclidean division gives a unique quotient VPN and remainder o:

$$
VA=VPN\,P+o,\qquad VPN=\lfloor VA/P\rfloor,\qquad 0\le o<P.
$$

A process-specific valid mapping supplies physical frame PFN and permissions:

$$
PA=PFN\,P+o.
$$

PFN×P has zero low k bits, so addition is equivalently OR after shifting. Offset is unchanged; only page identity changes. Mapping need not be invertible: several virtual pages can share one frame. Width, permissions, residence, and legality require separate checks.

For positive access length L, bytes starting at VA cross the page boundary iff $L>P-o$, assuming a representable address range. At o=4094 and L=4 with P=4096, two bytes occupy each page. Both mappings must permit access.

### Translation reach and record footprint

T TLB entries for page size P have illustrative reach T×P bytes; real hierarchy, associativity, and mixed page sizes complicate that bound. One GiB requires $2^{30}/2^{12}=262144$ pages at 4 KiB, or $2^{30}/2^{21}=512$ at 2 MiB.

Shrinking N records saves $N(S_{\mathrm{old}}-S_{\mathrm{new}})$ bytes. A 24→16 byte change saves eight million bytes for one million records. This is footprint arithmetic, not measured throughput: hot fields, cache boundaries, prefetching, and access patterns still matter.

[[Alignment and padding]] · [[Pages and page offsets]] · [[TLB]] · [[Serialization]]

## M004 - Floating-point fields, spacing, and rounding

**Introduced by:** [[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#A stretching ruler for real-valued quantities|M004, Parts 61–76]].

### Normal value from fraction positions

Let $t$ be the stored fraction width, $f_j$ the digit at fractional position $j$ (weight $2^{-j}$), and $F$ the unsigned integer represented by those digits. Multiplying their fractional sum by $2^t$ gives the integer field:

$$
\sum_{j=1}^{t}f_j2^{-j}=\frac{F}{2^t}.
$$

Normalization contributes the implicit leading one. Stored exponent $E$ and bias $B$ give $e=E-B$. Thus $x=(-1)^S(1+F/2^t)2^e$. For binary32, $t=23,B=127$, valid normal fields $1\le E\le254$ imply $-126\le e\le127$. For binary64, $t=52,B=1023$ and $1\le E\le2046$ imply $-1022\le e\le1023$.

Minimum normal uses $F=0,e=e_{\min}$; maximum finite uses $F=2^t-1,e=e_{\max}$:

$$
x_{\min,\mathrm{normal}}=2^{e_{\min}},\qquad
x_{\max}=(2-2^{-t})2^{e_{\max}}.
$$

These are mathematical format values, not a promise about every C++ floating type.

### Subnormal lattice and boundary continuity

For $E=0$, use effective exponent $e_{\min}$ and leading zero, not one:

$$
x=(-1)^S(F/2^t)2^{e_{\min}}=(-1)^S F\,2^{e_{\min}-t}.
$$

A lattice here means values occur at integer multiples of a common step. Binary32 step is $2^{-126-23}=2^{-149}$; binary64 step is $2^{-1022-52}=2^{-1074}$.

At $F=2^t-1$, the largest positive subnormal is $2^{e_{\min}}-2^{e_{\min}-t}$. The next value is the smallest normal $2^{e_{\min}}$. Their gap is exactly the same subnormal step. The next normal also increments its $F$ by one, so its gap is the same. This proves boundary continuity without calling every tiny result an underflow exception.

### Spacing, epsilon, and nearest-rounding error

In normal binade $[2^e,2^{e+1})$, consecutive values differ only by $F\to F+1$ (except the last transition to the next binade). Subtract their formulas:

$$
\Delta(e)=\left(\frac{F+1-F}{2^t}\right)2^e=2^{e-t}.
$$

Since $p=t+1$, $\Delta(e)=2^{e-(p-1)}$. At $e=0$ this is epsilon $2^{-t}$. At an ordinary power-of-two boundary above the minimum-normal edge, the lower gap belongs to exponent $e-1$ and is half the upper gap.

For an exact positive value $z$ in a normal binade, nearest rounding has absolute error at most $\Delta(e)/2$. Since $z\ge2^e$:

$$
\frac{|\operatorname{fl}(z)-z|}{|z|}
\le\frac{2^{e-t-1}}{2^e}=2^{-p}=u.
$$

Here $\operatorname{fl}$ denotes rounding to the destination format. Scope: no overflow, normal-range nearest rounding; subnormal relative precision needs separate treatment. “Machine epsilon” in this vault means upward spacing at 1, while unit roundoff $u$ is its half. Literature sometimes uses different epsilon conventions, so name the definition.

### Guard, round, sticky and even ties

Let retained significand integer be $q$. Guard $G$ is first discarded bit, round $R$ second, sticky $T$ ORs all later bits, and $L=q\bmod2$. If $G=0$, the discarded tail is below half a retained step. If $G=1$ and $R$ or $T$ is 1, it is above half. If $G=1,R=T=0$, it is exactly half: increment odd $q$, retain even $q$.

$$
\operatorname{increment}=G\land(R\lor T\lor L).
$$

The increment is on retained magnitude; sign/direction logic is additionally required for directed rounding. A carry can renormalize the significand and increase exponent. The toy midpoint 1.875 at three significant bits rounds to 2, not to an unnormalized stored significand.

### A zero-collapsed representation key

For a finite binary32 raw word $b$, let $m=b\mathbin{\&}\texttt{0x7FFFFFFF}$ and $C=2^{31}$. Define key $K=C-m$ for negative patterns, $K=C+m$ otherwise. Both zeros map to $C$. Larger magnitudes on the negative side decrease the key; larger positive values increase it. Distance is the unsigned absolute key difference.

Check: negative minimum subnormal has key $C-1$, zero $C$, positive minimum subnormal $C+1$: distance across the two nonzero neighbors is 2. Reject NaNs/infinities rather than assigning a misleading numerical metric. This is a defined testing convention, not the only possible ULP convention.

### Scaling a norm and a determinant

For finite vector components with $m=\max(|x|,|y|)>0$, factor $m^2$ before squaring:

$$
\sqrt{x^2+y^2}=m\sqrt{(x/m)^2+(y/m)^2}.
$$

The ratios have magnitude at most one. Intermediate overflow from $x^2$ is avoided; final overflow and other accuracy issues remain possible. Zero and non-finite inputs require explicit branches. Do not treat a teaching identity as a fully specified library implementation.

For an invertible 2×2 matrix $A$, $\det(cA)=c^2\det(A)$. For a consistent induced norm, $\kappa(A)=\|A\|\|A^{-1}\|$, and nonzero scalar $c$ gives $\kappa(cA)=|c|\|A\|\cdot|c|^{-1}\|A^{-1}\|=\kappa(A)$. An absolute determinant cutoff changes with scale even though this conditioning measure does not. General singularity/conditioning methods await later mathematics.

### Representation equality versus numerical equality

Binary32 zeros differ only by sign, but numerical equality identifies them. NaN ordinary equality identifies neither itself nor another NaN, even when raw words are equal. A raw-bit serializer, a numeric comparison, a hash policy, and a canonical format must therefore state separate contracts. Mapping every NaN to one quiet pattern loses payload/signaling information deliberately; it is not a lossless representation conversion.

## M005 - Representation error, cancellation, and rounding graphs

**Introduced by:** [[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#When arithmetic meets uncertainty|M005]].

### Termination in a base and exact one-tenth error

A finite fractional base-$B$ expansion has value $m/B^k$. Reducing $p/q$ means $q$ must divide $B^k$. Conversely, if every prime in $q$ divides $B$, choose $k$ large enough that $q$ divides $B^k$; multiply numerator and denominator to get a finite expansion. In binary this means $q=2^j$.

For binary32 word 3DCCCCCD: stored exponent 123 gives $e=-4$; fraction is 4CCCCD, and the hidden-one integer is $2^{23}+F=13421773$. Thus value is $13421773\,2^{-27}$. Subtract exact one tenth with a common denominator:

$$
\frac{13421773}{134217728}-\frac1{10}
=\frac{10(13421773)-134217728}{10(134217728)}
=\frac{2}{1342177280}=\frac1{671088640}.
$$

This is an exact rational calculation; computing both sides with rounded 0.1 is not the same reference.

### Cancellation bound and exact subtraction

For intended values $a,b$ and relative operand errors bounded by $\eta$, expansion gives perturbed difference $a-b+a\delta_a-b\delta_b$. Triangle inequality gives absolute error at most $\eta(|a|+|b|)$. Divide by $|a-b|$ only when nonzero. Additional final subtraction rounding must be analyzed separately.

Sterbenz's exact-difference lemma concerns nearby stored values in a suitable radix format with gradual underflow. For nonnegative $x,y$ with $x/2\le y\le2x$, the difference is representable. The appropriate common-exponent integer significands have enough trailing-zero/range structure after cancellation to fit the smaller result. It says nothing about the unknown real operands' initial rounding errors; changing subnormal semantics can remove its assumptions.

The attachment's binary pair is 1753/1024 and 1751/1024, hence difference $2^{-9}$, not its stated longer pattern. For desired decimal 1.00000006, the nearest binary32 word is 3F800001, so subtracting stored 1 is exactly $2^{-23}$ although the desired difference is $6\times10^{-8}$.

### Rounding products and summation bounds

Let local factors satisfy $|\delta_i|\le u$, with $nu<1$. The standard product-of-factors bound writes a relevant product (and supported inverse-factor variant) as $1+\theta_n$, $|\theta_n|\le\gamma_n=nu/(1-nu)$. The denominator accounts for interaction terms; for small $nu$ it approximates $nu$. It is a tool applied to dependency paths, not a universal bound obtained solely by counting source operators.

For sequential sum $s=\sum_i x_i$, each input travels through at most $n-1$ rounded additions. Expanding the computed expression attaches factors to terms; triangle inequality yields:

$$
|\widehat s-s|\le\gamma_{n-1}\sum_i|x_i|.
$$

Assume nearest normal-range arithmetic and no range failure. The ratio $\sum_i|x_i|/|s|$ exposes sensitivity when terms cancel. Balanced trees reduce maximum path length; they do not eliminate input uncertainty or promise the smallest error on every example.

### FMA witness and distributive witness

Put $p=2^{-23}$, $a=b=1+p$, $c=-(1+2p)$. The exact product is $1+2p+p^2$. Separate binary32 product rounding removes $p^2$, so the next addition gives zero. A fused operation subtracts before that rounding and produces representable $p^2=2^{-46}$.

For $a=10^{10},b=1+2^{-23},c=-1$, $b+c=2^{-23}$ exactly. The factored path gives representable $10^{10}2^{-23}=1192.0928955078125$. The separately rounded product $ab$ lies on a 1024-spaced grid and rounds to $10^{10}+1024$; $ac=-10^{10}$, leaving 1024. Compiler contraction changes the graph and must be controlled.

### Capped history is mathematically order-dependent

For positive cap $M$ and observation weight $w>0$, define $W'=\min(M,W+w)$, $c=\min(w,W')$, $r=W'-c$. Then $D'=(Dr+dc)/W'$. Without the cap and with exact totals, induction recovers $D=\sum_iw_id_i/\sum_iw_i$.

With $M=2,w=1$, ordering 0,1,−1 gives after three observations $D=-1/4$; ordering 0,−1,1 gives $D=+1/4$. All values are dyadic and exact. Thus a cap changes history influence even without roundoff. Batch exact averaging is not an equivalent replacement.

### Tolerance is not an equivalence relation

Absolute closeness at threshold 1 links 0 to 0.75 and 0.75 to 1.5, but not 0 to 1.5. Nontransitivity prevents treating this predicate as mathematical equality for arbitrary grouping/hashing. Numeric equality and bit identity also differ at signed zero/NaNs. Choose contracts independently.

## Representation choice - Fixed grids, codebooks and low-precision fields

[[Continuous Notes/01 - From Signals to Meaning - Bits, Numbers and Memory#Choosing what information to keep|Continuous explanation]] supplies the motivation and examples. This companion reconstructs the constraints.

### A scale is an interpretation, not an integer operation

Let signed N-bit raw I range from -2^(N-1) through 2^(N-1)-1, and choose positive S=2^F. Dividing both endpoints by S gives the exact decoded interval. Adjacent raw integers differ by one, hence decoded step is 1/S.

Nearest rounding selects q with |xS-q|≤1/2 when no clipping occurs. Divide by positive S:

$$
\left|x-\frac qS\right|\le\frac1{2S}.
$$

The symmetric range requirement M must fit the positive endpoint, which is one raw unit smaller in magnitude than the negative endpoint. Thus require M·2^F≤2^(N-1)-1. Taking base-two logarithms yields the scale feasibility interval in the story, with integer floor/ceiling and F≥0. A requested maximum error e can instead require 1/(2S)≤e; a requested step and a requested error are not the same condition.

At F=16, raw minimum −2147483648 decodes to −32768; maximum2147483647 decodes to32768−2^-16. Using an approximate endpoint32768 in a conversion check incorrectly accepts a value whose raw code cannot fit.

### Different operand scales require an explicit output scale

For decoded a=A/S_a and b=B/S_b, desired output scale S_o gives:

$$
I_{\mathrm{mul}}=\operatorname{roundPolicy}
  \left(\frac{AB\,S_o}{S_aS_b}\right),
\qquad
I_{\mathrm{div}}=\operatorname{roundPolicy}
  \left(\frac{A\,S_bS_o}{B\,S_a}\right),\quad B\ne0.
$$

With all scales equal S, these reduce to AB/S and AS/B. Intermediate products in the general formula can overflow even when the simplified answer fits; simplify factors carefully or use a proved wider domain. The lab implements only the bounded same-scale case.

For signed nearest-even division, separate sign from unsigned magnitude. Write n=qd+r with 0≤r<d, d>0. Increment q if r>d−r, or if r=d−r and q is odd. Comparing with d−r avoids overflowing 2r. Apply the sign after checking the destination magnitude, including the asymmetric signed minimum. For −5/2, q=2,r=1: tie keeps even2, then sign gives−2. For−7/2, q=3,r=1: tie increments to4, then sign gives−4.

### Affine codebook error and endpoint constraints

Let s>0 and integer zero point z define reconstruction s(q−z). Without clipping, q−z=round(x/s), so multiplication of the nearest-code bound by s gives:

$$
\left|x-s(q-z)\right|\le s/2.
$$

This derivation assumes an ideal scale and rounding operation. Finite scale storage and evaluation add error; clipping invalidates the half-step bound.

An 8-bit unsigned endpoint fit from−1 to1 has s=2/255 and ideal zero point127.5. An integer zero point cannot equal127.5. Choosing128 makes zero exact but reconstructs endpoints−256/255 and254/255. Choosing127 shifts the interval oppositely. Rounding an affine zero point changes endpoint fit; one cannot declare all three targets exact under this uniform grid.

### Metadata changes the compression ratio

For N>0 samples and G>0, code bytes are ceil(N/2), and float32 scales cost4·ceil(N/G). For full groups and even N, divide payload bits by N to get4+32/G bits/sample. Partial groups and odd code counts require the ceiling form.

At N=5,G=32, payload is3+4=7 bytes, versus20 FP32 bytes: approximately2.86× reduction, not6.4×. At large divisible N andG=32, payload approaches0.625 bytes/sample, giving4/0.625=6.4×. Neither figure includes the header.

For a one-bit row with C samples and two binary32 centroids, a separately packed row costs ceil(C/8)+8 bytes before its other metadata. The approximation1+64/C bits/sample ignores final-byte padding.

### Low precision changes the grid, not the decoding principle

A normal binary format with t fraction bits has p=t+1 precision and binade spacing2^(e−t). Binary16 uses t=10,e_min=−14, giving minimum subnormal2^(−14−10)=2^-24. BF16 uses t=7,e_min=−126, giving2^-133 in the ideal interchange format.

Maximum finite uses the largest finite exponent and fraction1−2^-t, so (2−2^-t)2^e_max. This derives65504 for binary16 and(2−2^-7)2^127 for BF16.

In binary32→BF16 nearest-even conversion, the discarded low word d is compared with halfway0x8000. If d is above halfway, increment the high word; at equality, increment only if its low bit is odd. The finite-word bias0x7FFF+L expresses this decision. Classify NaNs first, or a payload only in discarded bits can turn into an infinity encoding.

## External bytes - Bounds, identity and error detection

### Bounds belong to the arithmetic domain of the parser

For unsigned maximum U, a nonzero factor a permits multiplication a·b only when b≤floor(U/a). Do this before evaluating the product. For a span length L and read length n at offset o, require o≤L and n≤L−o; the first comparison protects subtraction.

After a validated header length H, exact payload length P requires P=L−H. This avoids computing H+P before proving it fits. File-schema uint64 values also need representation/resource checks before converting to host size_t or allocating.

### CRC polynomial toy trace

This is a toy unreflected CRC with zero initialization and no final XOR, not the laboratory's CRC-32 parameters. Let message bits1101 mean polynomial x³+x²+1 and generator1011 mean x³+x+1. Append three zero positions and divide with XOR:

```text
1101000 xor 1011000 = 0110000
0110000 xor 0101100 = 0011100
0011100 xor 0010110 = 0001010
0001010 xor 0001011 = 0000001
```

Remainder001 is appended to the original message, giving1101001. Dividing that codeword by1011 yields remainder zero. This works because coefficients lie in GF(2): subtracting a polynomial is XOR, with no borrow. Real named CRC parameters must additionally declare reflection, initialization, final XOR and the message coverage.

### A digest cannot be injective on unrestricted messages

An h-bit digest has2^h outputs. Choose2^h+1 distinct messages. At least two must share an output by the pigeonhole principle. Cryptographic strength concerns difficulty of finding certain collisions/preimages, not mathematical nonexistence.

A changed file and a recomputed untrusted digest can agree. Authentication needs a trusted reference or keyed/signature mechanism; a polynomial remainder alone provides neither identity nor authorization.

[[Supplementary/Code Snippets#Representation codecs and bounded file laboratory|Executable bounded codecs]] · [[Supplementary/Sources and Code Anchors|Verification scope]]
