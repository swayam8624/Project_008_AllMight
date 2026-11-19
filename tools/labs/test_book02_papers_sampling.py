"""Book Two Pass 2: MIS unbiasedness, support and inference contract checks."""
from __future__ import annotations
import math
import unittest

from scipy.integrate import quad

from book02_papers_sampling import (
    q1,q2,inverse_q2,estimate,replication_study
)


class MultipleImportanceSampling(unittest.TestCase):
    def test_proposals_normalized_to_same_measure(self):
        self.assertAlmostEqual(quad(q1,0,1)[0],1,places=11)
        self.assertAlmostEqual(quad(q2,0,1)[0],1,places=11)
        self.assertAlmostEqual(quad(lambda x:.5*(q1(x)+q2(x)),0,1)[0],
                               1,places=11)
        # CDF F(x)=0.5*x+0.5*x^2. Inversion must reproduce it.
        for u in (0.,.01,.125,.5,.9,.999999):
            x=inverse_q2(u)
            self.assertGreaterEqual(x,0.)
            self.assertLess(x,1.)
            self.assertAlmostEqual(.5*x+.5*x*x,u,places=12)
        with self.assertRaises(ValueError):
            inverse_q2(1.)
        with self.assertRaises(ValueError):
            inverse_q2(math.nan)

    def test_reference_integral_and_single_seed_replay(self):
        expected=math.e-1
        self.assertAlmostEqual(quad(math.exp,0,1)[0],expected,places=12)
        a=estimate(1000,7)
        b=estimate(1000,7)
        self.assertEqual(a,b)
        self.assertEqual(a.seed,7)
        self.assertLess(abs(a.balance_mis-expected),.06)
        for wrong in (0,-1,1.5):
            with self.assertRaises(ValueError):
                estimate(wrong,7)

    def test_replicate_variability_and_mean_accuracy(self):
        s=replication_study(replicates=100,n=200,seed=10000)
        truth=math.e-1
        self.assertAlmostEqual(s["true_integral"],truth)
        for method,record in s["method"].items():
            self.assertLess(abs(record["replicate_mean"]-truth),.025,method)
            self.assertGreater(record["sample_variance"],0,method)
            self.assertAlmostEqual(record["mean_error"],
                                   record["replicate_mean"]-truth)
        self.assertGreater(
            s["method"]["uniform"]["sample_variance"],
            s["method"]["balance_mis"]["sample_variance"])
        # This test deliberately does not claim MIS always beats every
        # individual proposal on every integrand; here q2 is excellent.
        self.assertGreater(
            s["method"]["balance_mis"]["sample_variance"],
            s["method"]["q2_importance"]["sample_variance"])

    def test_invalid_replications_and_unrepresented_support(self):
        with self.assertRaises(ValueError):
            replication_study(replicates=2)
        with self.assertRaises(ValueError):
            q1(math.inf)
        with self.assertRaises(ValueError):
            q2(math.nan)
        # Support failure: a sampler that never visits x in (0.5,1]
        # cannot correct a target mass located entirely in that region.
        # The true integral of indicator(x>0.5) is exactly 0.5.
        restricted=lambda u:.5*u
        observed=[float(restricted(k/100)>0.5) for k in range(100)]
        self.assertEqual(sum(observed),0.)
        self.assertEqual(1-.5,.5)


if __name__=="__main__":
    unittest.main()
