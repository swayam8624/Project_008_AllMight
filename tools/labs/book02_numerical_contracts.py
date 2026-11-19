#!/usr/bin/env python3
"""Independent numerical oracle for Book Two's analytic teaching figures.

No external dependencies. These deterministic calculations are reference
checks for authored equations, NOT a measured GPU/CPU performance study.
Run with python3 and python3 -O: the checks intentionally do not use assert.
"""
from __future__ import annotations

import json
import math


def check(test: bool, message: str) -> None:
    if not test:
        raise RuntimeError(message)


def close(a: float, b: float, tol: float = 1e-11) -> bool:
    return math.isclose(a, b, abs_tol=tol, rel_tol=tol)


def solve_2x2(a: float, b: float, c: float, d: float,
              u: float, v: float) -> tuple[float, float]:
    """Teaching-only 2x2 solve with explicit rank/finite rejection.

    This calculation is NOT a general-purpose backward-stable solver;
    use a validated QR/SVD implementation for ill-conditioned matrices.
    """
    if not all(math.isfinite(z) for z in (a, b, c, d, u, v)):
        raise ValueError("nonfinite linear-system input")
    s = max(abs(a), abs(b), abs(c), abs(d))
    if not s:
        raise ValueError("zero matrix")
    aa, bb, cc, dd = (z / s for z in (a, b, c, d))
    det = aa*dd - bb*cc
    if not math.isfinite(det) or abs(det) <= 32*math.ulp(1.0):
        raise ValueError("numerically singular 2x2 matrix")
    x = (dd*(u/s) - bb*(v/s))/det
    y = (aa*(v/s) - cc*(u/s))/det
    if not math.isfinite(x) or not math.isfinite(y):
        raise ValueError("unrepresentable solution")
    return x, y


def linear_transformation(theta: float) -> tuple[tuple[float, float],
                                                   tuple[float, float]]:
    if not math.isfinite(theta):
        raise ValueError("nonfinite angle")
    x, y = math.cos(theta), math.sin(theta)
    # Plate 01: A = [[2,0.6],[0,0.45]].
    return (x, y), (2*x+0.6*y, 0.45*y)


def gaussian(x: float, y: float) -> float:
    if not (math.isfinite(x) and math.isfinite(y)):
        raise ValueError("nonfinite Gaussian coordinates")
    # Plate 02: normal distribution with sigma_x=1.2, sigma_y=0.55.
    return math.exp(-0.5*((x/1.2)**2+(y/0.55)**2)) / (
        2*math.pi*1.2*0.55)


def gradient_descent(alpha: float, n: int) -> list[tuple[float, float, float]]:
    """Plate 03: f=(x-1)^2+4(y+1/2)^2, start=(-1,1)."""
    if not math.isfinite(alpha) or alpha < 0 or type(n) is not int or n < 0 or n > 10000:
        raise ValueError("invalid gradient-descent arguments")
    x, y, result = -1.0, 1.0, []
    for k in range(n+1):
        f = (x-1)**2 + 4*(y+0.5)**2
        if not math.isfinite(f):
            raise ValueError("unrepresentable objective")
        result.append((x, y, f))
        if k < n:
            x -= alpha*2*(x-1)
            y -= alpha*8*(y+0.5)
    return result


def heat_mode(x: float, y: float, t: float) -> float:
    """Plate 04: exact zero-Dirichlet heat eigenmode, kappa=1."""
    if not all(map(math.isfinite, (x, y, t))) or not (
        0 <= x <= 1 and 0 <= y <= 1 and t >= 0
    ):
        raise ValueError("heat-domain input outside exact solution")
    return math.exp(-2*math.pi**2*t)*math.sin(math.pi*x)*math.sin(math.pi*y)


def forward_euler_error(n: int) -> float:
    """Plate 05: absolute error for y'=-y at T=1, n equal steps."""
    if type(n) is not int or n < 2:
        raise ValueError("n must be an integer >=2")
    return abs((1-1/n)**n-math.exp(-1))


def condition_numbers(delta: float) -> tuple[float, float]:
    """Closed-form 2-norm condition numbers of A and A^T A.

    A=[[1,1],[1,1+delta]], det(A)=delta. Small eigenvalue is computed
    using the determinant product to avoid subtractive cancellation.
    """
    if not math.isfinite(delta) or not 0 < delta < 0.5:
        raise ValueError("delta outside witness domain")
    tr = 4+2*delta+delta**2
    high = (tr+math.sqrt(tr*tr-4*delta*delta))/2
    low = delta**2/high
    cond = math.sqrt(high/low)
    return cond, high/low


def metric_gradient() -> tuple[tuple[float, float], tuple[float, float]]:
    """Local differential (2,7) and metric gradient (1/6,2/3).

    A=[[2,1],[0,3]], G=A^T A=[[4,2],[2,10]];
    the local metric gradient solves Gg=A^T(1,2).
    """
    local = solve_2x2(4, 2, 2, 10, 2, 7)
    check(close(2*local[0]+local[1], 1) and
          close(3*local[1], 2), "world/local gradient transport")
    return (2., 7.), local


def expect_invalid(fn, *args) -> None:
    try:
        fn(*args)
    except ValueError:
        return
    raise RuntimeError(f"{fn.__name__} accepted invalid inputs")


def run() -> dict:
    # Verify that the depicted basis vectors equal the exact matrix columns.
    check(linear_transformation(0)[1] == (2., 0.), "transformed first axis")
    ux, uy = linear_transformation(math.pi/2)[1]
    check(close(ux, .6) and close(uy, .45), "transformed second axis")
    for theta in (0.1, .8, 1.9, 5.4):
        src, dst = linear_transformation(theta)
        recovered = solve_2x2(2, .6, 0, .45, *dst)
        check(all(close(x,y) for x,y in zip(src,recovered)), "round-trip circle")

    covector, gradient = metric_gradient()
    check(covector == (2.,7.) and close(gradient[0],1/6)
          and close(gradient[1],2/3), "covector is not the gradient vector")

    c, cn = condition_numbers(.001)
    check(4000 < c < 4010 and close(cn,c*c,1e-7),
          "normal equations must square conditioning")
    # At delta=.001, a +1e-4 change to the second observation
    # changes the exact solution (2,-1) to (1.9,-.9).
    changed = solve_2x2(1,1,1,1.001,1,1-.001+.0001)
    check(all(close(x,y) for x,y in zip(changed,(1.9,-.9))),
          "sensitivity example")

    peak = gaussian(0,0)
    check(close(peak, 1/(2*math.pi*1.2*.55)), "Gaussian peak")
    check(gaussian(1,1) < peak, "Gaussian density shape")
    # Analytical probability in +-6 x +-3 from independent normal CDFs.
    prob_x = math.erf(6/(math.sqrt(2)*1.2))
    prob_y = math.erf(3/(math.sqrt(2)*.55))
    check(prob_x*prob_y > .999999, "normalization/support")

    steps = gradient_descent(.1,4)
    expected = [(-1,1,13),(-.6,-.2,2.92),(-.28,-.44,1.6528),
                (-.024,-.488,1.049152),(.1808,-.4976,.67111168)]
    for actual,wanted in zip(steps,expected):
        check(all(close(x,y) for x,y in zip(actual,wanted)),
              "descent coordinates must match plot")
    check(close(abs(1-8*.2),.6) and abs(1-8*.3)>1,
          "descent stability examples")

    t=.04
    heat_peak=heat_mode(.5,.5,t)
    check(close(heat_peak,math.exp(-2*math.pi**2*t)),
          "heat peak from exact eigenmode")
    check(close(heat_mode(0,.5,t),0) and close(heat_mode(.5,.5,0),1),
          "Dirichlet and initial conditions")
    # Independently differentiate numerically rather than comparing
    # a symbolic expression to itself.
    x,y,h,dt=.37,.44,5e-4,1e-5
    u=heat_mode(x,y,t)
    ut=(heat_mode(x,y,t+dt)-heat_mode(x,y,t-dt))/(2*dt)
    uxx=(heat_mode(x+h,y,t)-2*u+heat_mode(x-h,y,t))/h**2
    uyy=(heat_mode(x,y+h,t)-2*u+heat_mode(x,y-h,t))/h**2
    check(abs(ut-uxx-uyy)<1e-5, "numerical heat PDE residual")

    rates=[math.log(forward_euler_error(n)/
                    forward_euler_error(2*n), 2) for n in (8,16,32,64,128)]
    check(all(.9<r<1.15 for r in rates),
          "observed first-order convergence")
    check(all(forward_euler_error(n)>forward_euler_error(2*n)
              for n in (2,4,8,16,32,64,128)), "error decreases")

    for fn,args in ((solve_2x2,(1,1,1,1,0,0)),
                    (solve_2x2,(math.inf,1,1,1,0,0)),
                    (linear_transformation,(math.nan,)),
                    (gaussian,(math.nan,0)),
                    (gradient_descent,(math.inf,3)),
                    (heat_mode,(.3,.3,-1)),
                    (forward_euler_error,(1,)),
                    (forward_euler_error,(2.5,)),
                    (condition_numbers,(-.1,))):
        expect_invalid(fn,*args)
    return {
        "status":"PASS", "plates_checked":5,
        "condition_A":c, "condition_AtA":cn,
        "gaussian_peak":peak, "heat_peak_t0_04":heat_peak,
        "forward_euler_slopes":rates,
        "coordinate_gradient":gradient,
    }


if __name__=="__main__":
    print(json.dumps(run(),indent=2))
