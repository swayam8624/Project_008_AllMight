#!/usr/bin/env python3
"""Planar anchor Jacobian/impulse and motor-bound witness; not a world solver."""
import math
import random


def require(condition, message):
    # Remains active with python -O; a lesson check must not silently disappear.
    if not condition:
        raise AssertionError(message)


def rotate(angle, point):
    c, s = math.cos(angle), math.sin(angle)
    return c*point[0]-s*point[1], s*point[0]+c*point[1]


def constraint_and_rows(q, anchors):
    # q stores (COM x, COM y, angle) for A followed by B. Each anchor is
    # measured from its body's LOCAL COM, not an unrelated mesh pivot.
    if not all(math.isfinite(v) for v in (*q, *anchors[0], *anchors[1])):
        raise ValueError('Finite body poses and local anchors required')
    ra = rotate(q[2], anchors[0])
    rb = rotate(q[5], anchors[1])
    error = (q[3]+rb[0]-q[0]-ra[0], q[4]+rb[1]-q[1]-ra[1])
    if not all(math.isfinite(v) for v in error):
        raise ValueError("Anchor separation overflow")
    # d(R*a)/d(angle) = (-r_y,r_x). The negative A contribution reverses
    # those signs. Rows map generalized body velocity into anchor velocity.
    rows = ((-1., 0., ra[1], 1., 0., -rb[1]),
            (0., -1., -ra[0], 0., 1., rb[0]))
    return error, rows


def solve_symmetric_two(k00, k01, k11, rhs):
    # Both rows constrain distance, so they have compatible units. Normalize
    # to avoid forming an unscaled determinant. This tolerance is an explicit
    # singularity policy, not a general condition estimator or sparse solver.
    if not all(math.isfinite(v) for v in (k00, k01, k11, *rhs)):
        raise ValueError("Finite matrix and right-hand side required")
    scale = max(abs(k00), abs(k01), abs(k11))
    if not math.isfinite(scale) or scale == 0:
        raise ValueError('Both-fixed or invalid constraint block')
    a, b, d = k00/scale, k01/scale, k11/scale
    determinant = a*d-b*b
    if a <= 0 or d <= 0 or determinant <= 1e-12:
        raise ValueError('Singular, redundant or ill-conditioned anchor rows')
    u, v = rhs[0]/scale, rhs[1]/scale
    result = ((d*u-b*v)/determinant, (a*v-b*u)/determinant)
    if not all(math.isfinite(x) for x in result):
        raise ValueError('Unrepresentable impulse')
    return result


def solve_anchor(q, anchors, inverse_mass, velocity, target):
    if not all(math.isfinite(w) and w >= 0 for w in inverse_mass):
        raise ValueError('Nonnegative finite inverse masses/inertias required')
    if not all(math.isfinite(v) for v in (*velocity, *target)):
        raise ValueError('Finite velocities required')
    _, rows = constraint_and_rows(q, anchors)
    # K = J M^-1 J^T. The angular inverse inertia occupies the third component
    # of each body block, so lever-arm effects enter without a special case.
    def k(i, j):
        return sum(rows[i][n]*inverse_mass[n]*rows[j][n] for n in range(6))
    current = tuple(sum(row[n]*velocity[n] for n in range(6)) for row in rows)
    impulse = solve_symmetric_two(k(0, 0), k(0, 1), k(1, 1),
                                 tuple(target[i]-current[i] for i in range(2)))
    delta = tuple(inverse_mass[n]*sum(rows[i][n]*impulse[i] for i in range(2))
                  for n in range(6))
    # This applies a velocity correction only. Position integration, contacts,
    # warm starting, sleep, breakage and frame-to-frame joint state are absent.
    return tuple(velocity[n]+delta[n] for n in range(6)), impulse


def bias_velocity(error, beta, step, speed_cap):
    if not (math.isfinite(step) and step > 0 and 0 <= beta <= 1
            and math.isfinite(speed_cap) and speed_cap >= 0):
        raise ValueError('Positive step, valid beta and finite nonnegative cap required')
    if not all(math.isfinite(c) for c in error):
        raise ValueError("Finite positional error required")
    target = tuple(-beta*c/step for c in error)
    if not all(math.isfinite(c) for c in target):
        raise ValueError("Bias arithmetic overflow before capping")
    length = math.hypot(*target)
    factor = min(1., speed_cap/length) if length else 1.
    return tuple(x*factor for x in target)


def motor_update(previous, increment, max_torque, step):
    if not all(math.isfinite(x) for x in (previous, increment, max_torque, step)):
        raise ValueError('Finite motor data required')
    if max_torque < 0 or step <= 0:
        raise ValueError('Motor cap nonnegative and timestep positive required')
    cap = step*max_torque
    if not math.isfinite(cap):
        raise ValueError("Motor cap overflow")
    total = max(-cap, min(cap, previous+increment))
    # Bound the ACCUMULATED impulse. The corrective increment can exceed cap
    # when it cancels a previously applied impulse with the opposite sign.
    return total, total-previous


def run():
    q = (0., 0., 0., .02, -.51, 0.)
    anchors = ((0., 0.), (0., .5))
    weights = (0., 0., 0., .5, .5, 2.)
    velocity = (0., 0., 0., 1., -.5, 0.)
    error, rows = constraint_and_rows(q, anchors)
    target = bias_velocity(error, .2, 1/60, 10.)
    after, impulse = solve_anchor(q, anchors, weights, velocity, target)
    require(all(math.isclose(x, y, abs_tol=1e-12)
                for x, y in zip(impulse, (-1.24, 1.24))), 'Worked hinge impulse')
    require(math.isclose(after[5], 1.24, abs_tol=1e-12), 'Angular response')
    actual = tuple(sum(row[i]*after[i] for i in range(6)) for row in rows)
    require(all(math.isclose(x, y, abs_tol=1e-12) for x, y in zip(actual, target)),
            'Post-solve anchor speed equals bias target')

    # Swapping A/B reverses error and row impulse, but preserves each actual
    # body's resulting velocity. This catches a body-order sign mistake.
    swapped, _ = solve_anchor(q[3:]+q[:3], anchors[::-1], weights[3:]+weights[:3],
                              velocity[3:]+velocity[:3], tuple(-x for x in target))
    require(all(math.isclose(x, y, abs_tol=1e-12)
                for x, y in zip(swapped[3:]+swapped[:3], after)), 'Body swap invariance')

    rng = random.Random(920)
    for _ in range(30):
        pose = tuple(rng.uniform(-2, 2) for _ in range(6))
        points = ((.2, -.4), (-.1, .6))
        _, analytic = constraint_and_rows(pose, points)
        for axis in range(6):
            lo, hi = list(pose), list(pose)
            lo[axis] -= 1e-6
            hi[axis] += 1e-6
            left, _ = constraint_and_rows(lo, points)
            right, _ = constraint_and_rows(hi, points)
            for row in range(2):
                derivative = (right[row]-left[row])/2e-6
                require(math.isclose(derivative, analytic[row][axis], abs_tol=1e-8),
                        'Independent finite-difference anchor Jacobian')
    total, delta = motor_update(-.8, 1.7, 100., .01)
    require(math.isclose(total, .9) and math.isclose(delta, 1.7), 'Total versus delta cap')
    require(math.isclose(math.hypot(*bias_velocity((100., 100.), .2, .01, 3.)), 3.),
            'Large positional error respects declared bias-speed cap')
    try:
        solve_anchor(q, anchors, (0.,)*6, velocity, target)
    except ValueError:
        pass
    else:
        raise AssertionError('Both-fixed singular block accepted')
    try:
        solve_symmetric_two(1., 1., 1., (1., 1.))
    except ValueError:
        pass
    else:
        raise AssertionError('Redundant rows accepted')
    for call in (lambda: bias_velocity((math.nan, 0), .2, .01, 3),
                 lambda: motor_update(0., 1., math.inf, .01),
                 lambda: solve_symmetric_two(1., 0., 1., (math.nan, 0.))):
        try:
            call()
        except ValueError:
            pass
        else:
            raise AssertionError('Nonfinite solver input accepted')
    print('PASS: planar hinge trace, 360 finite differences, body swap, motor total cap, '
          'bias cap and singular cases; no spatial joint/world-solver claim')


if __name__ == '__main__':
    run()
