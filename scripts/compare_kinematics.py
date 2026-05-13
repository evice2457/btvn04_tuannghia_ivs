#!/usr/bin/env python3
"""Homework 2: Ackermann vs Differential Drive — simulate 10s, export CSV, plot."""
import math
import csv
import matplotlib.pyplot as plt

WHEELBASE = 2.5   # metres
DT = 0.01         # seconds
T_TOTAL = 10.0


class AckermannKinematics:
    """Bicycle model. Input: centre steering angle phi (rad)."""

    def __init__(self, wheelbase):
        self.L = wheelbase

    def step(self, x, y, theta, v, phi, dt):
        omega = (v / self.L) * math.tan(phi)
        return (
            x + v * math.cos(theta) * dt,
            y + v * math.sin(theta) * dt,
            theta + omega * dt,
        )


class DiffDriveKinematics:
    """Unicycle model. Input: linear velocity v and angular velocity omega."""

    def step(self, x, y, theta, v, omega, dt):
        return (
            x + v * math.cos(theta) * dt,
            y + v * math.sin(theta) * dt,
            theta + omega * dt,
        )


def get_command(t):
    """S-curve profile: straight → left arc → right arc → straight."""
    v = 1.0
    if t < 2.0:
        phi = 0.0
    elif t < 5.0:
        phi = 0.35   # ~20 deg left
    elif t < 8.0:
        phi = -0.35  # ~20 deg right
    else:
        phi = 0.0
    return v, phi


def simulate():
    ackermann = AckermannKinematics(WHEELBASE)
    diff = DiffDriveKinematics()

    ax, ay, atheta = 0.0, 0.0, 0.0
    dx, dy, dtheta = 0.0, 0.0, 0.0

    rows = []
    t = 0.0
    n_steps = round(T_TOTAL / DT) + 1

    for _ in range(n_steps):
        v, phi = get_command(t)
        # Convert Ackermann phi to equivalent diff-drive omega
        omega = (v / WHEELBASE) * math.tan(phi)

        ax, ay, atheta = ackermann.step(ax, ay, atheta, v, phi, DT)
        dx, dy, dtheta = diff.step(dx, dy, dtheta, v, omega, DT)

        rows.append({
            "time": round(t, 4),
            "x_diff": dx, "y_diff": dy, "theta_diff": dtheta,
            "x_ackermann": ax, "y_ackermann": ay, "theta_ackermann": atheta,
        })
        t = round(t + DT, 6)

    return rows


def export_csv(rows, path="kinematics_comparison.csv"):
    fields = ["time", "x_diff", "y_diff", "theta_diff",
              "x_ackermann", "y_ackermann", "theta_ackermann"]
    with open(path, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fields)
        writer.writeheader()
        for row in rows:
            writer.writerow({k: f"{v:.6f}" for k, v in row.items()})
    print(f"Saved {path}")


def plot(rows):
    times = [r["time"] for r in rows]
    xa = [r["x_ackermann"] for r in rows]
    ya = [r["y_ackermann"] for r in rows]
    xd = [r["x_diff"] for r in rows]
    yd = [r["y_diff"] for r in rows]
    ta = [r["theta_ackermann"] for r in rows]
    td = [r["theta_diff"] for r in rows]

    fig, axes = plt.subplots(1, 3, figsize=(16, 5))
    fig.suptitle("Ackermann vs Differential Drive — 10 s S-curve simulation")

    # XY trajectory
    axes[0].plot(xd, yd, "b-", linewidth=2.5, label="Diff Drive")
    axes[0].plot(xa, ya, "r--", linewidth=2, label="Ackermann")
    axes[0].set_xlabel("x (m)")
    axes[0].set_ylabel("y (m)")
    axes[0].set_title("XY Trajectory")
    axes[0].legend()
    axes[0].axis("equal")
    axes[0].grid(True)

    # Position vs time
    axes[1].plot(times, xd, "b-",  label="x diff")
    axes[1].plot(times, xa, "r--", label="x ackermann")
    axes[1].plot(times, yd, "c-",  label="y diff")
    axes[1].plot(times, ya, "m--", label="y ackermann")
    axes[1].set_xlabel("time (s)")
    axes[1].set_ylabel("position (m)")
    axes[1].set_title("Position vs Time")
    axes[1].legend()
    axes[1].grid(True)

    # Heading vs time
    axes[2].plot(times, td, "b-",  label="theta diff")
    axes[2].plot(times, ta, "r--", label="theta ackermann")
    axes[2].set_xlabel("time (s)")
    axes[2].set_ylabel("theta (rad)")
    axes[2].set_title("Heading vs Time")
    axes[2].legend()
    axes[2].grid(True)

    plt.tight_layout()
    plt.savefig("kinematics_comparison.png", dpi=150)
    print("Saved kinematics_comparison.png")
    plt.show()


if __name__ == "__main__":
    rows = simulate()
    export_csv(rows)
    plot(rows)
