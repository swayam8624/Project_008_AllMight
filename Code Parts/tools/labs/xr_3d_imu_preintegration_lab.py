#!/usr/bin/env python3
"""Deterministic 3-D IMU preintegration witness.

Scope:
- SO(3) exponential/logarithm
- zero-order-hold IMU preintegration over many high-rate samples
- composition of preintegrated intervals
- reconstruction of the later navigation state with gravity
- explicit gyro/accelerometer bias handling

This is not a complete VIO/SLAM estimator: it does not include visual residuals,
bias Jacobians, covariance propagation, marginalization, loop closure, or
camera/IMU calibration estimation.

Conventions: R_i maps body-i vectors into world coordinates; gyro samples are
rad/s, accelerometer samples are specific force in m/s^2, dt is seconds,
position is metres, and velocity is m/s. Gravity is a world-frame vector. Each
sample is held constant over its interval; force is rotated using the relative
orientation at the interval start. These choices define this discrete witness
and should not be silently substituted for midpoint or higher-order schemes.
"""
from __future__ import annotations
from dataclasses import dataclass
import math
import numpy as np

EPS = 1e-10
GRAVITY = np.array([0.0, 0.0, -9.81])


def hat(v: np.ndarray) -> np.ndarray:
    """Skew matrix [v]_x such that [v]_x w equals the 3-D cross product v x w."""
    x, y, z = v
    return np.array([[0.0, -z, y], [z, 0.0, -x], [-y, x, 0.0]])


def so3_exp(phi: np.ndarray) -> np.ndarray:
    """Map a rotation vector (axis times angle, radians) onto SO(3).

    The series branch avoids division by tiny angles; the general branch is
    Rodrigues' formula. The returned matrix actively rotates body coordinates
    into the next frame under the multiplication convention used below.
    """
    theta = float(np.linalg.norm(phi))
    W = hat(phi)
    if theta < 1e-8:
        return np.eye(3) + W + 0.5 * (W @ W)
    a = math.sin(theta) / theta
    b = (1.0 - math.cos(theta)) / (theta * theta)
    return np.eye(3) + a * W + b * (W @ W)


def so3_log(R: np.ndarray) -> np.ndarray:
    """Return the principal rotation vector; this compact lab excludes near pi."""
    cos_theta = float(np.clip((np.trace(R) - 1.0) * 0.5, -1.0, 1.0))
    theta = math.acos(cos_theta)
    if theta < 1e-8:
        return 0.5 * np.array([
            R[2, 1] - R[1, 2],
            R[0, 2] - R[2, 0],
            R[1, 0] - R[0, 1],
        ])
    if abs(math.pi - theta) < 1e-5:
        raise ValueError("compact logarithm intentionally excludes the near-pi branch")
    return theta / (2.0 * math.sin(theta)) * np.array([
        R[2, 1] - R[1, 2],
        R[0, 2] - R[2, 0],
        R[1, 0] - R[0, 1],
    ])


@dataclass
class Delta:
    """Relative motion in the starting body frame plus elapsed seconds."""
    dR: np.ndarray
    dv: np.ndarray
    dp: np.ndarray
    dt: float


def preintegrate(
    gyros: np.ndarray,
    accels: np.ndarray,
    dt: float,
    gyro_bias: np.ndarray,
    accel_bias: np.ndarray,
) -> Delta:
    """Compress equally spaced samples into gravity-free relative motion.

    ``dR`` is dimensionless, ``dv`` has m/s, ``dp`` has m, and ``dt`` has s.
    No gravity is inserted here: this keeps the relative measurement reusable
    when the optimizer changes the starting world pose or gravity estimate.
    """
    if gyros.shape != accels.shape or gyros.ndim != 2 or gyros.shape[1] != 3:
        raise ValueError("gyros and accels must both have shape (N,3)")
    if dt <= 0.0 or not np.isfinite(dt):
        raise ValueError("dt must be finite and positive")

    dR = np.eye(3)
    dv = np.zeros(3)
    dp = np.zeros(3)

    for wm, fm in zip(gyros, accels):
        omega = wm - gyro_bias
        specific_force = fm - accel_bias

        # The force during this interval is expressed through the orientation
        # at the start of the interval: a zero-order-hold discretization.
        a0 = dR @ specific_force
        dp = dp + dv * dt + 0.5 * a0 * dt * dt
        dv = dv + a0 * dt
        dR = dR @ so3_exp(omega * dt)

    return Delta(dR=dR, dv=dv, dp=dp, dt=len(gyros) * dt)


def compose(a: Delta, b: Delta) -> Delta:
    """Compose i->j and j->k deltas, rotating b's increments into frame i."""
    return Delta(
        dR=a.dR @ b.dR,
        dv=a.dv + a.dR @ b.dv,
        dp=a.dp + a.dv * b.dt + a.dR @ b.dp,
        dt=a.dt + b.dt,
    )


def apply_delta(
    R_i: np.ndarray,
    p_i: np.ndarray,
    v_i: np.ndarray,
    delta: Delta,
    gravity: np.ndarray = GRAVITY,
) -> tuple[np.ndarray, np.ndarray, np.ndarray]:
    """Recover state j, adding world gravity only at state reconstruction."""
    R_j = R_i @ delta.dR
    v_j = v_i + gravity * delta.dt + R_i @ delta.dv
    p_j = (
        p_i
        + v_i * delta.dt
        + 0.5 * gravity * delta.dt * delta.dt
        + R_i @ delta.dp
    )
    return R_j, p_j, v_j


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def main() -> None:
    """Run geometry, stationary-gravity, yaw, composition, and bias checks."""
    # 1. Exp/log and orthogonality are checked independently.
    phi = np.array([0.2, -0.1, 0.3])
    R = so3_exp(phi)
    np.testing.assert_allclose(R.T @ R, np.eye(3), atol=1e-12)
    np.testing.assert_allclose(so3_log(R), phi, atol=1e-11)

    # 2. A stationary, level IMU measures +g specific force in body z.
    # Gravity is added only when the preintegrated delta is applied.
    dt = 0.005
    n = 200
    gyros = np.zeros((n, 3))
    accels = np.tile(np.array([0.0, 0.0, 9.81]), (n, 1))
    zero = np.zeros(3)
    stationary = preintegrate(gyros, accels, dt, zero, zero)
    R1, p1, v1 = apply_delta(np.eye(3), zero, zero, stationary)
    np.testing.assert_allclose(R1, np.eye(3), atol=1e-12)
    np.testing.assert_allclose(v1, zero, atol=1e-10)
    np.testing.assert_allclose(p1, zero, atol=1e-10)

    # 3. Constant yaw rate integrates on SO(3), not by adding matrix entries.
    yaw_rate = 0.6
    gyros_yaw = np.tile(np.array([0.0, 0.0, yaw_rate]), (n, 1))
    yaw_delta = preintegrate(gyros_yaw, np.zeros_like(gyros_yaw), dt, zero, zero)
    expected = so3_exp(np.array([0.0, 0.0, yaw_rate * n * dt]))
    np.testing.assert_allclose(yaw_delta.dR, expected, atol=2e-12)

    # 4. Splitting an interval and composing it must agree with one full pass.
    half_a = preintegrate(gyros_yaw[:100], accels[:100], dt, zero, zero)
    half_b = preintegrate(gyros_yaw[100:], accels[100:], dt, zero, zero)
    joined = compose(half_a, half_b)
    full = preintegrate(gyros_yaw, accels, dt, zero, zero)
    np.testing.assert_allclose(joined.dR, full.dR, atol=2e-12)
    np.testing.assert_allclose(joined.dv, full.dv, atol=2e-12)
    np.testing.assert_allclose(joined.dp, full.dp, atol=2e-12)
    require(abs(joined.dt - full.dt) < EPS, "composed time interval must match")

    # 5. Bias is part of the state. Ignoring a known gyro bias creates drift.
    true_bias = np.array([0.0, 0.0, 0.03])
    biased_measurements = np.tile(np.array([0.0, 0.0, 0.2]) + true_bias, (n, 1))
    corrected = preintegrate(biased_measurements, np.zeros_like(biased_measurements), dt, true_bias, zero)
    uncorrected = preintegrate(biased_measurements, np.zeros_like(biased_measurements), dt, zero, zero)
    correct_angle = np.linalg.norm(so3_log(corrected.dR))
    wrong_angle = np.linalg.norm(so3_log(uncorrected.dR))
    require(abs(correct_angle - 0.2) < 1e-10, "bias-corrected yaw must match truth")
    require(wrong_angle - correct_angle > 0.02, "ignored gyro bias must create visible drift")

    print("3D IMU preintegration: 5 suites PASS")


if __name__ == "__main__":
    main()
