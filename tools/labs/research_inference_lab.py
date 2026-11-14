#!/usr/bin/env python3
"""Small deterministic inference witness for paired graphics/engine experiments.

It intentionally uses only the Python standard library.  The goal is to make
the research contracts executable: preserve pairing, report an effect size,
obtain a randomization p-value without a normality assumption, build a paired
bootstrap interval, and correct a family of p-values with Holm's procedure.
"""

from __future__ import annotations
import itertools
import math
import random
import statistics


def paired_differences(a: list[float], b: list[float]) -> list[float]:
    # Position i must name the same scene/run/block in both conditions; unequal
    # or empty arrays do not define the requested paired comparison.
    if len(a) != len(b) or not a:
        raise ValueError("paired samples must be non-empty and have equal length")
    if any(not math.isfinite(x) for x in a + b):
        raise ValueError("paired observations must be finite")
    differences = [x - y for x, y in zip(a, b)]
    if any(not math.isfinite(x) for x in differences):
        raise ValueError("paired differences overflowed the finite range")
    return differences


def finite_differences(d: list[float]) -> list[float]:
    """Validate and normalize differences without changing their signs or ratios."""
    if not d or any(not math.isfinite(x) for x in d):
        raise ValueError("paired differences must be non-empty and finite")
    scale = max(abs(x) for x in d)
    return [x / scale for x in d] if scale else [0.0] * len(d)


def cohen_dz(d: list[float]) -> float:
    # Standardize the mean paired change by its sample spread. Constant effects
    # need an explicit result because division by zero is otherwise undefined.
    if len(d) < 2:
        raise ValueError("need at least two paired differences")
    normalized = finite_differences(d)
    sd = statistics.stdev(normalized)
    if sd == 0.0:
        # Reporting convention for a degenerate denominator: no changes report
        # zero, while a constant nonzero change flags signed infinity. This is
        # not a finite effect-size estimate; preserve the improvement's sign.
        mean = statistics.fmean(normalized)
        return math.copysign(math.inf, mean) if mean != 0.0 else 0.0
    return statistics.fmean(normalized) / sd


def exact_sign_flip_pvalue(d: list[float]) -> float:
    """Two-sided exact randomization test under sign-exchangeability.

    Under the sharp paired null, each difference may have either sign with
    equal probability.  This is exact only when that sign-flip null is a
    defensible design/model assumption.
    """
    normalized = finite_differences(d)
    n = len(normalized)
    # Exact enumeration costs 2**n; fail clearly above this teaching cap.
    if n > 20:
        raise ValueError("exact enumeration is intentionally capped at n<=20")
    observed = abs(math.fsum(normalized))
    # Compare dimensionless signed sums. A few local ULPs cover only addition
    # roundoff; unlike a fixed epsilon, this tolerance does not depend on the
    # measurement unit or the magnitude of the original observations.
    extreme = 0
    total = 0
    for signs in itertools.product((-1.0, 1.0), repeat=n):
        candidate = abs(math.fsum(s * x for s, x in zip(signs, normalized)))
        tolerance = 4.0 * max(math.ulp(observed), math.ulp(candidate))
        extreme += candidate >= observed or observed - candidate <= tolerance
        total += 1
    return extreme / total


def paired_bootstrap_mean_ci(
    d: list[float], *, confidence: float = 0.95, draws: int = 20_000, seed: int = 7
) -> tuple[float, float]:
    if not (0.0 < confidence < 1.0):
        raise ValueError("confidence must lie in (0,1)")
    if draws < 100:
        raise ValueError("use enough bootstrap draws to estimate quantiles")
    normalized = finite_differences(d)
    rng = random.Random(seed)
    n = len(normalized)
    scale = max(abs(x) for x in d)
    # Resampling differences is equivalent to resampling intact pairs and
    # retains the covariance that independent marginal resampling would lose.
    means = []
    for _ in range(draws):
        # One bootstrap draw contains n sampled indices, with replacement.
        means.append(statistics.fmean(normalized[rng.randrange(n)] for _ in range(n)))
    means.sort()
    # Percentile order statistics are an interval construction, not a universal
    # small-sample coverage guarantee.
    alpha = 1.0 - confidence
    lo = means[int((alpha / 2.0) * (draws - 1))]
    hi = means[int((1.0 - alpha / 2.0) * (draws - 1))]
    return lo * scale, hi * scale


def holm_adjust(pvalues: list[float]) -> list[float]:
    """Holm step-down adjusted p-values preserving original order."""
    # Invalid inputs should not turn into plausible-looking adjusted values.
    if any(not math.isfinite(p) or not 0.0 <= p <= 1.0 for p in pvalues):
        raise ValueError("p-values must be finite and lie in [0,1]")
    m = len(pvalues)
    order = sorted(range(m), key=pvalues.__getitem__)
    adjusted = [0.0] * m
    running = 0.0
    for rank, idx in enumerate(order):
        # The running maximum enforces monotone step-down adjusted p-values.
        candidate = (m - rank) * pvalues[idx]
        running = max(running, candidate)
        adjusted[idx] = min(1.0, running)
    return adjusted


def expect_value_error(operation, *args) -> None:
    """Make negative-contract checks survive python -O."""
    try:
        operation(*args)
    except ValueError:
        return
    raise RuntimeError(f"{operation.__name__} accepted invalid input")


def main() -> None:
    # Same six scenes, same measurement units, same pairing.
    baseline = [10.4, 8.8, 12.1, 9.7, 11.5, 10.0]
    proposed = [9.8, 8.3, 11.2, 9.5, 10.7, 9.2]
    d = paired_differences(baseline, proposed)

    mean_gain = statistics.mean(d)
    effect = cohen_dz(d)
    p = exact_sign_flip_pvalue(d)
    ci = paired_bootstrap_mean_ci(d)

    require(mean_gain > 0.0, "paired mean should favor proposed method")
    require(effect > 1.0, "synthetic paired effect should be large")
    require(0.0 <= p <= 1.0, "randomization p-value lies in [0,1]")
    require(ci[0] > 0.0, "synthetic paired interval should exclude zero")

    raw = [0.004, 0.018, 0.041, 0.20]
    adj = holm_adjust(raw)
    require(all(a >= p0 for a, p0 in zip(adj, raw)), "adjustment cannot lower p-values")
    require(adj[0] <= adj[1] <= adj[2] <= adj[3], "sorted Holm results are monotone")

    # Scaling an engineering measurement changes its unit, not its exact
    # sign-flip inference. Normalization plus relative ULP ties preserves p.
    for unit_scale in (1e-20, 1.0, 1e20):
        scaled = [value * unit_scale for value in d]
        require(exact_sign_flip_pvalue(scaled) == p,
                "sign-flip p-value changed when only the measurement unit changed")

    # Negative contracts demonstrate that invalid pair shapes and p-values
    # fail loudly rather than yielding a result that looks statistically valid.
    invalid_calls = (
        (paired_differences, ([1.0], [])),
        (paired_differences, ([math.nan], [0.0])),
        (paired_differences, ([math.inf], [0.0])),
        (paired_differences, ([1e308], [-1e308])),
        (cohen_dz, ([1.0, math.nan],)),
        (cohen_dz, ([1.0, math.inf],)),
        (exact_sign_flip_pvalue, ([],)),
        (exact_sign_flip_pvalue, ([math.nan],)),
        (exact_sign_flip_pvalue, ([math.inf],)),
        (paired_bootstrap_mean_ci, ([math.nan],)),
        (paired_bootstrap_mean_ci, ([math.inf],)),
        (holm_adjust, ([1.1],)),
    )
    for operation, args in invalid_calls:
        expect_value_error(operation, *args)

    # Constant differences have no estimated spread. Check both directions so
    # an all-negative effect cannot accidentally be reported as improvement.
    require(cohen_dz([2.0, 2.0]) == math.inf, "positive degenerate effect sign")
    require(cohen_dz([-2.0, -2.0]) == -math.inf, "negative degenerate effect sign")
    require(cohen_dz([0.0, 0.0]) == 0.0, "zero-change reporting convention")

    print(f"paired mean improvement: {mean_gain:.6f}")
    print(f"Cohen dz: {effect:.6f}")
    print(f"exact sign-flip p-value: {p:.6f}")
    print(f"95% paired bootstrap percentile CI: [{ci[0]:.6f}, {ci[1]:.6f}]")
    print("Holm adjusted:", ", ".join(f"{x:.6f}" for x in adj))
    print("research inference: paired, finite, scale, degenerate, interval and Holm checks PASS")


def require(condition: bool, message: str) -> None:
    # Explicit checks survive python -O, which removes assert statements.
    if not condition:
        raise RuntimeError(message)


if __name__ == "__main__":
    main()
