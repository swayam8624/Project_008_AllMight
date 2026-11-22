"""Book Two Pass 2: tested educational reproduction of paper ideas.

These checks certify our mathematical examples, *not* original-paper
benchmark reproduction or equivalence to Shewchuk's expansion arithmetic.
"""
from __future__ import annotations
import math
import random
import unittest

from book02_papers_geometry import (
    orient2d, naive_orientation, line_through, total_least_squares,
    ransac_trials, ransac_line, synthetic_line_data
)


class RobustPredicates(unittest.TestCase):
    def test_exact_sign_and_zero(self):
        self.assertEqual(orient2d((0,0),(3,0),(0,2)).sign,1)
        self.assertEqual(orient2d((0,0),(0,2),(3,0)).sign,-1)
        flat=orient2d((0,0),(1,1),(2,2))
        self.assertEqual(flat.sign,0)
        self.assertEqual(flat.path,"exact-rational")

    def test_overflow_cancellation_naive_failure(self):
        top=1e308
        # This is a finite binary64 triple whose exact determinant has
        # a nonzero negative sign, despite overflow in float products.
        points=((0.,0.),(top,top),(.9*top,math.nextafter(.9*top,0.)))
        exact=orient2d(*points)
        self.assertEqual(exact.sign,-1)
        self.assertEqual(exact.path,"exact-rational")
        self.assertEqual(naive_orientation(*points),0)

    def test_nearly_collinear_with_exact_binary64_reference(self):
        p=(0.,0.)
        q=(1.,1.)
        r=(2.,math.nextafter(2.,math.inf))
        self.assertEqual(orient2d(p,q,r).sign,1)
        self.assertEqual(orient2d(p,r,q).sign,-1)

    def test_symmetry_and_translation(self):
        rng=random.Random(76213)
        for _ in range(150):
            pts=[(rng.randint(-100,100),rng.randint(-100,100))
                 for _ in range(3)]
            a,b,c=pts
            s=orient2d(a,b,c).sign
            self.assertEqual(s,-orient2d(a,c,b).sign)
            shift=(10000.,-20000.)
            translated=[(p[0]+shift[0],p[1]+shift[1]) for p in pts]
            self.assertEqual(s,orient2d(*translated).sign)

    def test_nonfinite_inputs_rejected(self):
        for invalid in (math.nan,math.inf,-math.inf):
            with self.assertRaises(ValueError):
                orient2d((0,invalid),(1,2),(3,4))

    def test_exact_input_not_unrounded_intended_decimal(self):
        self.assertEqual(orient2d((0.,0.),(.1,.1),(.2,.2)).sign,0)
        # The contract concerns actual stored binary64 values; decimal
        # real-number assumptions require a separate input model.


class RobustConsensus(unittest.TestCase):
    def test_minimal_model_degeneracy(self):
        self.assertIsNone(line_through((1,1),(1,1)))
        with self.assertRaises(ValueError):
            total_least_squares([(0,0),(0,0)])
        with self.assertRaises(ValueError):
            ransac_line([(0,0),(0,0)],threshold=.1)

    def test_stopping_math(self):
        self.assertEqual(ransac_trials(1.,2,.99),1)
        n=ransac_trials(.5,2,.99)
        self.assertEqual(n,17)
        self.assertLessEqual((1-.25)**n,.01)
        with self.assertRaises(ValueError):
            ransac_trials(0.,2,.99)
        with self.assertRaises(ValueError):
            ransac_trials(.4,2,1.)

    def test_seeded_outlier_resistance(self):
        points,truth=synthetic_line_data()
        fit=ransac_line(points,threshold=.08,seed=73013)
        estimated_slope=-fit.line.a/fit.line.b
        estimated_intercept=-fit.line.c/fit.line.b
        self.assertLess(abs(estimated_slope-2.),.03)
        self.assertLess(abs(estimated_intercept-1.),.07)
        self.assertGreaterEqual(len(set(fit.inlier_indices)&set(truth)),75)
        self.assertLessEqual(len(set(fit.inlier_indices)-set(truth)),4)
        self.assertLess(fit.inlier_rms,.08)
        self.assertLessEqual(fit.iterations,2000)

        # All-data TLS is deliberately contaminated by structured outliers.
        all_data=total_least_squares(points)
        all_slope=-all_data.a/all_data.b
        self.assertGreater(abs(all_slope-2.),abs(estimated_slope-2.))

        fit_again=ransac_line(points,threshold=.08,seed=73013)
        self.assertEqual(fit,fit_again)

    def test_no_hidden_success_on_bad_parameters(self):
        pts=[(0.,0.),(1.,1.),(2.,2.)]
        for threshold in (0.,-1.,math.nan):
            with self.assertRaises(ValueError):
                ransac_line(pts,threshold=threshold)
        with self.assertRaises(ValueError):
            ransac_line(pts,threshold=.1,min_inliers=10)
        with self.assertRaises(ValueError):
            ransac_line(pts,threshold=.1,max_trials=0)


if __name__=="__main__":
    unittest.main()
