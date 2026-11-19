"""Adversarial coverage contracts: analytical geometry vs independent sampling."""
from fractions import Fraction as F
import unittest

from book02_coverage_oracle import (
    EXAMPLE, clipped_triangle, polygon_area, exact_coverage, orient,
    inside_triangle, sampled_coverage, compare,
)


class ExactPixelCoverage(unittest.TestCase):
    def test_hand_derived_polygon_and_area(self):
        poly=clipped_triangle(EXAMPLE)
        self.assertEqual(poly, [
            (F(5,16),F(1)), (F(0),F(7,12)),
            (F(0),F(1,4)), (F(1),F(1,4)),
            (F(1),F(7,12)), (F(11,16),F(1)),
        ])
        self.assertEqual(polygon_area(poly),F(119,192))
        self.assertEqual(exact_coverage(EXAMPLE),F(119,192))

    def test_full_half_and_zero_coverage(self):
        tri_full=((F(-1),F(-1)),(F(3),F(-1)),(F(-1),F(3)))
        tri_half=((F(0),F(0)),(F(1),F(0)),(F(0),F(1)))
        tri_out=((F(2),F(2)),(F(3),F(2)),(F(2),F(3)))
        self.assertEqual(exact_coverage(tri_full),F(1))
        self.assertEqual(exact_coverage(tri_half),F(1,2))
        self.assertEqual(exact_coverage(tri_out),F(0))

    def test_orientation_reversal_and_vertex_rotation_invariance(self):
        tri=EXAMPLE
        self.assertEqual(orient(*tri),-orient(tri[0],tri[2],tri[1]))
        for trial in (tri,(tri[0],tri[2],tri[1]),
                      (tri[1],tri[2],tri[0])):
            self.assertEqual(exact_coverage(trial),F(119,192))

    def test_barycentric_edge_definition_is_declared(self):
        half=((F(0),F(0)),(F(1),F(0)),(F(0),F(1)))
        self.assertTrue(inside_triangle(half,(F(1,2),F(1,2))))
        self.assertFalse(inside_triangle(half,(F(1),F(1))))

    def test_degenerate_and_bad_sample_arguments_rejected(self):
        bad=((F(0),F(0)),(F(1),F(1)),(F(2),F(2)))
        with self.assertRaises(ValueError):
            exact_coverage(bad)
        with self.assertRaises(ValueError):
            sampled_coverage(EXAMPLE,0,12)
        with self.assertRaises(TypeError):
            sampled_coverage(EXAMPLE,5,3.5)
        with self.assertRaises(ValueError):
            compare(EXAMPLE,10,1)

    def test_sampling_has_repeatable_but_nontrivial_variation(self):
        a=sampled_coverage(EXAMPLE,128,20261120)
        b=sampled_coverage(EXAMPLE,128,20261120)
        self.assertEqual(a,b)
        self.assertGreaterEqual(a,F(0))
        self.assertLessEqual(a,F(1))
        e=compare(EXAMPLE,512,60,73013)
        self.assertEqual(e.exact,F(119,192))
        self.assertAlmostEqual(e.theoretical_variance,
             float(e.exact*(1-e.exact))/512)
        self.assertLess(abs(e.mean_estimate-float(e.exact)),.04)
        self.assertGreater(e.empirical_variance,0)
        self.assertLess(e.empirical_variance,.005)


if __name__=="__main__":
    unittest.main()
