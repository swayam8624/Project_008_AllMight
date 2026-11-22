"""Book Two Pass 2: reproducible geometry-processing and optimization tests.

Source-level reproduction of core method steps, not publication figure or
runtime benchmark reproduction. SciPy is used as an independent numerical
reference; deterministic synthetic inputs avoid downloading hidden datasets.
"""
from __future__ import annotations
import math
import unittest
import numpy as np
from scipy.optimize import least_squares

from book02_papers_numeric import (
    planar_grid, fem_operators, heat_method,
    curve_model, curve_residual, curve_jacobian,
    robust_cost, damped_huber_fit, synthetic_curve, scipy_fit_reference,
)


class MeshHeatMethod(unittest.TestCase):
    def test_fem_mass_stiffness_invariants(self):
        V,F=planar_grid(9,9,.25)
        K,M,G,A=fem_operators(V,F)
        self.assertEqual(K.shape,(81,81))
        self.assertEqual(len(F),128)
        self.assertEqual(G.shape,(len(F),3,2))
        self.assertTrue(np.all(M>0))
        self.assertAlmostEqual(np.sum(M),4.,places=11)  # 2x2 m domain
        self.assertLess(np.max(np.abs((K-K.T).toarray())),1e-12)
        self.assertLess(np.max(np.abs(K@np.ones(len(V)))),1e-12)
        for seed in (1,7,101):
            y=np.random.default_rng(seed).normal(size=len(V))
            self.assertGreaterEqual(float(y@(K@y)),-1e-10)
        self.assertTrue(np.all(A>0))

    def test_exact_linear_field_energy(self):
        V,F=planar_grid(7,8,.2)
        K,M,G,A=fem_operators(V,F)
        values=V[:,0]+2*V[:,1]
        # ∫ |grad(x+2y)|² dA = 5 * domain area.
        energy=float(values@(K@values))
        area=(7-1)*.2*(8-1)*.2
        self.assertAlmostEqual(energy,5*area,places=10)

    def test_geodesic_distance_vs_flat_euclidean_oracle(self):
        n=19
        V,F=planar_grid(n,n,.12)
        src=(n//2)*n+n//2
        res=heat_method(V,F,src)
        truth=np.linalg.norm(V-V[src],axis=1)
        distance=res.distances
        self.assertEqual(distance.shape,truth.shape)
        self.assertAlmostEqual(distance[src],0.,places=12)
        self.assertTrue(np.all(np.isfinite(distance)))
        # Accuracy threshold is deliberately coarse: a one-step finite
        # element heat-method demonstration is not a production solver.
        self.assertLess(np.median(np.abs(distance-truth)),.25)
        self.assertLess(np.max(np.abs(distance-truth)),.6)
        self.assertGreater(distance[0],.6)
        self.assertGreater(np.max(distance),distance[src])
        K,M,_,_=fem_operators(V,F)
        self.assertAlmostEqual(float(M@res.heat),float(M[src]),places=9)
        self.assertEqual(len(res.vector_field),len(F))
        self.assertTrue(np.all(np.linalg.norm(res.vector_field,axis=1)<=1+1e-12))

    def test_heat_gauge_only_changes_absolute_offset(self):
        V,F=planar_grid(11,11,.15)
        src=60
        result=heat_method(V,F,src)
        self.assertEqual(result.distances[src],0.)
        # Source anchor is a coordinate convention, not an observation.
        self.assertTrue(np.isfinite(result.distances).all())

    def test_invalid_mesh_and_timestep(self):
        V,F=planar_grid(5,5)
        with self.assertRaises(ValueError):
            planar_grid(2,5)
        with self.assertRaises(ValueError):
            heat_method(V,F,-1)
        with self.assertRaises(ValueError):
            heat_method(V,F,0,time_factor=0)
        bad=V.copy();bad[0]=bad[1]
        with self.assertRaises(ValueError):
            fem_operators(bad,F)
        with self.assertRaises(ValueError):
            fem_operators(V,np.asarray([[0,1,999]],dtype=int))


class NonlinearLeastSquares(unittest.TestCase):
    def test_analytic_jacobian_against_central_differences(self):
        p=np.array([2.,-.7,.5]);x=np.linspace(0,3,17)
        analytic=curve_jacobian(p,x)
        h=1e-6
        fd=np.column_stack([
            (curve_model(p+np.eye(3)[k]*h,x)-
             curve_model(p-np.eye(3)[k]*h,x))/(2*h)
            for k in range(3)
        ])
        self.assertLess(float(np.max(np.abs(analytic-fd))),1e-8)

    def test_huber_fit_vs_scipy_and_outlier_baseline(self):
        x,y,target=synthetic_curve()
        ours=damped_huber_fit(x,y)
        ref=scipy_fit_reference(x,y)
        ordinary=least_squares(curve_residual,np.array([1.,-.2,.1]),
                               jac=curve_jacobian,args=(x,y),
                               max_nfev=5000)
        self.assertTrue(ref.success)
        self.assertTrue(np.isfinite(ours.parameters).all())
        self.assertGreater(ours.accepted_steps,2)
        self.assertLess(ours.gradient_norm,1e-4)
        self.assertLess(float(np.linalg.norm(ours.parameters-ref.x)),.06)
        self.assertLess(abs(ours.cost-ref.cost),1e-4)
        self.assertLess(float(np.linalg.norm(ours.parameters-target)),
                        float(np.linalg.norm(ordinary.x-target)))
        self.assertLess(abs(ours.parameters[1]-target[1]),.09)
        # Least-squares optimum with outlier must not be described
        # as an unbiased physical parameter estimate.
        self.assertEqual(len(x),35)

    def test_huber_definition_and_rejections(self):
        self.assertAlmostEqual(robust_cost(np.array([.5]),1.),.125)
        self.assertAlmostEqual(robust_cost(np.array([2.]),1.),1.5)
        x,y,_=synthetic_curve()
        for delta in (0.,-1.,math.nan):
            with self.assertRaises(ValueError):
                damped_huber_fit(x,y,delta=delta)
        with self.assertRaises(ValueError):
            damped_huber_fit(x[:-1],y)
        bad=y.copy();bad[0]=math.inf
        with self.assertRaises(ValueError):
            damped_huber_fit(x,bad)


if __name__=="__main__":
    unittest.main()
