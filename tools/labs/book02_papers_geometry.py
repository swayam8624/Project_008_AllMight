#!/usr/bin/env python3
"""Book Two paper laboratory A: certified 2D orientation and RANSAC lines.

References (reviewed at author/publisher pages):
  Shewchuk (1997), Adaptive Precision Floating-Point Arithmetic and
  Fast Robust Geometric Predicates:
      https://www.cs.cmu.edu/~quake/robust.html
  Fischler & Bolles (1981), Random Sample Consensus:
      https://doi.org/10.1145/358669.358692

IMPORTANT ORIGINALITY / LIMIT:
This is a from-scratch pedagogical experiment, NOT a port or full performance
reproduction of Shewchuk's adaptive expansion-arithmetic predicates. A fast
floating-point determinant is compared to a conservative filter, then the
uncertain sign is found using exact rational arithmetic on the original
binary64 coordinates. This exact fallback is intentionally much slower.

RANSAC is an independently authored 2D line-estimation teaching implementation,
not the original paper's camera location determination problem or a production
estimator. Every dataset and stopping assumption is disclosed.
"""
from __future__ import annotations

from dataclasses import dataclass
from fractions import Fraction
import math
import random
import sys
from typing import Iterable

Point2 = tuple[float, float]


def _point(p: Point2) -> Point2:
    if len(p) != 2 or not all(math.isfinite(t) for t in p):
        raise ValueError("A point must contain two finite binary64 values.")
    return float(p[0]), float(p[1])


@dataclass(frozen=True)
class PredicateResult:
    sign: int
    path: str
    determinant_approx: float
    error_bound: float


def orient2d(pa: Point2, pb: Point2, pc: Point2) -> PredicateResult:
    """Return the exact orientation *sign of supplied binary64 values*.

    Translation into local differences reduces but does not remove roundoff.
    We conservatively accept the fast sign only when the determinant safely
    exceeds a scale-related error bound; otherwise Fraction.from_float gives
    an independent exact oracle on the binary inputs. No decimal-value
    inference is made about coordinates already rounded on entry.

    This is deliberately a teaching witness, not a substitute for the original
    Shewchuk expansion kernels in a high-performance geometry library.
    """
    ax, ay = _point(pa)
    bx, by = _point(pb)
    cx, cy = _point(pc)
    acx, acy, bcx, bcy = ax-cx, ay-cy, bx-cx, by-cy
    left, right = acx*bcy, acy*bcx
    det = left-right
    # Filter uses an intentionally generous factor compared with the
    # paper's adaptive sign bounds. If any intermediate is infinite,
    # skip float certification entirely and use the exact rational sign.
    scale = abs(left)+abs(right)
    bound = 64.0*sys.float_info.epsilon*scale
    if (math.isfinite(det) and math.isfinite(scale)
            and math.isfinite(bound) and abs(det) > bound):
        return PredicateResult((det > 0)-(det < 0), "filtered-float", det, bound)

    A = tuple(Fraction.from_float(v) for v in (ax, ay))
    B = tuple(Fraction.from_float(v) for v in (bx, by))
    C = tuple(Fraction.from_float(v) for v in (cx, cy))
    exact = (A[0]-C[0])*(B[1]-C[1])-(A[1]-C[1])*(B[0]-C[0])
    return PredicateResult((exact > 0)-(exact < 0), "exact-rational", det, bound)


def naive_orientation(pa: Point2, pb: Point2, pc: Point2) -> int:
    """Uncertified floating-point baseline; may overflow or cancel."""
    ax,ay = _point(pa)
    bx,by = _point(pb)
    cx,cy = _point(pc)
    d = (ax-cx)*(by-cy)-(ay-cy)*(bx-cx)
    if math.isnan(d):
        return 0  # Explicitly unsafe result, used only to show failure.
    return (d > 0)-(d < 0)


@dataclass(frozen=True)
class Line:
    """a*x+b*y+c=0 with unit Euclidean normal (a,b)."""
    a: float
    b: float
    c: float

    def distance(self, pt: Point2) -> float:
        x,y = _point(pt)
        return abs(self.a*x+self.b*y+self.c)


def line_through(p: Point2, q: Point2) -> Line | None:
    x0,y0 = _point(p)
    x1,y1 = _point(q)
    dx,dy=x1-x0,y1-y0
    norm=math.hypot(dx,dy)
    if not norm or not math.isfinite(norm):
        return None
    a,b=-dy/norm,dx/norm
    c=-(a*x0+b*y0)
    return Line(a,b,c) if all(map(math.isfinite,(a,b,c))) else None


def total_least_squares(points: Iterable[Point2]) -> Line:
    """Orthogonal 2D total least squares via covariance eigenvector angle.

    This is suitable for small clean consensus subsets, not a guarantee
    under arbitrary outliers or poorly-scaled extreme coordinates.
    """
    pts=[_point(p) for p in points]
    if len(pts)<2:
        raise ValueError("Need at least two points.")
    mx=math.fsum(p[0] for p in pts)/len(pts)
    my=math.fsum(p[1] for p in pts)/len(pts)
    xx=math.fsum((p[0]-mx)**2 for p in pts)
    yy=math.fsum((p[1]-my)**2 for p in pts)
    xy=math.fsum((p[0]-mx)*(p[1]-my) for p in pts)
    if not all(map(math.isfinite,(xx,xy,yy))) or max(xx,yy)<=0:
        raise ValueError("Degenerate or unrepresentable point covariance.")
    theta=.5*math.atan2(2*xy,xx-yy)
    a,b=-math.sin(theta),math.cos(theta)
    return Line(a,b,-a*mx-b*my)


def ransac_trials(inlier_fraction: float, sample_size: int,
                  confidence: float) -> int:
    """Idealized independent, with-replacement minimal-sample bound.

    k >= log(1-confidence)/log(1-w**m). Real pair draws without
    replacement and finite populations need separate modeling.
    """
    if not (0<inlier_fraction<=1 and 0<confidence<1) or sample_size<1:
        raise ValueError("Invalid RANSAC stopping parameters.")
    if inlier_fraction==1.:
        return 1
    chance=inlier_fraction**sample_size
    if chance>=1:
        return 1
    return max(1,math.ceil(math.log1p(-confidence)/math.log1p(-chance)))


@dataclass(frozen=True)
class RansacResult:
    line: Line
    inlier_indices: tuple[int,...]
    iterations: int
    bound_based_on_best_fraction: int
    inlier_rms: float


def ransac_line(points: list[Point2], *, threshold: float,
                confidence: float=.99, seed: int=73013,
                max_trials: int=2000, min_inliers: int=2) -> RansacResult:
    """Find the largest consensus for a 2D line, then orthogonally refit.

    Threshold is a perpendicular distance in the coordinates' units.
    Sampling uses a local seeded generator, so callers control repeatability.
    A data-dependent adaptive bound is a heuristic termination estimate,
    not a formal success certificate for an adversarial point collection.
    """
    pts=[_point(p) for p in points]
    if len(pts)<2 or not math.isfinite(threshold) or threshold<=0 or not (
        0<confidence<1) or not isinstance(max_trials,int) or max_trials<1 or (
        not isinstance(min_inliers,int) or min_inliers<2
    ):
        raise ValueError("Invalid points, threshold, or search parameters.")
    rng=random.Random(seed)
    best:tuple[int,...]=()
    best_line:Line|None=None
    best_error=math.inf
    target=max_trials
    trials=0
    while trials<min(target,max_trials):
        trials+=1
        i,j=rng.sample(range(len(pts)),2)
        candidate=line_through(pts[i],pts[j])
        if candidate is None:
            continue
        inliers=tuple(k for k,p in enumerate(pts)
                      if candidate.distance(p)<=threshold)
        error=math.fsum(candidate.distance(pts[k])**2 for k in inliers)
        if len(inliers)>len(best) or (len(inliers)==len(best)
                                    and error<best_error):
            best,best_line,best_error=inliers,candidate,error
            if best:
                w=len(best)/len(pts)
                target=min(target,ransac_trials(w,2,confidence))
    if best_line is None or len(best)<min_inliers:
        raise ValueError("No acceptable line consensus found.")
    # Refit on the discovered inliers, then recompute the *reported*
    # consensus against the refitted line. Do not report the original
    # support as though it were guaranteed to survive refinement.
    refined=total_least_squares(pts[k] for k in best)
    final=tuple(k for k,p in enumerate(pts) if refined.distance(p)<=threshold)
    if len(final)<min_inliers:
        raise ValueError("Refit failed to preserve acceptable consensus.")
    rms=math.sqrt(math.fsum(refined.distance(pts[k])**2 for k in final)/len(final))
    return RansacResult(refined,final,trials,target,rms)


def synthetic_line_data() -> tuple[list[Point2],tuple[int,...]]:
    """80 mildly noisy line points, 30 structured outliers, fixed seed.

    True model: y=2x+1; perturbation is deterministic bounded jitter.
    This teaching dataset is NOT an image dataset or a physical measurement.
    """
    rng=random.Random(20261119)
    good=[]
    for i in range(80):
        x=-4+8*i/79
        good.append((x,2*x+1+rng.uniform(-.015,.015)))
    bad=[]
    for i in range(30):
        x=-3.8+7.6*i/29
        bad.append((x,-7+0.4*x+rng.uniform(-.08,.08)))
    entries=[(p,True) for p in good]+[(p,False) for p in bad]
    rng.shuffle(entries)
    return [p for p,_ in entries],tuple(i for i,(_,ok) in enumerate(entries) if ok)
