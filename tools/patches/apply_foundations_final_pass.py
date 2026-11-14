#!/usr/bin/env python3
from pathlib import Path
import os
import tempfile

ROOT = Path(__file__).resolve().parents[2]
NOTE = ROOT / "Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md"

def code_block(path, language):
    return "\n\n~~~" + language + "\n" + path.read_text().rstrip() + "\n~~~\n"

def insert_or_refresh_before(text, anchor, marker, body):
    if text.count(anchor) != 1:
        raise SystemExit("expected exactly one insertion anchor: " + anchor)
    normalized = body.strip("\n")
    marker_count = text.count(marker)
    if marker_count > 1:
        raise SystemExit("duplicate managed-section marker: " + marker)
    if marker_count == 1:
        marker_start = text.index(marker)
        # Consume blank lines immediately before the managed section so refresh
        # cannot accumulate whitespace on repeated runs.
        start = marker_start
        while start > 0 and text[start - 1] == "\n":
            start -= 1
        anchor_start = text.find(anchor, marker_start)
        if anchor_start < 0:
            raise SystemExit("managed section is no longer before its anchor: " + marker)
        # A second managed marker before this anchor signals ambiguous ownership.
        next_marker = text.find("<!-- Bible-final-pass:", marker_start + len(marker))
        if next_marker >= 0 and next_marker < anchor_start:
            raise SystemExit("managed sections overlap before anchor: " + marker)
        end = anchor_start
        return text[:start] + "\n\n" + normalized + "\n\n" + text[end:]
    return text.replace(anchor, normalized + "\n\n" + anchor, 1)

text = NOTE.read_text()

life_code = code_block(ROOT / "tools/labs/temporary_lifetime_lab.cpp", "cpp")
se3_code = code_block(ROOT / "tools/labs/se3_exp_log_lab.cpp", "cpp")
inf_code = code_block(ROOT / "tools/labs/research_inference_lab.py", "python")

life = """
<!-- Bible-final-pass: C++ temporary lifetime -->
#### Turn the lifetime rule into a scope trace

The preceding example showed that binding a local const reference can extend a temporary, while returning the same reference through a function does not extend it again. The distinction is visible if we count the object while the local binding is active, then watch a non-owning string view observe storage whose owner remains alive. That is a deliberately small witness: it makes the valid timeline executable without dereferencing a dangling observer.

Its comments identify the owner and the exact scope boundary. A pointer, span, iterator, callback capture, or coroutine reference follows the same review question: which object owns the storage, and which event ends its validity?
""" + life_code + """
"""

se3 = r"""
<!-- Bible-final-pass: SE3 exp log -->
#### From a six-number pose update to an actual rigid transform

The previous section derived the tangent space of SO(3) and showed how a small rotational perturbation changes a point. One bridge was still missing: how a six-component local pose update becomes a finite rigid transform, and how a nearby rigid transform becomes a six-component update again.

A rigid transform $T\in SE(3)$ contains a rotation $R\in SO(3)$ and a translation $t$:

$$
T=\begin{bmatrix}R&t\\0&1\end{bmatrix}
$$

A local six-vector is commonly written as:

$$
\xi=\begin{bmatrix}\omega\\\rho\end{bmatrix}
$$

$\omega$ is a three-component local rotation vector measured in radians. $\rho$ is the translational part of the tangent coordinate. In general, $\rho$ is not the final translation column $t$ when rotation is nonzero.

The wedge form is:

$$
\widehat{\xi}=\begin{bmatrix}[\omega]_\times&\rho\\0&0\end{bmatrix}
$$

The group exponential gives:

$$
\exp(\widehat{\xi})=\begin{bmatrix}\exp([\omega]_\times)&V(\omega)\rho\\0&1\end{bmatrix}
$$

Let $\theta=\lVert\omega\rVert$. Rodrigues' formula is:

$$
R=I+\frac{\sin\theta}{\theta}[\omega]_\times+
\frac{1-\cos\theta}{\theta^2}[\omega]_\times^2
$$

The translation uses the SO(3) left Jacobian:

$$
V(\omega)=I+\frac{1-\cos\theta}{\theta^2}[\omega]_\times+
\frac{\theta-\sin\theta}{\theta^3}[\omega]_\times^2
$$

The matrix $V$ is the coupling that is easy to miss. When $\omega=0$, $V=I$ and $t=\rho$. With rotation, the finite translation is $t=V(\omega)\rho$. Rotation and translation are therefore not six unrelated scalar slots.

Near $\theta=0$, the direct fractions lose numerical accuracy through cancellation. Use the series:

$$
R\approx I+[\omega]_\times+\tfrac12[\omega]_\times^2,
\qquad
V\approx I+\tfrac12[\omega]_\times+\tfrac16[\omega]_\times^2
$$

The logarithm reverses the construction. First recover $\omega=\operatorname{Log}(R)$, then recover $\rho$ from:

$$
\rho=V(\omega)^{-1}t
$$

For ordinary non-singular angles:

$$
V^{-1}=I-\tfrac12[\omega]_\times+
\left(\frac1{\theta^2}-\frac{1+\cos\theta}{2\theta\sin\theta}\right)
[\omega]_\times^2
$$

Near zero:

$$
V^{-1}\approx I-\tfrac12[\omega]_\times+\tfrac1{12}[\omega]_\times^2
$$

Now the earlier pose Jacobian has a precise update convention. A left update is:

$$T_{\text{new}}=\exp(\widehat{\delta\xi})T$$

A right update is:

$$T_{\text{new}}=T\exp(\widehat{\delta\xi})$$

These are not interchangeable. They express the increment in different frames, so the Jacobians differ. Any paper or implementation that says only "we optimize six pose parameters" is incomplete until it states component ordering, units, frame convention, and left-versus-right composition.

A simple hand trace makes the coupling visible. Let $\omega=(0,0,0.6)$ radians and $\rho=(1,0,0)$ length unit. The resulting translation is $V(\omega)\rho$, not simply $(1,0,0)$. The finite motion means "advance while rotating," so the displacement follows the geometry of the rigid-motion group.

This is why bundle adjustment, visual odometry, XR tracking, camera calibration, robotics, and inverse graphics commonly update a pose through a local Lie-algebra increment instead of adding values directly to a 4 by 4 matrix.

The executable witness implements Rodrigues' formula, an SO(3) logarithm for ordinary non-pi cases, V, inverse(V), and SE(3) round trips. Its code comments identify the small-angle series and the deliberate near-pi rejection. That rejection is a scope boundary: a production implementation needs a branch-aware axis recovery near pi, zero/tiny/ordinary/near-pi tests, inverse/composition checks, and analytic Jacobians compared against finite differences.
""" + se3_code + """
"""

inference = """
<!-- Bible-final-pass: research inference -->
#### A research claim needs more than a p-value

The paired t-test above is useful, but a paper-quality result needs four separate questions:

1. How large is the effect?
2. How uncertain is that estimate?
3. How surprising would the data be under a stated null model?
4. Was the experiment powerful enough to detect an effect that would actually matter?

For paired measurements, keep the per-unit difference in the original unit:

~~~text
D_i = E_A,i - E_B,i
~~~

The mean difference $\overline{D}$ is the effect in the original engineering unit. Do not throw that away. A standardized paired effect size is often:

~~~text
d_z = mean(D) / standard_deviation(D)
~~~

The standardized value helps compare scales, but it does not replace the original unit. "0.8 standard deviations" and "0.18 ms" answer different questions.

When a normal model for paired differences is doubtful, a paired sign-flip/randomization test can be useful if its design assumption is justified. Under the sharp paired null, keep each observed magnitude but flip its sign. Recompute the statistic for all sign choices, or for a large random sample of them, and measure how often the null statistic is at least as extreme as the observed one.

This is not assumption-free. Its validity comes from the sign-exchangeability or randomization argument. If the experimental design does not justify that exchangeability, the resulting p-value does not have the claimed interpretation. The code comments keep this assumption attached to the exact enumeration rather than presenting it as a generic test.

A paired bootstrap answers a different question. Resample the pairs, or equivalently their paired differences, together with replacement. Recompute the statistic on each bootstrap sample. If the design was paired, do not independently resample algorithm A and algorithm B; that destroys the covariance that made pairing useful.

Power asks whether the experiment could detect a meaningful effect. Formally:

~~~text
power = P(reject H0 | a specified alternative is true)
~~~

Power depends on the effect regarded as important, the variability, the test, the significance level, the design, and the sample size. A study that has only 20 percent power to detect the improvement it cares about cannot conclude "there is no difference" merely because p is above 0.05.

For complex graphics/XR experiments, simulation-based power analysis is often clearer than forcing a closed-form formula. Specify a realistic effect and noise model, generate many complete synthetic studies, rerun the entire analysis pipeline, and count how often the decision rule detects the effect.

Multiple comparisons add another contract. Holm's step-down procedure controls the familywise error rate without using the single blunt Bonferroni threshold for every hypothesis. False-discovery-rate procedures answer a different question. The correction strategy should be chosen before inspecting which comparison looks favorable whenever the analysis is confirmatory.

A strong result table therefore reports more than a p-value: effect in the original unit, uncertainty interval, experimental unit and sample count, pairing/blocking structure, exact test or interval construction, multiple-comparison policy, and the compute/time/memory cost needed to obtain the effect.

The executable witness below preserves pairing, computes d_z, performs an exact sign-flip test for a small sample, forms a paired bootstrap percentile interval, and applies Holm adjustment. Code comments retain the exchangeability assumption, the exponential enumeration cap, and the fact that a percentile interval is not guaranteed to have nominal coverage for every small or skewed sample.
""" + inf_code + """
"""

text = insert_or_refresh_before(
    text,
    "## Move permission is not resource transfer",
    "<!-- Bible-final-pass: C++ temporary lifetime -->",
    life,
)
text = insert_or_refresh_before(
    text,
    "## Reliable optimization in practice",
    "<!-- Bible-final-pass: SE3 exp log -->",
    se3,
)
text = insert_or_refresh_before(
    text,
    "### Likelihood turns a forward probability model into an inverse estimation rule",
    "<!-- Bible-final-pass: research inference -->",
    inference,
)

if text != NOTE.read_text():
    with tempfile.NamedTemporaryFile("w", encoding="utf-8", dir=NOTE.parent, delete=False) as handle:
        handle.write(text)
        temporary_path = Path(handle.name)
    os.replace(temporary_path, NOTE)
print("foundations final-pass patch applied")
