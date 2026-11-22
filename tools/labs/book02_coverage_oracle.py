#!/usr/bin/env python3
"""Book Two mathematical bridge: exact pixel coverage vs sampled coverage.

A projected 2D triangle covers a unit square by a *continuous* area fraction.
The exact rational polygon clipping here supplies a transparent oracle.
A seeded independent subpixel experiment then tests the Bernoulli estimator.

This is NOT GPU rasterization, MSAA rules, perspective interpolation, depth
testing, conservative rasterization, or a rendering performance comparison.
"""
from __future__ import annotations

from dataclasses import dataclass
from fractions import Fraction as F
from random import Random
from statistics import mean, variance

Point = tuple[F, F]
Triangle = tuple[Point, Point, Point]
EXAMPLE: Triangle = ((F(-1, 4), F(1, 4)),
                     (F(5, 4), F(1, 4)),
                     (F(1, 2), F(5, 4)))


def orient(a: Point, b: Point, c: Point) -> F:
    """Twice signed area in exact rational arithmetic."""
    return (b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0])


def _clip(poly: list[Point], axis: int, bound: F, positive: bool) -> list[Point]:
    if not poly:
        return []
    inside=lambda p: p[axis]>=bound if positive else p[axis]<=bound
    output=[]
    prev=poly[-1]
    for curr in poly:
        if inside(prev)!=inside(curr):
            denom=curr[axis]-prev[axis]
            if denom==0:
                raise ArithmeticError("Crossing edge cannot be parallel to boundary")
            t=(bound-prev[axis])/denom
            output.append((prev[0]+t*(curr[0]-prev[0]),
                           prev[1]+t*(curr[1]-prev[1])))
        if inside(curr):
            output.append(curr)
        prev=curr
    return output


def clipped_triangle(tri: Triangle) -> list[Point]:
    """Sutherland–Hodgman clipping against closed unit-square boundaries."""
    if orient(*tri)==0:
        raise ValueError("Degenerate input triangle")
    polygon=list(tri)
    for axis,bound,positive in ((0,F(0),True),(0,F(1),False),
                                (1,F(0),True),(1,F(1),False)):
        polygon=_clip(polygon,axis,bound,positive)
    return polygon


def polygon_area(poly: list[Point]) -> F:
    """Unsigned shoelace area; exact for rational vertices."""
    if len(poly)<3:
        return F(0)
    return abs(sum((a[0]*b[1]-a[1]*b[0]
                    for a,b in zip(poly,poly[1:]+poly[:1])),F(0))/2)


def exact_coverage(tri: Triangle) -> F:
    """Covered fraction because the pixel square has unit area."""
    value=polygon_area(clipped_triangle(tri))
    if not F(0)<=value<=F(1):
        raise ArithmeticError("Clipped area outside the pixel")
    return value


def inside_triangle(tri: Triangle, point: Point) -> bool:
    """Inclusive halfplanes: enough for a single continuous-area integral.

    At shared triangle edges production rasterizers need a consistent
    edge-ownership convention (such as top-left), not this closed test.
    """
    if orient(*tri)==0:
        raise ValueError("Degenerate triangle")
    s=[orient(tri[i],tri[(i+1)%3],point) for i in range(3)]
    return all(v>=0 for v in s) or all(v<=0 for v in s)


def sampled_coverage(tri: Triangle, n: int, seed: int) -> F:
    """Bernoulli subpixel sampling with a recorded pseudorandom stream."""
    if n<=0:
        raise ValueError("Sample count must be positive")
    if not isinstance(seed,int):
        raise TypeError("seed must be integer")
    if orient(*tri)==0:
        raise ValueError("Degenerate triangle")
    rng=Random(seed)
    inside=sum(inside_triangle(tri,(F.from_float(rng.random()),
                                    F.from_float(rng.random()))) for _ in range(n))
    return F(inside,n)


@dataclass(frozen=True)
class Evidence:
    exact: F
    sample_count: int
    independent_replicates: int
    mean_estimate: float
    empirical_variance: float
    theoretical_variance: float


def compare(tri: Triangle=EXAMPLE, n: int=256,
            repeats: int=40, seed: int=73013) -> Evidence:
    """Compare the observed variance against p(1-p)/n for independent trials."""
    if n<=0 or repeats<2:
        raise ValueError("At least one sample and two replicates required")
    p=exact_coverage(tri)
    observations=[float(sampled_coverage(tri,n,seed+i*104729))
                  for i in range(repeats)]
    return Evidence(p,n,repeats,mean(observations),variance(observations),
                    float(p*(1-p))/n)


if __name__=="__main__":
    result=compare()
    print("Exact clipped polygon:",clipped_triangle(EXAMPLE))
    print("Exact area:",result.exact,"=",float(result.exact))
    print("Replicates:",result.independent_replicates,
          "Samples each:",result.sample_count)
    print("Observed mean and variance:",result.mean_estimate,result.empirical_variance)
    print("Independent Bernoulli theoretical variance:",result.theoretical_variance)
