#!/usr/bin/env python3
"""Deterministic planar visual-inertial EKF, not full 3-D VIO or OpenXR.
World is horizontal; gravity removed from horizontal acceleration channels.
Three surveyed landmarks give camera bearings. Requires NumPy.
"""
import math
import numpy as np

DT = 0.02
LANDMARKS = (np.array([6.0, 4.0]), np.array([-3.0, 5.0]), np.array([4.0, -6.0]))
N = 240
PX, PY, VX, VY, YAW, BG, BAX, BAY = range(8)
BIAS = np.array([0.025, 0.040, -0.020])
STD_ACC, STD_GYR = 0.025, 0.007
STD_BIAS_G, STD_BIAS_A = 0.0005, 0.001
STD_CAMERA = 0.012
CHI2_99_1DOF = 6.635

def wrap(angle: float) -> float:
    """Normalize bearings and corrections to shortest signed angular distance."""
    return (angle + math.pi) % (2.0 * math.pi) - math.pi

def rotation(theta: float) -> np.ndarray:
    c, s = math.cos(theta), math.sin(theta)
    return np.array([[c, -s], [s, c]], dtype=float)

def propagation(x: np.ndarray, gyro: float, a_sensor: np.ndarray) -> np.ndarray:
    """x=(world p, world v, yaw, gyro bias, body accel bias); zero-order hold."""
    y = x.copy()
    R = rotation(x[YAW])  # body -> world
    a_world = R @ (a_sensor - x[[BAX, BAY]])
    y[[PX, PY]] += DT * x[[VX, VY]] + 0.5 * DT**2 * a_world
    y[[VX, VY]] += DT * a_world
    y[YAW] = wrap(x[YAW] + DT * (gyro - x[BG]))
    return y

def propagation_jacobian(x: np.ndarray, a_sensor: np.ndarray) -> np.ndarray:
    """F_ij=d f_i / d x_j; mistakes here create silently inconsistent covariance."""
    F = np.eye(8)
    R = rotation(x[YAW])
    J = np.array([[0.0, -1.0], [1.0, 0.0]])
    dacc_dtheta = R @ J @ (a_sensor - x[[BAX, BAY]])
    F[PX:PY+1, VX:VY+1] = DT * np.eye(2)
    F[PX:PY+1, YAW] = 0.5 * DT**2 * dacc_dtheta
    F[VX:VY+1, YAW] = DT * dacc_dtheta
    F[PX:PY+1, BAX:BAY+1] = -0.5 * DT**2 * R
    F[VX:VY+1, BAX:BAY+1] = -DT * R
    F[YAW, BG] = -DT
    return F

def process_covariance(x: np.ndarray) -> np.ndarray:
    """Inject per-sample gyro/accel noise and bias random-walk increments."""
    G = np.zeros((8, 3))
    R = rotation(x[YAW])
    G[PX:PY+1, :2] = 0.5 * DT**2 * R
    G[VX:VY+1, :2] = DT * R
    G[YAW, 2] = DT
    sample_variance = np.diag([STD_ACC**2, STD_ACC**2, STD_GYR**2])
    Q = G @ sample_variance @ G.T
    Q[BG, BG] += STD_BIAS_G**2 * DT
    Q[BAX, BAX] += STD_BIAS_A**2 * DT
    Q[BAY, BAY] += STD_BIAS_A**2 * DT
    return Q

def camera_bearing(x: np.ndarray, landmark=LANDMARKS[0]) -> float:
    dx, dy = landmark - x[[PX, PY]]
    if dx*dx + dy*dy < 1e-8:
        raise ValueError("undefined landmark bearing at zero range")
    return wrap(math.atan2(dy, dx) - x[YAW])

def camera_jacobian(x: np.ndarray, landmark=LANDMARKS[0]) -> np.ndarray:
    """For delta=(L-p), d atan2(delta_y,delta_x)/d p=(dy,-dx)/r^2."""
    dx, dy = landmark - x[[PX, PY]]
    r2 = dx*dx + dy*dy
    if r2 < 1e-8:
        raise ValueError("undefined bearing Jacobian at zero range")
    H = np.zeros((1, 8))
    H[0, PX], H[0, PY], H[0, YAW] = dy/r2, -dx/r2, -1.0
    return H

def camera_update(x: np.ndarray, P: np.ndarray, z: float, landmark=LANDMARKS[0]):
    """EKF with angular residual, 99% one-DOF NIS gate and Joseph covariance."""
    H = camera_jacobian(x, landmark)
    innovation = wrap(z - camera_bearing(x, landmark))
    variance = STD_CAMERA**2
    S = float((H @ P @ H.T)[0, 0] + variance)
    if innovation**2 / S > CHI2_99_1DOF:
        return x, P, False
    K = (P @ H.T)[:, 0] / S
    corrected = x + K * innovation
    corrected[YAW] = wrap(corrected[YAW])
    A = np.eye(8) - np.outer(K, H[0])
    next_cov = A @ P @ A.T + variance * np.outer(K, K)
    next_cov = 0.5 * (next_cov + next_cov.T)
    return corrected, next_cov, True

def check_jacobians_and_rejection():
    """Every analytic derivative must agree with independent central differences."""
    x = np.array([0.2, -0.1, 0.8, 0.1, 0.3, 0.02, -0.01, 0.03])
    a = np.array([0.4, -0.2])
    F, H = propagation_jacobian(x, a), camera_jacobian(x)
    eps = 1e-6
    for j in range(8):
        d = np.zeros(8)
        d[j] = eps
        plus = propagation(x + d, 0.12, a)
        minus = propagation(x - d, 0.12, a)
        derivative = (plus - minus) / (2 * eps)
        derivative[YAW] = wrap(plus[YAW] - minus[YAW]) / (2 * eps)
        np.testing.assert_allclose(F[:, j], derivative, atol=2e-8, rtol=1e-7)
        difference = wrap(camera_bearing(x + d) - camera_bearing(x - d))
        np.testing.assert_allclose(H[0, j], difference/(2*eps), atol=2e-8)
    P = np.diag([.02, .02, .02, .02, .02, .001, .001, .001])
    xx, PP, accepted = camera_update(x, P, camera_bearing(x) + 2.5)
    assert not accepted and np.array_equal(xx, x) and np.array_equal(PP, P)

def simulate():
    rng = np.random.default_rng(20261112)
    truth = np.array([0., 0., 1., 0.1, 0.1, *BIAS])
    est = np.array([0.08, -0.08, 0.85, 0.05, -0.03, 0., 0., 0.])
    P = np.diag([.12, .12, .08, .08, .08, .004, .012, .012])
    accepted = rejected = 0
    for step in range(N):
        t = step * DT
        angular_rate = 0.14 + 0.02 * math.sin(0.7*t)
        true_body_acc = np.array([0.06 + 0.02*math.sin(t), .015*math.cos(t)])
        # Advance truth using exact synthetic bias; sensor noise only affects estimates.
        truth = propagation(truth, angular_rate + BIAS[0], true_body_acc + BIAS[1:])
        measured_gyro = angular_rate + BIAS[0] + rng.normal(0, STD_GYR)
        measured_acc = true_body_acc + BIAS[1:] + rng.normal(0, STD_ACC, 2)
        F = propagation_jacobian(est, measured_acc)
        Q = process_covariance(est)
        est = propagation(est, measured_gyro, measured_acc)
        P = F @ P @ F.T + Q
        if step % 6 == 0:
            for index, landmark in enumerate(LANDMARKS):
                z = camera_bearing(truth, landmark) + rng.normal(0, STD_CAMERA)
                if step == 60 and index == 0:
                    z += 2.5  # deliberately wrong feature correspondence
                est, P, ok = camera_update(est, P, z, landmark)
                accepted += int(ok)
                rejected += int(not ok)
        assert np.min(np.linalg.eigvalsh(P)) > -1e-10
    position_error = float(np.linalg.norm(est[[PX, PY]] - truth[[PX, PY]]))
    yaw_error = abs(wrap(est[YAW] - truth[YAW]))
    print(f"camera updates accepted={accepted}, rejected={rejected}")
    print(f"final planar position error={position_error:.4f} m")
    print(f"final yaw error={yaw_error:.4f} rad")
    assert rejected >= 1 and accepted > 100 and position_error < 0.25

if __name__ == "__main__":
    check_jacobians_and_rejection()
    simulate()
    print("PASS: Jacobians, outlier gate, PSD covariance and deterministic replay")
