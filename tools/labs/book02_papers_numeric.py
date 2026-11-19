#!/usr/bin/env python3
"""Book Two paper laboratory B: nonlinear fitting and mesh heat distances.

Source methods read:
  Crane, Weischedel & Wardetzky (2013), Geodesics in Heat:
      https://www.cs.cmu.edu/~kmcrane/Projects/HeatMethod/
  libigl tutorial, cotangent Laplacian/mass-matrix conventions:
      https://libigl.github.io/tutorial/
  Ceres Solver, nonlinear least squares and robust loss formulation:
      https://ceres-solver.readthedocs.io/latest/nnls_modeling.html

This is a small, from-scratch educational implementation of the core
mathematics, NOT the authors' code or reproduction of their published
benchmark tables, mesh datasets or paper figures. It uses NumPy/SciPy as
independently inspectable reference numerical linear algebra.

Coordinate conventions: 2D right-handed Euclidean plane, units of metres.
Stiffness K = -integrated discrete Laplacian (positive semidefinite).
Diagonal lumped mass M is positive. Source gauge phi[source] = 0.
"""
from __future__ import annotations

from dataclasses import dataclass
import math

import numpy as np
from scipy import sparse
from scipy.sparse.linalg import spsolve
from scipy.optimize import least_squares


def planar_grid(nx: int, ny: int, spacing: float=0.2):
    """Triangulate rectangular grid, same diagonal per square."""
    if not (isinstance(nx,int) and isinstance(ny,int) and
            3<=nx<=80 and 3<=ny<=80 and np.isfinite(spacing) and spacing>0):
        raise ValueError("invalid grid dimensions or spacing")
    V=np.array([(i*spacing,j*spacing) for j in range(ny)
                for i in range(nx)],dtype=np.float64)
    F=[]
    for j in range(ny-1):
        for i in range(nx-1):
            a=j*nx+i; b=a+1; c=a+nx; d=c+1
            F.extend([(a,b,d),(a,d,c)])
    return V,np.asarray(F,dtype=np.int64)


def fem_operators(V: np.ndarray,F: np.ndarray):
    """Assemble finite element K, lumped M, and each face basis gradient.

    In each triangle, the P1 basis gradient is constant.
    K_ij += area * grad(N_i) dot grad(N_j).
    The row sum must vanish for a connected free-boundary mesh.
    """
    if V.ndim!=2 or V.shape[1]!=2 or F.ndim!=2 or F.shape[1]!=3:
        raise ValueError("Require planar V[n,2] and F[m,3].")
    n=len(V)
    if n==0 or not np.isfinite(V).all() or (
        F.size and (np.min(F)<0 or np.max(F)>=n)
    ):
        raise ValueError("Invalid mesh input or vertex references.")
    rows=[];cols=[];data=[]
    mass=np.zeros(n); grads=[]; areas=[]
    for ids in F:
        p=V[ids]
        twice=(p[1,0]-p[0,0])*(p[2,1]-p[0,1])-(p[1,1]-p[0,1])*(p[2,0]-p[0,0])
        if not np.isfinite(twice) or abs(twice)<1e-14:
            raise ValueError("Degenerate or numerically unrepresentable triangle")
        area=abs(twice)/2
        g=np.array([
            [(p[1,1]-p[2,1])/twice,(p[2,0]-p[1,0])/twice],
            [(p[2,1]-p[0,1])/twice,(p[0,0]-p[2,0])/twice],
            [(p[0,1]-p[1,1])/twice,(p[1,0]-p[0,0])/twice]
        ])
        mass[ids]+=area/3
        k=area*(g@g.T)
        for a in range(3):
            for b in range(3):
                rows.append(int(ids[a]));cols.append(int(ids[b]));data.append(k[a,b])
        grads.append(g);areas.append(area)
    if not len(areas) or np.any(mass<=0):
        raise ValueError("Mesh contains isolated vertices or no faces")
    K=sparse.coo_matrix((data,(rows,cols)),shape=(n,n)).tocsr()
    return K,mass,np.asarray(grads),np.asarray(areas)


@dataclass(frozen=True)
class HeatResult:
    distances: np.ndarray
    heat: np.ndarray
    vector_field: np.ndarray
    mean_edge_length: float
    divergence_rhs: np.ndarray


def heat_method(V: np.ndarray,F: np.ndarray,source: int,
                time_factor: float=1.0) -> HeatResult:
    """One backward heat step + normalized negative face gradient + Poisson.

    On a planar triangulation, the analytical Euclidean distance from a
    source is a useful independent oracle for error measurements.
    Boundary shape matters; the center of a convex rectangular domain
    has straight shortest paths to every interior point.
    """
    if not isinstance(source,int) or not 0<=source<len(V):
        raise ValueError("invalid source index")
    if not np.isfinite(time_factor) or time_factor<=0:
        raise ValueError("invalid heat-time factor")
    K,mass,grads,areas=fem_operators(V,F)
    edges=np.vstack((F[:,[0,1]],F[:,[1,2]],F[:,[2,0]]))
    h=float(np.mean(np.linalg.norm(V[edges[:,0]]-V[edges[:,1]],axis=1)))
    t=time_factor*h*h
    u0=np.zeros(len(V));u0[source]=1.0
    u=spsolve(sparse.diags(mass)+t*K,mass*u0)
    grad_u=np.einsum("fij,fi->fj",grads,u[F])
    mag=np.linalg.norm(grad_u,axis=1)
    X=np.zeros_like(grad_u)
    valid=mag>1e-14
    X[valid]=-grad_u[valid]/mag[valid,None]

    # Weak Poisson equation K * phi = ∫ grad(N_i) dot X dA
    rhs=np.zeros(len(V))
    for ids,g,area,field in zip(F,grads,areas,X):
        np.add.at(rhs,ids,area*(g@field))
    # Gauge fixing removes constant nullspace but adds no observation.
    free=np.ones(len(V),dtype=bool);free[source]=False
    phi=np.zeros(len(V))
    phi[free]=spsolve(K[free][:,free],rhs[free])
    # Minor discretization artifacts may create negative values; report
    # them rather than silently clipping into an unrealistically perfect map.
    if not all(np.isfinite(arr).all() for arr in (u,X,rhs,phi)):
        raise RuntimeError("nonfinite heat-method results")
    return HeatResult(phi,u,X,h,rhs)


def curve_model(p, x):
    a,b,c=np.asarray(p)
    return a*np.exp(b*x)+c


def curve_residual(p,x,y):
    return curve_model(p,x)-y


def curve_jacobian(p,x):
    a,b,c=np.asarray(p)
    z=np.exp(b*x)
    return np.column_stack([z,a*x*z,np.ones_like(x)])


def robust_cost(r,delta:float) -> float:
    """0.5 Huber rho(r**2) cost, consistent with SciPy least_squares."""
    a=np.abs(r)
    return float(np.sum(np.where(a<=delta,.5*r*r,
                                 delta*(a-.5*delta))))


@dataclass(frozen=True)
class FitResult:
    parameters: np.ndarray
    cost: float
    iterations: int
    accepted_steps: int
    gradient_norm: float
    stopped_by: str


def damped_huber_fit(x:np.ndarray,y:np.ndarray,
                     p0=(1.,-.2,.1),delta=.12,max_iter=180) -> FitResult:
    """Levenberg-like IRLS with explicit acceptance/rollback.

    Solve (J^T W J + lambda*I) step=-J^T W r; accept a step only if
    it lowers the specified Huber objective, otherwise increase lambda.
    This is a compact teaching solver, not a production Ceres equivalent.
    """
    x=np.asarray(x,dtype=float);y=np.asarray(y,dtype=float)
    p=np.asarray(p0,dtype=float)
    if (x.ndim!=1 or y.shape!=x.shape or len(x)<4 or p.shape!=(3,)
        or not all(map(np.isfinite,(delta,max_iter))) or delta<=0
        or not np.isfinite(x).all() or not np.isfinite(y).all()
        or not np.isfinite(p).all()):
        raise ValueError("invalid curve fitting input")
    damping=1e-2
    accepted=0
    criterion="iteration-cap"
    for k in range(1,max_iter+1):
        r=curve_residual(p,x,y)
        J=curve_jacobian(p,x)
        w=np.ones_like(r)
        nz=np.abs(r)>delta
        w[nz]=delta/np.abs(r[nz])
        grad=J.T@(w*r)
        gn=float(np.linalg.norm(grad))
        if gn<1e-10:
            criterion="gradient"
            break
        matrix=J.T@(w[:,None]*J)+damping*np.eye(3)
        step=np.linalg.solve(matrix,-grad)
        trial=p+step
        old_cost=robust_cost(r,delta)
        trial_cost=robust_cost(curve_residual(trial,x,y),delta)
        if np.isfinite(trial_cost) and trial_cost<old_cost:
            p=trial
            accepted+=1
            damping=max(1e-12,damping/2)
            if np.linalg.norm(step)<1e-10*(1+np.linalg.norm(p)):
                criterion="step"
                break
        else:
            damping=min(1e12,damping*8)
            if damping==1e12:
                criterion="damping-limit"
                break
    else:
        k=max_iter
    # Recalculate at final parameters, not stale trial Jacobian.
    residual=curve_residual(p,x,y)
    final_J=curve_jacobian(p,x)
    final_weights=np.minimum(1.,delta/np.maximum(np.abs(residual),1e-300))
    final_gradient=float(np.linalg.norm(final_J.T@(final_weights*residual)))
    return FitResult(p,robust_cost(residual,delta),k,
                     accepted,final_gradient,criterion)


def synthetic_curve():
    """Generated, not observed: a=2, b=-0.7, c=0.5 with one gross outlier."""
    x=np.linspace(0.,4.,35)
    target=np.array([2.,-.7,.5])
    y=curve_model(target,x)+.01*np.sin(5*x)
    y[13]+=2.5
    return x,y,target


def scipy_fit_reference(x,y,delta=.12):
    """Independent trusted reference: scipy.optimize.least_squares."""
    return least_squares(curve_residual,np.array([1.,-.2,.1]),
                         jac=curve_jacobian,args=(x,y),
                         loss="huber",f_scale=delta,
                         max_nfev=5000,xtol=1e-12,gtol=1e-12,ftol=1e-12)
