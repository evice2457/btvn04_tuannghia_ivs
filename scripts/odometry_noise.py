#!/usr/bin/env python3
"""Homework 3: Ackermann odometry with Gaussian noise on sensors.
Compares ground truth vs noisy odometry for sigma = 0.001, 0.01, 0.05.
Runs 5 trials per noise level to show the spread of drift.
"""
import math
import random
import matplotlib.pyplot as plt

WHEELBASE = 2.5
DT = 0.01
T_TOTAL = 10.0
NOISE_LEVELS = [0.001, 0.01, 0.05]
TRIALS = 5
SEED = 42

# Constant-circle command: v=1.0, phi=0.5 rad (~28 deg), ~125 deg of arc in 10s
V_TRUE = 1.0
PHI_TRUE = 0.5


def step(x, y, theta, v, phi, dt):
    omega = (v / WHEELBASE) * math.tan(phi)
    return (
        x + v * math.cos(theta) * dt,
        y + v * math.sin(theta) * dt,
        theta + omega * dt,
    )


def simulate_ground_truth():
    x, y, theta = 0.0, 0.0, 0.0
    path = [(x, y)]
    for _ in range(round(T_TOTAL / DT)):
        x, y, theta = step(x, y, theta, V_TRUE, PHI_TRUE, DT)
        path.append((x, y))
    return path


def simulate_noisy(sigma, seed_offset=0):
    """Gaussian noise on both velocity encoder and steering encoder."""
    rng = random.Random(SEED + seed_offset)
    x, y, theta = 0.0, 0.0, 0.0
    path = [(x, y)]
    for _ in range(round(T_TOTAL / DT)):
        v_meas = V_TRUE + rng.gauss(0.0, sigma)
        phi_meas = PHI_TRUE + rng.gauss(0.0, sigma)
        x, y, theta = step(x, y, theta, v_meas, phi_meas, DT)
        path.append((x, y))
    return path


def position_error(path_gt, path_noisy):
    ex = path_gt[-1][0] - path_noisy[-1][0]
    ey = path_gt[-1][1] - path_noisy[-1][1]
    return math.sqrt(ex ** 2 + ey ** 2)


def main():
    gt = simulate_ground_truth()
    gt_x = [p[0] for p in gt]
    gt_y = [p[1] for p in gt]

    fig, axes = plt.subplots(1, len(NOISE_LEVELS), figsize=(15, 5))
    fig.suptitle(
        f"Ackermann Odometry Noise — v={V_TRUE} m/s, phi={PHI_TRUE} rad, t={T_TOTAL}s"
    )

    colors = ["#FFA500", "#E05000", "#8B0000"]

    for i, sigma in enumerate(NOISE_LEVELS):
        axes[i].plot(gt_x, gt_y, "g-", linewidth=2.5, label="Ground Truth", zorder=5)

        errors = []
        for trial in range(TRIALS):
            noisy = simulate_noisy(sigma, seed_offset=trial * 100)
            nx = [p[0] for p in noisy]
            ny = [p[1] for p in noisy]
            err = position_error(gt, noisy)
            errors.append(err)
            label = f"Trial {trial + 1}" if trial == 0 else None
            axes[i].plot(nx, ny, color=colors[i], linewidth=1.2,
                         alpha=0.6, label=label)

        mean_err = sum(errors) / len(errors)
        axes[i].set_xlabel("x (m)")
        axes[i].set_ylabel("y (m)")
        axes[i].set_title(
            f"σ = {sigma}\nmean final error: {mean_err:.4f} m"
        )
        axes[i].legend(loc="upper left", fontsize=8)
        axes[i].axis("equal")
        axes[i].grid(True)

        print(f"σ = {sigma:5.3f} | errors per trial: "
              f"{', '.join(f'{e:.4f}' for e in errors)} | mean = {mean_err:.4f} m")

    plt.tight_layout()
    plt.savefig("odometry_noise.png", dpi=150)
    print("Saved odometry_noise.png")
    plt.show()


if __name__ == "__main__":
    main()
