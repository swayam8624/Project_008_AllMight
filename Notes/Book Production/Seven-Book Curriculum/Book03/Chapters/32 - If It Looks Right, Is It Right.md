# Chapter 32 — If It Looks Right, Is It Right?

*From Geometry to Images — Part VII: Building something we can trust*  
*Reader edition / C++23 laboratory P32*

> A beautiful picture is a wonderful reason to keep investigating. It is not a measurement of whether we were right.

## The house that lied without telling a lie

Our little paper house is back on the desk. The studio from Chapter 31 has projected its vertices into three cameras, recovered their positions and drawn the same image as the original scene. We could print the image, write a triumphant caption underneath and call the job finished.

Now imagine that someone has secretly changed the camera calibration. The second and third cameras are reported as twice as far from the first camera as they really were. The photographs are unchanged. The reconstruction algorithm trusts the reported translations.

What happens? The house becomes twice as large and twice as far away. Its pictures look exactly the same.

There is no rendering mistake. There is no algebraic contradiction. There is a mistake in what we believe the numbers mean. A computer cannot recover information that its measurements never contained, and a scientist cannot establish accuracy by measuring only something that remains unchanged under the mistake.

This is why the final chapter is not another rendering algorithm. It is a laboratory for deciding what the algorithms have actually established.

## The four questions behind a result

Verification asks whether the implementation obeys its specified mathematical or software contract. Validation asks whether the chosen model and assumptions represent the phenomenon of interest sufficiently well for the intended use. Numerical accuracy concerns the differences between computed and reference quantities. Performance concerns the resources consumed for a particular workload and environment.

These questions overlap, but they are not interchangeable. A perfect implementation of an ideal pinhole camera will not capture every optical phenomenon of a real lens. A fast renderer that drops triangles may be numerically consistent on the triangles it draws and still fail the task. A scientifically validated method can contain a software defect. A result that is reproducible may reproduce the same error.

![Four distinct questions about an algorithm.](../Assets/139-validation-questions.svg)

*Figure 32.1 — Correct implementation, suitable model, numerical error and resource cost require separate evidence.*

A useful claim names its domain. For example, Chapter 31 establishes exact coverage agreement for a controlled facade, in a declared pixel-coordinate system and three particular render passes. It does not establish exact coverage for every scene or validate a real camera. This smaller claim can be tested, defended and extended.

## A friend who calculates differently

An oracle is a way of deciding what the expected answer should be. It can be a closed-form result, an exact arithmetic predicate, an independently written reference implementation, a calibrated physical measurement or a carefully defined test fixture. Each has limits.

In Chapter 05, the exact binary64 orientation predicate evaluates the sign of the determinant of the stored coordinates. It does not recover the original real numbers lost during measurement or rounding. Chapter 18 uses integer edge functions after a specified subpixel snap. The coverage decision can be exact for that snapped grid even though the snapping changes the input geometry.

Chapter 31 uses rectangle inequalities and an analytical triangular span to predict winning pixels independently of the renderer's edge functions. It uses a separate disparity equation to check the reconstructed 3D positions. A second implementation that copies the first one's bug is not automatically an independent oracle; a different derivation and separate input path are valuable.

When an algorithm returns a matrix, compare invariant quantities as well as coefficients. A projective matrix can be multiplied by a nonzero scalar without changing its geometric predictions. A rotation represented by q and -q as unit quaternions is the same physical rotation. A mesh can be reordered without changing its surface. An oracle must respect the representation's equivalence classes.

## The test without a known answer

Sometimes we do not have a reference result for every possible input. We can still know how the answer should change when the input changes. This is metamorphic testing.

Translate all points and the camera centers by the same vector: if the orientations stay fixed and the transforms are converted consistently, the image should stay unchanged. Reorder the drawing of opaque triangles with distinct depths: visibility should not depend on submission order. Reverse the sign of a quaternion: its rotation stays the same. Rescale a scene and all camera translations together: ideal perspective measurements stay the same.

For optical world-to-camera coordinates,

\[
y_j=R_jX+t_j .
\]

If a positive factor k multiplies X and t_j, the resulting optical vector is ky_j. The perspective divide cancels that factor:

\[
\pi(Kk y_j)=\pi(Ky_j).
\tag{32.1}
\]

The P32 native test deliberately changes the assumed translations by factors of 0.5 and 2, leaving the measured pixels fixed. It triangulates the nine facade landmarks using the changed assumptions, checks that the resulting positions are respectively half and twice the original positions, and renders the new geometry. Both encoded PPM images match the original byte-for-byte. The maximum error relative to the expected scaled positions is 4.79633e-13 metres.

![The same photograph can be consistent with different metric geometry.](../Assets/140-capstone-scale.svg)

*Figure 32.2 — A global positive scale preserves perspective directions. The metric scale must come from independent evidence; equality of these images cannot establish it.*

Do not confuse this with a renderer invariant that applies to arbitrary scenes. Our diagnostic facade has flat colors, no physical light attenuation, and all the scaled vertices remain within the same near and far bounds. Metric depth and normalized depth values change, even though the winning colors do not. A renderer with distance-dependent fog or lighting might produce different colors after scaling.

A deliberately wrong calibration is useful here because we know how the error should propagate. This is not a failure of the triangulator: it correctly solves the wrong physical problem. If we had hidden the original metric positions, the image could not expose the mistake.

## Tolerance is a statement about a quantity

Floating-point comparisons should identify the quantity, its units and expected error scale. For two finite scalar results a and b, a useful mixed tolerance rule is

\[
|a-b|\le \tau_a+\tau_r\max(|a|,|b|).
\tag{32.2}
\]

Absolute tolerance tau_a carries the units of the quantity. Relative tolerance tau_r is dimensionless. Near zero the absolute term prevents an arbitrarily small denominator from dominating the decision. At large magnitudes the relative term allows a proportionate numerical error.

Neither tolerance should be selected just because it makes a failing test green. Use an error analysis, representation precision, a reference uncertainty or a task requirement to choose it before looking at the result. Compare signed orientation and index validity exactly when the contract makes them discrete; a tolerance on an invalid index is meaningless.

The native capstone uses an absolute 1e-10 metre tolerance for the small-scale synthetic reconstruction invariant, and separately compares the projection Jacobian against a centered finite difference with step h=1e-5 metres. For a coordinate direction e_i,

\[
J_i\approx \frac{\pi(X+h e_i)-\pi(X-h e_i)}{2h}.
\tag{32.3}
\]

For sufficiently smooth projection the truncation error is of order h squared, while subtractive rounding errors can grow as h becomes tiny. The best step depends on scale and conditioning. Our bounded nine-point test measured maximum component difference 2.08502e-9 pixels per metre, within the declared 1e-7 threshold. This tests the analytical derivative in the chosen fixture, not every skew, orientation or near-singular configuration.

A failed numerical check may reveal a bug, a weak problem, a reference mistake or a tolerance that was not justified. Investigate which. Do not round away the difference and declare success.

## A smaller residual can conceal a worse world

Consider two identical aligned cameras with focal scale f=800 pixels. The scene points lie at Z=4 metres. The corrected horizontal disparity is

\[
d=fB/Z .
\]

B is the camera baseline in metres. With independent coordinate errors of standard deviation sigma pixels in both images, the disparity error has standard deviation sqrt(2) sigma. Differentiating Z=fB/d gives

\[
\sigma_Z\approx \frac{Z^2}{fB}\sqrt{2}\sigma.
\tag{32.4}
\]

This is a first-order approximation under the rectified, known-calibration and independent-noise assumptions. It is not a confidence bound for disparity approaching zero or for wrong correspondences. The reciprocal mapping can introduce skew and bias in the recovered depth.

Now look at the image-space fitting objective. In the ideal rectified pair, the two horizontal observations can be fit exactly by selecting X and Z when disparity is positive. The two vertical observations must share one predicted vertical coordinate. With equal independent noise, the least-squares prediction is their mean. If the measured vertical difference is Delta v, the sum of the two squared vertical residuals is (Delta v)^2/2. The squared RMS per observation is consequently (Delta v)^2/4. Its population root-mean-square value is sigma/sqrt(2), independent of B.

This is an analytical explanation of why similar image-space residuals can accompany radically different depth errors. They measure different aspects of the same problem.

## The final capstone experiment

We keep the focal scale, scene depth and noise distribution fixed. We vary one factor: the stereo baseline. The baseline choices are 0.05, 0.10, 0.20, 0.40 and 0.80 metres. Each trial generates 200 scene points at four metres, with X and Y sampled uniformly from [-1,1] metres. Each point receives four independent standard-normal draws, scaled by sigma=0.25 pixels for the two coordinates in each view.

The exact same points and noise draws are reused at each baseline within the trial. This pairing helps isolate the effect of baseline rather than comparing unrelated random samples. There are twenty independent trial groups generated from the seeded pseudorandom stream, producing one hundred trial-baseline summaries, 20,000 attempted point reconstructions and 40,000 camera-point observations. The implementation calls the Chapter 29 triangulation and point-refinement functions without changing them.

The model counts rejected results separately. The summary uses zero-initialized numerical fields when no point is accepted, so such a row's error fields are undefined, not perfect zero errors. Inspect the accepted count before interpreting the metrics. In the observed sweep all 20,000 points were accepted. An independent unit test rejects identical zero-baseline rays. The experiment is not a validation of all degenerate configurations.

For N accepted landmarks define depth RMSE as the root mean square of Z error, and position RMSE as the root mean square of Euclidean 3D error:

\[
e_Z=\sqrt{\frac1N\sum_i(\hat Z_i-Z_i)^2},\qquad
e_X=\sqrt{\frac1N\sum_i\|\hat X_i-X_i\|^2}.
\tag{32.5}
\]

Both are in metres. Pixel RMS is the root mean square of the Euclidean pixel residual across the 2N observations. Do not average per-coordinate RMS and call it per-observation RMS. Report the aggregation explicitly: our table uses the arithmetic mean of twenty per-trial RMS values, not the pooled RMS over all 4,000 points at a baseline.

| Baseline (m) | Mean depth RMSE (mm) | Mean 3D RMSE (mm) | Mean reprojection RMS (px) | Predicted depth sigma (mm) |
|---:|---:|---:|---:|---:|
| 0.05 | 143.505 | 146.381 | 0.177192 | 141.421 |
| 0.10 | 71.417 | 72.854 | 0.177192 | 70.711 |
| 0.20 | 35.663 | 36.401 | 0.177192 | 35.355 |
| 0.40 | 17.825 | 18.240 | 0.177192 | 17.678 |
| 0.80 | 8.911 | 9.214 | 0.177192 | 8.839 |

![Depth error decreases with baseline while reprojection RMS stays almost constant.](../Assets/141-capstone-baseline.svg)

*Figure 32.3 — Twenty paired trial groups, 200 points per baseline per group. The dotted prediction is a first-order population standard deviation. The two panels use different units.*

The depth RMSE improved by roughly sixteen times between the smallest and largest baseline. The pixel residual did not reveal that improvement. The first-order prediction is close to the measured depth error in this particular noise regime, with a slightly larger deviation at the smallest baseline. This is consistent with increased nonlinearity of the inverse disparity map, but the experiment does not isolate all sources of the finite-sample difference.

A larger real baseline is not always preferable. It can reduce overlap, increase occlusion and make correspondence harder. We held the associations correct and the camera intrinsics and poses exact. Our result is a controlled geometric sensitivity result, not a universal camera design recommendation.

## Performance is an experiment, too

Suppose we optimize the triangulator by changing its linear solver, adding SIMD or moving the work to a GPU. A timing comparison must use the same input, output contract and accuracy requirements. Dropping rejected points, using a looser precision policy, omitting allocation costs or comparing a warmed optimized build with a cold debug build can produce an impressive but misleading speedup.

Define the measurement boundary. Kernel time, frame time and end-to-end time answer different questions. GPU submission may return before work has finished, so synchronize appropriately when measuring completed work. On a CPU, compiler optimization, thermal state, frequency, thread scheduling and caches can influence timing.

Use warmup, repeated paired trials, a stable workload and a declared aggregation. For paired positive times t_ref and t_new, report per-case ratios t_ref/t_new; a geometric mean is useful for a multiplicative summary, but also show the individual cases and total elapsed time. A geometric mean speedup is not necessarily the total-runtime speedup. A workload with many easy cases can conceal the few cases that consume most runtime.

If you measure latency, report a distribution or relevant quantiles, not only a mean. If you change an accuracy threshold, show the accuracy-performance tradeoff rather than pretending the workload is identical. For a research result, separate tuning data from the held-out evaluation and record all exclusions.

The P32 sweep deliberately does not report performance results. It was compiled with sanitization for correctness checks, and no controlled timing protocol was executed. A future optimized version needs its own release-mode performance evaluation against the same numerical and failure contracts.

## Keep the evidence near the claim

Reproducibility begins with mundane details: a commit ID, exact source files, compiler version, flags, input generation and seed, configuration, test command and raw output. An experiment should also preserve failure cases, units, and the rule used to aggregate results. A hash checks identity of bytes, not correctness of their contents. A fixed pseudorandom seed does not imply identical draws across every implementation of the C++ standard library's distribution algorithms.

![An evidence chain from declared experiment to checked conclusion.](../Assets/142-capstone-evidence.svg)

*Figure 32.4 — A claim should be traceable to data, a configuration, a verifier and a defined interpretation.*

A good chart does not mix metres with pixels on an unlabeled axis. It does not crop away failed trials, use a decorative curve to imply more observations than were measured, or compare a noisy residual with a noiseless reference without explaining the difference. If a zoomed axis is useful, label it. Report uncertainty where it is justified and preserve raw values for inspection.

The capstone executable writes the complete 100 summary rows to a CSV. It does not export the individual random draws or raw observation tracks, so the saved summaries alone cannot independently replicate every fit. Another implementation can check the expected relationship and recompute the aggregation; rerunning the exact source on the same implementation provides a stronger repeatability check. Cross-platform equivalence should use tolerances and declared invariants rather than assume bitwise identical random streams.

Run the final native workshop from the compiled companion:

~~~text
book03_capstone_tests
book03_capstone capstone.csv
~~~

The build registration adds a separate contract test. For a full local cumulative verification, from the book03_cpp23 source directory use:

~~~sh
cmake -S . -B build/book03-final -DCMAKE_BUILD_TYPE=Release
cmake --build build/book03-final --parallel
ctest --test-dir build/book03-final --output-on-failure
~~~

These are commands for the reader to run, not a statement that all cumulative tests have already passed. The final publication gate must also check the native frontend, diagrams, links, equation layout, code blocks, navigation, accessibility and original-source custody. A complete manuscript and a ready-to-publish edition are different milestones.

## The final project: make a claim that survives questions

Choose a modest reconstruction task. Start with the controlled facade, then add a known noncoplanar scene, calibrated cameras, noise and deliberate missing observations. Specify the coordinate conventions and data contracts. Implement the forward projection, triangulation and rendering using the modules developed throughout the book. Measure a physically meaningful error against a known reference, alongside the pixel residual and failure rate.

Next, create an independently computed oracle for a restricted case. Construct metamorphic tests for frame changes, scale, submission order and representation equivalence. Add an adversarial degenerate case. Save the generating input and the result, and document why the test should pass or fail. If an assumption is unavailable, report it rather than borrowing certainty from another metric.

Finally, prepare a short evidence dossier: claim, domain, algorithm, experiment, raw data, aggregate, failure cases, reproduction command and limitations. A useful dossier can say that a method failed. It is not a sales brochure. Its success is that a second reader can understand and challenge the conclusion.

## Exercises — finish the book by trying to prove it wrong

**Reconstruct 32A.** Derive the scale invariance in equation 32.1. Construct two camera-and-scene configurations that produce the same image but have different metric dimensions.

**Reconstruct 32B.** Derive the first-order depth uncertainty in equation 32.4 and the ideal rectified per-observation pixel RMS from the vertical residual.

**Falsify 32C.** Change only one camera's translation scale in a three-view fixture instead of all translations. Predict whether the same global similarity can still explain all observations.

**Falsify 32D.** Vary finite-difference step size over several powers of ten and compare the numerical derivative against the analytical Jacobian. Identify the rounding-dominated and truncation-dominated regimes.

**Transfer 32E.** Extend the baseline experiment to near-zero disparity, mismatches, calibration error and unequal noise. Report rejection rate and depth-error quantiles, including failures rather than substituting zero.

**Transfer 32F.** Run an optimized and a reference implementation on identical paired workloads. Check the output contract before measuring performance. Record warmups, repeated times, individual ratios and total elapsed time.

**Explain 32G.** Choose a figure from an earlier chapter and audit its units, axis labels, assumptions, reference data and caption. Identify what conclusion the figure cannot establish.

**Capstone 32H.** Deliver the evidence dossier above, including an example where a convincing image hides an incorrect physical quantity. Explain the discrepancy to a child first and then to a reviewer who can inspect the source.

## Where we go from here

Thirty-two chapters ago, a pixel was just a little colored window. Now we can explain why a triangle appears at a particular address, why some of its edges must be included and others excluded, why a camera bends a ray, why depth is difficult to infer, and why a beautiful surface can be unsupported by measurements.

We built the mathematical language, the forward image pipeline and a controlled path back to geometry. We have also learned where these teaching systems stop. The next books will make the world richer: illumination and materials, simulation, hardware-aware rendering and research. Those topics build upon the geometry and evidence contracts we established here.

If you remember one thing from this book, remember that understanding is not the ability to repeat an equation. It is the ability to make a prediction, implement it, observe the result, discover where the prediction fails and explain why.

The little house is still on the desk. You can now decide what its image tells you, what it does not, and how to find out more.
