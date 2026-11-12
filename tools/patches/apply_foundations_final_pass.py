#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
NOTE = ROOT / "Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md"

def code_block(path, language):
    return "\n\n~~~" + language + "\n" + path.read_text().rstrip() + "\n~~~\n"

def insert_before(text, anchor, marker, body):
    if marker in text:
        return text
    if anchor not in text:
        raise SystemExit("missing anchor: " + anchor)
    return text.replace(anchor, body.rstrip() + "\n\n" + anchor, 1)

text = NOTE.read_text()

life_code = code_block(ROOT / "tools/labs/temporary_lifetime_lab.cpp", "cpp")
se3_code = code_block(ROOT / "tools/labs/se3_exp_log_lab.cpp", "cpp")
inf_code = code_block(ROOT / "tools/labs/research_inference_lab.py", "python")

life = """
<!-- Bible-final-pass: C++ temporary lifetime -->
#### A temporary is still a real object with a real destruction point

C++ lets us write expressions without naming every intermediate object. That convenience can hide a lifetime. A temporary object is still an object: it comes into existence, occupies valid storage for a period, and is destroyed at a defined point. Any pointer, reference, span, iterator, string view, callback capture, or coroutine reference that observes it is useful only while the required object and storage remain valid.

Start with a safe local case:

~~~cpp
const Tracked& ref = make_tracked(42);
~~~

The returned temporary is bound directly to a local const reference. In this specific initialization context, the language extends the temporary's lifetime to the lifetime of that reference. This does not mean that references are owners, and it does not mean that every reference extends a lifetime.

Compare that with a non-owning view:

~~~cpp
std::string owner = "render_graph";
std::string_view view = owner;
~~~

The string owns its character storage. The string view only remembers an address and a length. It does not keep the string alive. If the owner disappears, or an operation invalidates the storage, the view does not become empty or repair itself; it dangles.

This is the engine rule to remember:

> Find the owner, find the destruction or invalidation point, and prove that every observer finishes before that point.

The compiler also performs temporary materialization when an expression value needs an actual object so that a reference or another operation can bind to it. The formal vocabulary matters less than the consequence: compact expression syntax can hide object creation and destruction.

A useful mental model is:

~~~text
expression
  -> may create or materialize an object
  -> that object has a lifetime
  -> observers are valid only while the required object and storage stay valid
~~~

Do not generalize the local const-reference example into "references extend lifetimes." Returning a reference to a local object is invalid. A view into a temporary buffer becomes invalid after the buffer dies. Passing an observer through another function does not automatically keep the source alive. A coroutine that suspends while holding a reference needs an explicit owner and cancellation/join policy.

The witness below runs only valid cases. The dangling-reference example is intentionally disabled because undefined behavior is not a correctness test.
""" + life_code + """
**Reconstruct without reading:** explain why the reference-bound temporary survives for the local reference, while string_view does not own its characters. Then identify the owner and invalidation point for a span, an iterator, a callback reference capture, and a coroutine reference.

**Evidence boundary:** this witness teaches the main ownership/lifetime distinction. It does not pretend to encode every temporary-lifetime corner case in the C++ standard. When a design depends on a subtle subobject, aggregate, range-for, or storage-reuse rule, verify that exact rule at the call site.
"""

se3 = """
<!-- Bible-final-pass: SE3 exp log -->
#### From a six-number pose update to an actual rigid transform

The previous section derived the tangent space of SO(3) and showed how a small rotational perturbation changes a point. One bridge was still missing: how a six-component local pose update becomes a finite rigid transform, and how a nearby rigid transform becomes a six-component update again.

A rigid transform in SE(3) contains a rotation R in SO(3) and a translation t:

~~~text
T = [ R  t ]
    [ 0  1 ]
~~~

A local six-vector is commonly written as:

~~~text
xi = [ omega ]
     [ rho   ]
~~~

omega is a three-component local rotation vector, measured in radians. rho is the translational part of the tangent coordinate. The important point is that rho is not generally equal to the final translation column t when rotation is nonzero.

The wedge form is:

~~~text
xi^ = [ [omega]x   rho ]
      [    0         0 ]
~~~

The group exponential gives:

~~~text
Exp(xi^) = [ Exp([omega]x)   V(omega) rho ]
           [       0               1      ]
~~~

Let theta = length(omega). Rodrigues' formula is:

~~~text
R = I
  + (sin(theta) / theta) [omega]x
  + ((1 - cos(theta)) / theta^2) [omega]x^2
~~~

The translation uses the SO(3) left Jacobian:

~~~text
V = I
  + ((1 - cos(theta)) / theta^2) [omega]x
  + ((theta - sin(theta)) / theta^3) [omega]x^2
~~~

This V matrix is the coupling that is easy to miss. When omega is zero, V becomes I and t equals rho. When rotation is present, the finite translation is V times rho. Rotation and translation are therefore not six unrelated scalar slots.

Near theta = 0, the direct fractions lose numerical accuracy through cancellation. Use the series:

~~~text
R approximately I + [omega]x + 1/2 [omega]x^2
V approximately I + 1/2 [omega]x + 1/6 [omega]x^2
~~~

The logarithm reverses the construction. First recover omega from Log(R), then recover rho from:

~~~text
rho = inverse(V(omega)) t
~~~

For ordinary non-singular angles:

~~~text
inverse(V) = I
           - 1/2 [omega]x
           + (1/theta^2 - (1 + cos(theta)) / (2 theta sin(theta))) [omega]x^2
~~~

Near zero:

~~~text
inverse(V) approximately I - 1/2 [omega]x + 1/12 [omega]x^2
~~~

Now the earlier pose Jacobian has a precise update convention. A left update is:

~~~text
T_new = Exp(delta_xi^) T
~~~

A right update is:

~~~text
T_new = T Exp(delta_xi^)
~~~

These are not interchangeable. They express the increment in different frames, so the Jacobians differ. Any paper or implementation that says only "we optimize six pose parameters" is incomplete until it states component ordering, units, frame convention, and left-versus-right composition.

A simple hand trace makes the coupling visible. Let omega = (0, 0, 0.6) and rho = (1, 0, 0). The resulting translation is V(omega) times rho, not simply (1, 0, 0). The finite motion means "advance while rotating," so the displacement follows the geometry of the rigid-motion group.

This is why bundle adjustment, visual odometry, XR tracking, camera calibration, robotics, and inverse graphics commonly update a pose through a local Lie-algebra increment instead of adding values directly to a 4 by 4 matrix.

The executable witness below implements Rodrigues' formula, an SO(3) logarithm for ordinary non-pi cases, V, inverse(V), and SE(3) round trips.
""" + se3_code + """
The near-pi logarithm is a real numerical branch, not a footnote. Around a rotation of pi, the simple theta/(2 sin(theta)) expression becomes delicate and the axis needs a branch-aware recovery method. The compact witness deliberately rejects that hard region instead of pretending the simple formula is universally robust.

**Reconstruct without reading:** derive Rodrigues from the exponential series, explain why V appears in translation, write the small-angle series, explain left versus right updates, and implement Exp/Log round trips. A production library must additionally test zero, tiny angles, ordinary angles, near-pi rotations, inverse/composition consistency, and analytical Jacobians against finite differences.
"""

inference = """
<!-- Bible-final-pass: research inference -->
#### A research claim needs more than a p-value

The paired t-test above is useful, but a paper-quality result needs four separate questions:

1. How large is the effect?
2. How uncertain is that estimate?
3. How surprising would the data be under a stated null model?
4. Was the experiment powerful enough to detect an effect that would actually matter?

For paired measurements, keep the per-unit difference:

~~~text
D_i = E_A,i - E_B,i
~~~

The mean difference, mean(D), is the effect in the original engineering unit. Do not throw that away. A standardized paired effect size is often:

~~~text
d_z = mean(D) / standard_deviation(D)
~~~

The standardized value helps compare scales, but it does not replace the original unit. "0.8 standard deviations" and "0.18 ms" answer different questions.

When a normal model for paired differences is doubtful, a paired sign-flip/randomization test can be useful if its design assumption is justified. Under the sharp paired null, keep each observed magnitude but flip its sign. Recompute the statistic for all sign choices, or for a large random sample of them, and measure how often the null statistic is at least as extreme as the observed one.

This is not assumption-free. Its validity comes from the sign-exchangeability or randomization argument. If the experimental design does not justify that exchangeability, the resulting p-value does not have the claimed interpretation.

A paired bootstrap answers a different question. Resample the pairs, or equivalently their paired differences, together with replacement. Recompute the statistic on each bootstrap sample. If the design was paired, do not independently resample algorithm A and algorithm B; that destroys the covariance that made pairing useful.

Power asks whether the experiment could detect a meaningful effect. Formally:

~~~text
power = P(reject H0 | a specified alternative is true)
~~~

Power depends on the effect regarded as important, the variability, the test, the significance level, the design, and the sample size. A study that has only 20 percent power to detect the improvement it cares about cannot conclude "there is no difference" merely because p is above 0.05.

For complex graphics/XR experiments, simulation-based power analysis is often clearer than forcing a closed-form formula. Specify a realistic effect and noise model, generate many complete synthetic studies, rerun the entire analysis pipeline, and count how often the decision rule detects the effect.

Multiple comparisons add another contract. Holm's step-down procedure controls the familywise error rate without using the single blunt Bonferroni threshold for every hypothesis. False-discovery-rate procedures answer a different question. The correction strategy should be chosen before inspecting which comparison looks favorable whenever the analysis is confirmatory.

A strong result table therefore reports more than a p-value: effect in the original unit, uncertainty interval, experimental unit and sample count, pairing/blocking structure, exact test or interval construction, multiple-comparison policy, and the compute/time/memory cost needed to obtain the effect.

The executable witness below preserves pairing, computes d_z, performs an exact sign-flip test for a small sample, forms a paired bootstrap percentile interval, and applies Holm adjustment.
""" + inf_code + """
**Reconstruct without reading:** given two per-scene error arrays, form paired differences, report their mean in the original unit, compute d_z, explain the null assumption behind a sign-flip test, bootstrap the pairs rather than the two marginal samples, and state the minimum effect your planned sample count was intended to detect.

**Evidence boundary:** real studies must additionally account for dependence between frames, repeated measurements on the same participant or scene, hierarchical datasets, missing data, adaptive stopping, and preregistered versus exploratory analyses. Those facts determine the statistical unit and therefore determine which inferential method is defensible.
"""

text = insert_before(
    text,
    "#### Data-oriented design begins with the loop, not with a fashionable container",
    "<!-- Bible-final-pass: C++ temporary lifetime -->",
    life,
)
text = insert_before(
    text,
    "## Reliable optimization in practice",
    "<!-- Bible-final-pass: SE3 exp log -->",
    se3,
)
text = insert_before(
    text,
    "### Likelihood turns a forward probability model into an inverse estimation rule",
    "<!-- Bible-final-pass: research inference -->",
    inference,
)

NOTE.write_text(text)
print("foundations final-pass patch applied")
