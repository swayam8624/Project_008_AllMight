#!/usr/bin/env python3
"""One planar P1 triangle: energy stability does not imply positivity.

Original teaching witness, not a full mesh solver. Standard library only.
"""
import json
import math


def triangle_stiffness(points):
    """Return positive-convention stiffness and area for a finite 2D triangle."""
    if len(points) != 3 or any(len(p) != 2 for p in points):
        raise ValueError("expected three planar vertices")
    if not all(math.isfinite(v) for p in points for v in p):
        raise ValueError("nonfinite coordinate")
    (x0, y0), (x1, y1), (x2, y2) = points
    det = (x1-x0)*(y2-y0) - (x2-x0)*(y1-y0)
    if not math.isfinite(det) or det == 0:
        raise ValueError("unrepresentable or degenerate triangle")
    area = abs(det)/2
    gradients = [((y1-y2)/det, (x2-x1)/det),
                 ((y2-y0)/det, (x0-x2)/det),
                 ((y0-y1)/det, (x1-x0)/det)]
    stiffness = [[area*sum(a*b for a, b in zip(gi, gj))
                  for gj in gradients] for gi in gradients]
    if area == 0 or not all(math.isfinite(v) for row in stiffness for v in row):
        raise ValueError("unrepresentable area or stiffness")
    return stiffness, area


def multiply(matrix, vector):
    return [sum(a*b for a, b in zip(row, vector)) for row in matrix]


def energy(matrix, vector):
    return sum(a*b for a, b in zip(vector, multiply(matrix, vector)))/2


def require(condition, message):
    if not condition:
        raise RuntimeError(message)


def main():
    vertices = [(0., 0.), (2., 0.), (.5, .5)]
    stiffness, area = triangle_stiffness(vertices)
    expected = [[1.25, .25, -1.5], [.25, .25, -.5], [-1.5, -.5, 2.]]
    require(stiffness == expected and area == .5, "independent matrix trace")
    require(multiply(stiffness, [1., 1., 1.]) == [0., 0., 0.], "constant mode")
    for field in ([1., 0., 0.], [0., 1., 0.], [0., 0., 1.], [1., -2., 3.]):
        require(energy(stiffness, field) >= 0, "selected energy witnesses")
    # Uniform scaling leaves 2D P1 stiffness unchanged but scales mass as area.
    scaled, scaled_area = triangle_stiffness([(3*x, 3*y) for x, y in vertices])
    require(all(math.isclose(a, b) for r, s in zip(stiffness, scaled)
                for a, b in zip(r, s)) and scaled_area == 9*area, "scale law")
    reversed_s, _ = triangle_stiffness([vertices[i] for i in [0, 2, 1]])
    require(reversed_s == [[expected[i][j] for j in [0, 2, 1]]
                          for i in [0, 2, 1]], "orientation permutation")
    try:
        triangle_stiffness([(0., 0.), (1., 0.), (2., 0.)])
    except ValueError:
        pass
    else:
        raise RuntimeError("degenerate triangle accepted")
    old = [0., 1., 0.]
    mass, step = area/3, .01  # diffusivity = 1 m^2/s; step in seconds
    new = [u-step*su/mass for u, su in zip(old, multiply(stiffness, old))]
    expected_new = [-.015, .985, .03]
    require(all(math.isclose(a, b) for a, b in zip(new, expected_new)), "step trace")
    require(min(new) < 0 and energy(stiffness, new) < energy(stiffness, old),
            "energy decrease with positivity violation")
    require(math.isclose(sum(new), sum(old)), "mass-weighted mean conservation")
    print(json.dumps({"stiffness": stiffness, "area": area, "new": new,
                      "energy_before": energy(stiffness, old),
                      "energy_after": energy(stiffness, new), "result": "PASS"}))


if __name__ == "__main__":
    main()
