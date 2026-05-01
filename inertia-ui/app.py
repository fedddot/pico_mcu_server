#!/usr/bin/env python3
"""
inertia-ui: visualizes IMU vectors (accelerometer g and gyroscope w) received
from linux_uart_json_client in an isometric 3D plot.
"""

import argparse
import json
import os
import subprocess
import tempfile
import time

import matplotlib
import matplotlib.pyplot as plt
import numpy as np
from mpl_toolkits.mplot3d import Axes3D  # noqa: F401

# Default path to the linux_uart_json_client binary
DEFAULT_CLIENT = os.path.join(
    os.path.dirname(__file__),
    "../../nanoipc/build/apps/linux_uart_json_client/linux_uart_json_client",
)
DEFAULT_CLIENT = os.path.normpath(DEFAULT_CLIENT)


def parse_args():
    p = argparse.ArgumentParser(
        description="IMU vector visualizer using linux_uart_json_client"
    )
    p.add_argument("--client", default=DEFAULT_CLIENT, help="Path to linux_uart_json_client binary")
    p.add_argument("--port", default="/dev/ttyACM0", help="Serial port (default: /dev/ttyACM0)")
    p.add_argument("--baud", default="9600", help="Baud rate (default: 9600)")
    p.add_argument("--parity", default="NONE", help="Parity (default: NONE)")
    p.add_argument("--data-bits", default="8", help="Data bits (default: 8)")
    p.add_argument("--stop-bits", default="1", help="Stop bits (default: 1)")
    p.add_argument("--timeout", default="3", help="Client timeout in seconds (default: 3)")
    p.add_argument(
        "--interval",
        type=float,
        default=1.0,
        help="Refresh interval in seconds (default: 1.0)",
    )
    p.add_argument("--x_gyro_compensation", type=float, default=0.0, help="Offset added to gyro_x (default: 0.0)")
    p.add_argument("--y_gyro_compensation", type=float, default=0.0, help="Offset added to gyro_y (default: 0.0)")
    p.add_argument("--z_gyro_compensation", type=float, default=0.0, help="Offset added to gyro_z (default: 0.0)")
    return p.parse_args()


def query_imu(args, req_file: str, resp_file: str) -> dict:
    """Invoke linux_uart_json_client and return the parsed JSON response."""
    with open(req_file, "w") as f:
        f.write("{}\n")

    cmd = [
        args.client,
        "--port", args.port,
        "--baud", args.baud,
        "--parity", args.parity,
        "--data-bits", args.data_bits,
        "--stop-bits", args.stop_bits,
        "--request", req_file,
        "--response", resp_file,
        "--timeout", args.timeout,
    ]
    subprocess.run(cmd, check=True, timeout=int(args.timeout) + 2)

    with open(resp_file) as f:
        return json.load(f)


def make_arrow(ax, vec, color, label, scale):
    """Draw a 3D arrow from the origin for vec, scaled to fit the plot."""
    x, y, z = vec
    ax.quiver(
        0, 0, 0, x, y, z,
        color=color,
        arrow_length_ratio=0.15,
        linewidth=2,
        label=f"{label} ({x:.3f}, {y:.3f}, {z:.3f})",
    )


def set_isometric_view(ax):
    """Configure the axes for an isometric-like perspective."""
    ax.view_init(elev=35.264, azim=45)


def setup_axes(ax, scale):
    ax.set_xlim(-scale, scale)
    ax.set_ylim(-scale, scale)
    ax.set_zlim(-scale, scale)
    ax.set_xlabel("X")
    ax.set_ylabel("Y")
    ax.set_zlabel("Z")
    ax.set_box_aspect([1, 1, 1])
    set_isometric_view(ax)

    # Draw reference axes (thin gray)
    for direction, kwargs in [
        ((scale, 0, 0), dict(color="red",   linestyle="--", linewidth=0.5)),
        ((0, scale, 0), dict(color="green", linestyle="--", linewidth=0.5)),
        ((0, 0, scale), dict(color="blue",  linestyle="--", linewidth=0.5)),
    ]:
        ax.plot(
            [0, direction[0]], [0, direction[1]], [0, direction[2]],
            **kwargs, alpha=0.4,
        )


def render(fig, ax, data: dict, gyro_compensation=(0.0, 0.0, 0.0)):
    ax.cla()

    g = np.array([data["accel_x"], data["accel_y"], data["accel_z"]])
    w = np.array([
        data["gyro_x"] + gyro_compensation[0],
        data["gyro_y"] + gyro_compensation[1],
        data["gyro_z"] + gyro_compensation[2],
    ])

    g_mag = np.linalg.norm(g)
    w_mag = np.linalg.norm(w)

    # Normalize gyro to same visual scale as accel so both fit the view
    w_disp = w / w_mag * g_mag if w_mag > 1e-9 else w

    scale = max(g_mag * 1.4, 0.1)
    setup_axes(ax, scale)

    make_arrow(ax, g,      color="#e74c3c", label="g (accel) [m/s²]", scale=scale)
    make_arrow(ax, w_disp, color="#3498db", label="ω (gyro, normalized) [°/s]", scale=scale)

    temp = data.get("temperature", float("nan"))
    ax.set_title(
        f"IMU Vectors  |  T = {temp:.2f} °C\n"
        f"|g| = {g_mag:.4f} m/s²   |ω| = {w_mag:.2f} °/s",
        fontsize=10,
    )
    ax.legend(loc="upper left", fontsize=8)

    fig.canvas.draw()
    fig.canvas.flush_events()


def main():
    args = parse_args()

    matplotlib.use("TkAgg")  # interactive backend; change to "Qt5Agg" if needed
    plt.ion()
    fig = plt.figure(figsize=(8, 7))
    ax = fig.add_subplot(111, projection="3d")
    fig.suptitle("Inertia UI", fontsize=13, fontweight="bold")

    with tempfile.NamedTemporaryFile(suffix=".json", delete=False) as req_f:
        req_file = req_f.name
    with tempfile.NamedTemporaryFile(suffix=".json", delete=False) as resp_f:
        resp_file = resp_f.name

    try:
        while plt.fignum_exists(fig.number):
            try:
                data = query_imu(args, req_file, resp_file)
                render(fig, ax, data, gyro_compensation=(
                    args.x_gyro_compensation,
                    args.y_gyro_compensation,
                    args.z_gyro_compensation,
                ))
            except subprocess.CalledProcessError as e:
                print(f"[error] client failed: {e}")
            except Exception as e:
                print(f"[error] {e}")
            time.sleep(args.interval)
    except KeyboardInterrupt:
        pass
    finally:
        os.unlink(req_file)
        os.unlink(resp_file)
        plt.close(fig)


if __name__ == "__main__":
    main()
