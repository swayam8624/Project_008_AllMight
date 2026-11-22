#!/usr/bin/env python3
"""A scalar two-splat gradient and topology-state lab, not a scene trainer."""
from dataclasses import dataclass, replace
import math
import random


def require(condition, message):
    # Checks remain active under python -O; these are executable lesson claims.
    if not condition:
        raise AssertionError(message)


def sigmoid(x):
    # Branch to avoid overflowing exp for a large negative or positive logit.
    if x >= 0:
        return 1 / (1 + math.exp(-x))
    e = math.exp(x)
    return e / (1 + e)


def loss_and_gradient(parameters, footprints, target, background=0.0):
    # The trainable vector is (front color, back color, front logit, back logit).
    # Geometry, sorting order and the Gaussian footprint values are held fixed.
    c1, c2, a1, a2 = parameters
    g1, g2 = footprints
    if not all(math.isfinite(v) for v in (*parameters, *footprints, target, background)):
        raise ValueError('All scalar inputs must be finite')
    if not (0 <= g1 <= 1 and 0 <= g2 <= 1):
        raise ValueError('This unclamped teaching model requires footprints in [0,1]')
    o1, o2 = sigmoid(a1), sigmoid(a2)
    alpha1, alpha2 = o1*g1, o2*g2
    behind = alpha2*c2 + (1-alpha2)*background
    color = alpha1*c1 + (1-alpha1)*behind
    residual = color-target
    # Chain rule: pixel alpha = opacity * footprint. Its logit derivative is
    # g*o*(1-o), NOT alpha*(1-alpha), unless the footprint equals one.
    dcolor = (alpha1, (1-alpha1)*alpha2,
              (c1-behind)*g1*o1*(1-o1),
              (1-alpha1)*(c2-background)*g2*o2*(1-o2))
    return .5*residual*residual, tuple(residual*d for d in dcolor), color


def finite_difference(parameters, footprints, target, axis, step=1e-6):
    # Re-evaluate the forward loss twice. This oracle does not use the analytic
    # derivative, so a missing transmittance or footprint factor is detectable.
    if not math.isfinite(step) or step <= 0:
        raise ValueError('Finite-difference step must be finite and positive')
    lo, hi = list(parameters), list(parameters)
    lo[axis] -= step
    hi[axis] += step
    return (loss_and_gradient(hi, footprints, target)[0]
            - loss_and_gradient(lo, footprints, target)[0])/(2*step)


@dataclass(frozen=True)
class ScalarSplat:
    # Only one scalar parameter and its two Adam moments are represented here.
    # Real trainers maintain aligned state for every trainable parameter group.
    identity: int
    parameter: float
    first_moment: float
    second_moment: float


def prune_and_clone(records, prune_ids, clone_ids):
    # This lab chooses prune-before-clone, with pruning winning conflicts.
    # It is a declared teaching policy, not Graphdeco's exact control order.
    identities = [r.identity for r in records]
    if len(identities) != len(set(identities)):
        raise ValueError('Stable identities must be unique')
    if not (set(prune_ids) | set(clone_ids)) <= set(identities):
        raise ValueError('An edit refers to an absent identity')
    output = [r for r in records if r.identity not in prune_ids]
    next_id = max(identities, default=-1) + 1
    for parent in records:
        if parent.identity in clone_ids and parent.identity not in prune_ids:
            # The child copies the parameter, but starts with zero moments.
            # Survivors retain their own moments despite changing array indices.
            output.append(replace(parent, identity=next_id,
                                  first_moment=0.0, second_moment=0.0))
            next_id += 1
    return output


def run():
    parameters = (.8, .2, 0., math.log(1/3))
    loss, gradient, color = loss_and_gradient(parameters, (1., 1.), .4)
    require(math.isclose(color, .425), 'Worked pixel color')
    require(math.isclose(gradient[0], .0125), 'Front color gradient')
    require(math.isclose(gradient[2], .0046875), 'Front logit gradient')
    # A small simultaneous update lowers this particular local loss. It does
    # not establish global convergence or identify a 3-D scene from one pixel.
    updated = tuple(p-.1*g for p, g in zip(parameters, gradient))
    require(loss_and_gradient(updated, (1., 1.), .4)[0] < loss, 'Local SGD descent')

    rng = random.Random(731)
    for _ in range(60):
        p = (rng.uniform(-.2, 1.2), rng.uniform(-.2, 1.2),
             rng.uniform(-3, 3), rng.uniform(-3, 3))
        footprints = (rng.uniform(.05, 1), rng.uniform(.05, 1))
        target = rng.uniform(-.1, 1.1)
        _, analytic, _ = loss_and_gradient(p, footprints, target)
        for axis in range(4):
            numerical = finite_difference(p, footprints, target, axis)
            require(math.isclose(analytic[axis], numerical, rel_tol=2e-5, abs_tol=1e-8),
                    'Finite-difference gradient disagreement')

    # A deliberate wrong formula must disagree when footprint != 1.
    _, correct, c = loss_and_gradient(parameters, (.35, .8), .4)
    alpha = sigmoid(parameters[2])*.35
    behind = sigmoid(parameters[3])*.8*parameters[1]
    wrong = (c-.4)*(parameters[0]-behind)*alpha*(1-alpha)
    require(not math.isclose(correct[2], wrong, rel_tol=1e-3),
            'The negative chain-rule example must expose the missing footprint')
    require(sigmoid(-1000.) == 0. and sigmoid(1000.) == 1., 'Finite saturation')

    original = [ScalarSplat(7, .2, .3, .4), ScalarSplat(9, .8, .7, .6)]
    edited = prune_and_clone(original, {7}, {7, 9})
    require([r.identity for r in edited] == [9, 10], 'Prune wins; child gets new identity')
    require(edited[0] == original[1], 'Survivor state follows identity, not old index')
    require(edited[1].parameter == .8 and edited[1].first_moment == 0
            and edited[1].second_moment == 0, 'Child state initialization')
    require(len(original) == 2, 'Topology edit leaves source checkpoint immutable')
    for bad in ([original[0], original[0]],):
        try:
            prune_and_clone(bad, set(), set())
        except ValueError:
            pass
        else:
            raise AssertionError('Duplicate identity accepted')
    try:
        finite_difference(parameters, (1, 1), .4, 0, step=0)
    except ValueError:
        pass
    else:
        raise AssertionError('Zero finite-difference step accepted')
    print('PASS: worked gradient, SGD, 240 finite differences, wrong-chain rejection, '
          'saturation, topology/moment identity and negative inputs; scalar CPU model only')


if __name__ == '__main__':
    run()
