#!/usr/bin/env python3
"""Small NeRF-style volume-rendering and inverse-rendering witness.

The lab isolates the part a paper often compresses into one equation:
density -> alpha -> transmittance -> sample weight -> pixel color.
It verifies analytic gradients with finite differences and runs a tiny inverse
problem.  It is not a neural-network trainer or a benchmark implementation.
"""
from __future__ import annotations
import math
import numpy as np


def volume_render(
    sigma: np.ndarray,
    color: np.ndarray,
    delta: np.ndarray,
    background: float = 0.0,
) -> tuple[float, np.ndarray, float]:
    if sigma.ndim != 1 or color.shape != sigma.shape or delta.shape != sigma.shape:
        raise ValueError("sigma, color and delta must be equal-length 1-D arrays")
    if np.any(sigma < 0.0) or np.any(delta <= 0.0):
        raise ValueError("density must be nonnegative and interval length positive")

    alpha = 1.0 - np.exp(-sigma * delta)
    weights = np.empty_like(alpha)
    T = 1.0
    for i, a in enumerate(alpha):
        weights[i] = T * a
        T *= 1.0 - a
    pixel = float(np.dot(weights, color) + T * background)
    return pixel, weights, T


def analytic_gradients(
    sigma: np.ndarray,
    color: np.ndarray,
    delta: np.ndarray,
    background: float,
) -> tuple[np.ndarray, np.ndarray]:
    """Return dC/dsigma and dC/dcolor for a scalar-color ray."""
    pixel, weights, _ = volume_render(sigma, color, delta, background)
    del pixel

    alpha = 1.0 - np.exp(-sigma * delta)

    # tail[i] is the color produced by samples strictly after i when their
    # incoming transmittance is reset to one.
    n = len(sigma)
    tail = np.empty(n)
    tail_color = background
    for i in range(n - 1, -1, -1):
        tail[i] = tail_color
        tail_color = alpha[i] * color[i] + (1.0 - alpha[i]) * tail_color

    d_color = weights.copy()
    d_sigma = np.empty(n)
    T = 1.0
    for i in range(n):
        # d alpha / d sigma = delta * exp(-sigma*delta)
        d_alpha = delta[i] * (1.0 - alpha[i])
        d_sigma[i] = T * d_alpha * (color[i] - tail[i])
        T *= 1.0 - alpha[i]
    return d_sigma, d_color


def finite_difference(
    sigma: np.ndarray,
    color: np.ndarray,
    delta: np.ndarray,
    background: float,
    eps: float = 1e-6,
) -> tuple[np.ndarray, np.ndarray]:
    gs = np.zeros_like(sigma)
    gc = np.zeros_like(color)
    for i in range(len(sigma)):
        plus, minus = sigma.copy(), sigma.copy()
        plus[i] += eps
        minus[i] -= eps
        # Keep the finite-difference probe inside the legal sigma >= 0 domain.
        if minus[i] < 0.0:
            raise ValueError("finite-difference density probe crossed zero")
        a = volume_render(plus, color, delta, background)[0]
        b = volume_render(minus, color, delta, background)[0]
        gs[i] = (a - b) / (2.0 * eps)

        cp, cm = color.copy(), color.copy()
        cp[i] += eps
        cm[i] -= eps
        a = volume_render(sigma, cp, delta, background)[0]
        b = volume_render(sigma, cm, delta, background)[0]
        gc[i] = (a - b) / (2.0 * eps)
    return gs, gc


def positional_encoding(x: float, levels: int) -> np.ndarray:
    values = [x]
    for k in range(levels):
        f = (2.0**k) * math.pi
        values.extend((math.sin(f * x), math.cos(f * x)))
    return np.array(values)


def main() -> None:
    sigma = np.array([0.4, 1.2, 0.7, 0.3])
    color = np.array([0.1, 0.8, 0.4, 0.9])
    delta = np.array([0.2, 0.25, 0.15, 0.4])
    background = 0.05

    # 1. Weights plus surviving background transmittance form a partition.
    pixel, weights, T_end = volume_render(sigma, color, delta, background)
    np.testing.assert_allclose(np.sum(weights) + T_end, 1.0, atol=1e-12)
    assert 0.0 <= pixel <= 1.0

    # 2. Analytic rendering gradients agree with independent finite differences.
    gs, gc = analytic_gradients(sigma, color, delta, background)
    fs, fc = finite_difference(sigma, color, delta, background)
    np.testing.assert_allclose(gs, fs, atol=3e-10, rtol=2e-7)
    np.testing.assert_allclose(gc, fc, atol=3e-10, rtol=2e-7)

    # 3. A tiny inverse problem can optimize density through the renderer.
    target_sigma = np.array([0.7, 1.5, 0.5, 0.8])
    target = volume_render(target_sigma, color, delta, background)[0]
    estimate = np.full(4, 0.55)
    initial_loss = None
    for _ in range(1500):
        pred = volume_render(estimate, color, delta, background)[0]
        residual = pred - target
        loss = 0.5 * residual * residual
        if initial_loss is None:
            initial_loss = loss
        dC_dsigma, _ = analytic_gradients(estimate, color, delta, background)
        estimate -= 1.0 * residual * dC_dsigma
        estimate = np.maximum(estimate, 1e-6)
    final_pred = volume_render(estimate, color, delta, background)[0]
    final_loss = 0.5 * (final_pred - target) ** 2
    assert final_loss < initial_loss * 1e-4

    # 4. One pixel cannot uniquely identify all densities/colors.
    # A single homogeneous slab has C = alpha*c on a black background.
    sigma_a, c_a, d = 1.0, 0.8, 0.5
    target_pixel = (1.0 - math.exp(-sigma_a * d)) * c_a
    sigma_b = 2.0
    alpha_b = 1.0 - math.exp(-sigma_b * d)
    c_b = target_pixel / alpha_b
    assert 0.0 < c_b < 1.0
    pixel_b = (1.0 - math.exp(-sigma_b * d)) * c_b
    assert abs(pixel_b - target_pixel) < 1e-12
    assert abs(sigma_a - sigma_b) > 0.5 and abs(c_a - c_b) > 0.1

    # 5. Positional encoding exposes multiple spatial frequencies explicitly.
    enc = positional_encoding(0.125, 4)
    assert enc.shape == (9,)
    assert np.all(np.isfinite(enc))

    print("NeRF volume rendering: 5 suites PASS")


if __name__ == "__main__":
    main()
