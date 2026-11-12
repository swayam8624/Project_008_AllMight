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
    if len(a) != len(b) or not a:
        raise ValueError("paired samples must be non-empty and have equal length")
    return [x - y for x, y in zip(a, b)]


def cohen_dz(d: list[float]) -> float:
    if len(d) < 2:
        raise ValueError("need at least two paired differences")
    sd = statistics.stdev(d)
    if sd == 0.0:
        return math.inf if statistics.mean(d) != 0.0 else 0.0
    return statistics.mean(d) / sd


def exact_sign_flip_pvalue(d: list[float]) -> float:
    """Two-sided exact randomization test under sign-exchangeability.

    Under the sharp paired null, each difference may have either sign with
    equal probability.  This is exact only when that sign-flip null is a
    defensible design/model assumption.
    """
    n = len(d)
    if n > 20:
        raise ValueError("exact enumeration is intentionally capped at n<=20")
    observed = abs(statistics.mean(d))
    extreme = 0
    total = 0
    for signs in itertools.product((-1.0, 1.0), repeat=n):
        candidate = abs(statistics.mean(s * x for s, x in zip(signs, d)))
        extreme += candidate >= observed - 1e-15
        total += 1
    return extreme / total


def paired_bootstrap_mean_ci(
    d: list[float], *, confidence: float = 0.95, draws: int = 20_000, seed: int = 7
) -> tuple[float, float]:
    if not (0.0 < confidence < 1.0):
        raise ValueError("confidence must lie in (0,1)")
    if draws < 100:
        raise ValueError("use enough bootstrap draws to estimate quantiles")
    rng = random.Random(seed)
    n = len(d)
    means = []
    for _ in range(draws):
        means.append(statistics.mean(d[rng.randrange(n)] for _ in range(n)))
    means.sort()
    alpha = 1.0 - confidence
    lo = means[int((alpha / 2.0) * (draws - 1))]
    hi = means[int((1.0 - alpha / 2.0) * (draws - 1))]
    return lo, hi


def holm_adjust(pvalues: list[float]) -> list[float]:
    """Holm step-down adjusted p-values preserving original order."""
    m = len(pvalues)
    order = sorted(range(m), key=pvalues.__getitem__)
    adjusted = [0.0] * m
    running = 0.0
    for rank, idx in enumerate(order):
        candidate = (m - rank) * pvalues[idx]
        running = max(running, candidate)
        adjusted[idx] = min(1.0, running)
    return adjusted


def main() -> None:
    # Same six scenes, same measurement units, same pairing.
    baseline = [10.4, 8.8, 12.1, 9.7, 11.5, 10.0]
    proposed = [9.8, 8.3, 11.2, 9.5, 10.7, 9.2]
    d = paired_differences(baseline, proposed)

    mean_gain = statistics.mean(d)
    effect = cohen_dz(d)
    p = exact_sign_flip_pvalue(d)
    ci = paired_bootstrap_mean_ci(d)

    assert mean_gain > 0.0
    assert effect > 1.0
    assert 0.0 <= p <= 1.0
    assert ci[0] > 0.0

    raw = [0.004, 0.018, 0.041, 0.20]
    adj = holm_adjust(raw)
    assert all(a >= p0 for a, p0 in zip(adj, raw))
    assert adj[0] <= adj[1] <= adj[2] <= adj[3]

    print(f"paired mean improvement: {mean_gain:.6f}")
    print(f"Cohen dz: {effect:.6f}")
    print(f"exact sign-flip p-value: {p:.6f}")
    print(f"95% paired bootstrap percentile CI: [{ci[0]:.6f}, {ci[1]:.6f}]")
    print("Holm adjusted:", ", ".join(f"{x:.6f}" for x in adj))
    print("research inference: 5 suites PASS")


if __name__ == "__main__":
    main()
