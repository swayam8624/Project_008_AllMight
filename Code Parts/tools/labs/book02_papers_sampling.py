#!/usr/bin/env python3
"""Book Two paper laboratory C: balance-heuristic multiple importance sampling.

Sources:
 Eric Veach & Leonidas Guibas, "Optimally Combining Sampling Techniques
 for Monte Carlo Rendering" (SIGGRAPH 1995)
 https://lightfield.stanford.edu/papers/combine/
 Eric Veach, "Robust Monte Carlo Methods for Light Transport Simulation"
 (1997 thesis), chapter 9
 https://graphics.stanford.edu/papers/veach_thesis/

This is an independently written one-dimensional demonstration of the
*multi-sample balance heuristic*, NOT a light-transport renderer or
reproduction of paper image-variance benchmarks.

Target: I = integral_0^1 exp(x) dx = e - 1, exact.
Two proposals: p1(x)=1 and p2(x)=x+1/2 on [0,1].
Both have strictly positive support, and both integrate to 1.
With n samples from each, the balanced estimator is
 (1/(2n)) sum_{samples x from both proposals} exp(x)/m(x)
where m(x)=(p1(x)+p2(x))/2=0.75+0.5x.

The proposal choice and estimator must be stated together; the same
x coordinate density under different measures is NOT interchangeable.
"""
from __future__ import annotations

from dataclasses import dataclass
import math
import random
import statistics


def q1(x:float)->float:
    if not math.isfinite(x):
        raise ValueError("nonfinite x")
    return 1. if 0.<=x<=1. else 0.


def q2(x:float)->float:
    if not math.isfinite(x):
        raise ValueError("nonfinite x")
    return .5+x if 0.<=x<=1. else 0.


def inverse_q2(u:float)->float:
    """Exact inverse CDF of .5x+.5x^2 on [0,1]."""
    if not math.isfinite(u) or not 0<=u<1:
        raise ValueError("u must be a finite variate in [0,1).")
    return (math.sqrt(1+8*u)-1)/2


@dataclass(frozen=True)
class SampleEstimate:
    uniform: float
    q2_importance: float
    balance_mis: float
    count_per_proposal: int
    seed: int


def estimate(n:int,seed:int)->SampleEstimate:
    if type(n) is not int or not 1<=n<=1_000_000 or type(seed) is not int:
        raise ValueError("invalid sample count or seed")
    rng=random.Random(seed)
    uniform=[rng.random() for _ in range(n)]
    tilted=[inverse_q2(rng.random()) for _ in range(n)]
    # Uniform strategy and a second importance sampler individually estimate
    # the exact same integral, with different sampling variation.
    p1=sum(math.exp(x)/q1(x) for x in uniform)/n
    p2=sum(math.exp(x)/q2(x) for x in tilted)/n
    # Multi-sample balance weight in the equal-proposal-count case.
    mixed=sum(math.exp(x)/(.75+.5*x) for x in uniform+tilted)/(2*n)
    return SampleEstimate(p1,p2,mixed,n,seed)


def replication_study(*,replicates:int=100,n:int=200,seed:int=10000)->dict:
    """Independent random streams across replicates, not extra samples.

    Repeating with the *same* seed would produce identical pseudorandom
    outcomes and cannot estimate a sampling distribution.
    """
    if type(replicates) is not int or not 3<=replicates<=10000:
        raise ValueError("invalid replicate count")
    estimates=[estimate(n,seed+i) for i in range(replicates)]
    truth=math.e-1
    result={"true_integral":truth,
            "replicates":replicates,"samples_per_proposal":n,
            "first_seed":seed,"method":{}}
    for key in ("uniform","q2_importance","balance_mis"):
        samples=[getattr(s,key) for s in estimates]
        result["method"][key]={
            "replicate_mean":statistics.fmean(samples),
            "sample_variance":statistics.variance(samples),
            "mean_error":statistics.fmean(samples)-truth
        }
    return result
