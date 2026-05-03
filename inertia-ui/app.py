#!/usr/bin/env python3
"""
inertia-ui: real-time 3D cube orientation visualizer driven by integrated gyroscope (φ).

Rendering: pygame + PyOpenGL (local, GPU-accelerated, no browser required).
The cube's pitch / yaw / roll are driven by φ = ∫ω dt (integrated gyro).

Controls
--------
  R      – reset φ (orientation) to zero
  Escape – quit
"""

import argparse
import json
import math
import os
import subprocess
import tempfile
import threading
import time

import numpy as np
import pygame
from pygame.locals import DOUBLEBUF, KEYDOWN, OPENGL, QUIT, K_ESCAPE, K_r
from OpenGL.GL import (
    GL_BLEND, GL_COLOR_BUFFER_BIT, GL_DEPTH_BUFFER_BIT, GL_DEPTH_TEST,
    GL_LINES, GL_LINEAR, GL_ONE_MINUS_SRC_ALPHA, GL_MODELVIEW, GL_PROJECTION,
    GL_QUADS, GL_RGBA, GL_SRC_ALPHA, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
    GL_TEXTURE_MIN_FILTER, GL_UNSIGNED_BYTE,
    glBegin, glBindTexture, glBlendFunc, glClear, glClearColor, glColor3f,
    glColor4f, glDeleteTextures, glDisable, glEnable, glEnd, glGenTextures,
    glLineWidth, glLoadIdentity, glMatrixMode, glOrtho, glPopMatrix,
    glPushMatrix, glTexImage2D, glTexParameteri, glTranslatef, glRotatef,
    glVertex2f, glVertex3fv, glTexCoord2f,
)
from OpenGL.GLU import gluPerspective

# ── Cube geometry ─────────────────────────────────────────────────────────────

VERTICES = np.array([
    [-1, -1, -1], [ 1, -1, -1], [ 1,  1, -1], [-1,  1, -1],  # back  (-Z)
    [-1, -1,  1], [ 1, -1,  1], [ 1,  1,  1], [-1,  1,  1],  # front (+Z)
], dtype=float)

# Each entry: (vertex_indices, rgb_color)
# Opposite face pairs share a hue; brighter shade faces the viewer by default.
FACES = [
    ([0, 1, 2, 3], (0.85, 0.20, 0.20)),  # Back   -Z  dark-red
    ([7, 6, 5, 4], (1.00, 0.50, 0.50)),  # Front  +Z  light-red
    ([0, 4, 7, 3], (0.20, 0.65, 0.20)),  # Left   -X  dark-green
    ([1, 2, 6, 5], (0.50, 1.00, 0.50)),  # Right  +X  light-green
    ([0, 1, 5, 4], (0.20, 0.20, 0.85)),  # Bottom -Y  dark-blue
    ([3, 2, 6, 7], (0.50, 0.50, 1.00)),  # Top    +Y  light-blue
]

EDGES = [
    (0, 1), (1, 2), (2, 3), (3, 0),
    (4, 5), (5, 6), (6, 7), (7, 4),
    (0, 4), (1, 5), (2, 6), (3, 7),
]

HUD_H = 115   # pixels reserved for the HUD strip at the bottom

# ── Shared IMU state ──────────────────────────────────────────────────────────

class ImuState:
    def __init__(self):
        self.lock        = threading.Lock()
        self.phi         = np.zeros(3, dtype=float)  # integrated angle  [deg]
        self.w           = np.zeros(3, dtype=float)  # angular velocity  [deg/s]
        self.g           = np.zeros(3, dtype=float)  # acceleration      [m/s^2]
        self.temperature = float("nan")
        self.error       = ""

# ── CLI ───────────────────────────────────────────────────────────────────────

DEFAULT_CLIENT = os.path.normpath(os.path.join(
    os.path.dirname(__file__),
    "../../nanoipc/build/apps/linux_uart_json_client/linux_uart_json_client",
))


def parse_args():
    p = argparse.ArgumentParser(description="IMU cube visualizer using linux_uart_json_client")
    p.add_argument("--client",    default=DEFAULT_CLIENT, help="Path to linux_uart_json_client binary")
    p.add_argument("--port",      default="/dev/ttyACM0")
    p.add_argument("--baud",      default="9600")
    p.add_argument("--parity",    default="NONE")
    p.add_argument("--data-bits", default="8")
    p.add_argument("--stop-bits", default="1")
    p.add_argument("--timeout",   default="3")
    p.add_argument("--interval",  type=float, default=0.1,
                   help="IMU poll interval in seconds (default: 0.1)")
    p.add_argument("--x_gyro_compensation", type=float, default=0.0,
                   help="Offset added to gyro_x (default: 0.0)")
    p.add_argument("--y_gyro_compensation", type=float, default=0.0,
                   help="Offset added to gyro_y (default: 0.0)")
    p.add_argument("--z_gyro_compensation", type=float, default=0.0,
                   help="Offset added to gyro_z (default: 0.0)")
    return p.parse_args()

# ── IMU polling thread ────────────────────────────────────────────────────────

def imu_poll_thread(args, state, stop):
    req_tmp  = tempfile.NamedTemporaryFile(suffix=".json", delete=False)
    resp_tmp = tempfile.NamedTemporaryFile(suffix=".json", delete=False)
    req_file, resp_file = req_tmp.name, resp_tmp.name
    req_tmp.close()
    resp_tmp.close()

    try:
        while not stop.is_set():
            t0 = time.monotonic()
            try:
                with open(req_file, "w") as f:
                    f.write("{}\n")
                subprocess.run(
                    [
                        args.client,
                        "--port", args.port, "--baud", args.baud,
                        "--parity", args.parity, "--data-bits", args.data_bits,
                        "--stop-bits", args.stop_bits,
                        "--request", req_file, "--response", resp_file,
                        "--timeout", args.timeout,
                    ],
                    check=True, timeout=int(args.timeout) + 2,
                    stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                )
                with open(resp_file) as f:
                    d = json.load(f)

                w = np.array([
                    d["gyro_x"] + args.x_gyro_compensation,
                    d["gyro_y"] + args.y_gyro_compensation,
                    d["gyro_z"] + args.z_gyro_compensation,
                ])
                g    = np.array([d["accel_x"], d["accel_y"], d["accel_z"]])
                temp = d.get("temperature", float("nan"))

                with state.lock:
                    state.phi        += w * args.interval
                    state.w           = w
                    state.g           = g
                    state.temperature = temp
                    state.error       = ""

            except Exception as exc:
                with state.lock:
                    state.error = str(exc)

            sleep = args.interval - (time.monotonic() - t0)
            if sleep > 0:
                stop.wait(sleep)
    finally:
        os.unlink(req_file)
        os.unlink(resp_file)

# ── OpenGL drawing ────────────────────────────────────────────────────────────

def draw_cube(phi):
    """Render a colored cube rotated by phi = (pitch, yaw, roll) in degrees."""
    glLoadIdentity()
    glTranslatef(0.0, 0.0, -5.0)
    glRotatef(phi[0], 1.0, 0.0, 0.0)  # pitch - rotation around X
    glRotatef(phi[1], 0.0, 1.0, 0.0)  # yaw   - rotation around Y
    glRotatef(phi[2], 0.0, 0.0, 1.0)  # roll  - rotation around Z

    glBegin(GL_QUADS)
    for indices, color in FACES:
        glColor3f(*color)
        for i in indices:
            glVertex3fv(VERTICES[i])
    glEnd()

    glLineWidth(2.5)
    glColor3f(0.04, 0.04, 0.04)
    glBegin(GL_LINES)
    for a, b in EDGES:
        glVertex3fv(VERTICES[a])
        glVertex3fv(VERTICES[b])
    glEnd()


def draw_hud(font, phi, w, g, temp, error, width, height):
    """Render a semi-transparent HUD strip at the bottom via an OpenGL texture."""
    surf = pygame.Surface((width, HUD_H), pygame.SRCALPHA)
    surf.fill((12, 12, 18, 200))

    phi_mag = np.linalg.norm(phi)
    w_mag   = np.linalg.norm(w)

    lines = [
        "pitch={:+9.2f}  yaw={:+9.2f}  roll={:+9.2f}  |phi|={:.1f} deg".format(
            phi[0], phi[1], phi[2], phi_mag),
        "w  x={:+9.2f}  y={:+9.2f}  z={:+9.2f}  |w|={:.1f} deg/s".format(
            w[0], w[1], w[2], w_mag),
        "g  x={:+8.4f}  y={:+8.4f}  z={:+8.4f}  m/s^2".format(g[0], g[1], g[2]),
        ("T  {:.2f} C".format(temp) if not math.isnan(temp) else "T  ---")
        + ("   [!] " + error if error else "")
        + "          [R] reset phi   [Esc] quit",
    ]

    y = 6
    for line in lines:
        txt = font.render(line, True, (210, 218, 230))
        surf.blit(txt, (10, y))
        y += 26

    # Upload pygame surface as an OpenGL texture (flip=True: pygame y -> GL y)
    raw = pygame.image.tostring(surf, "RGBA", True)
    tex = glGenTextures(1)
    glBindTexture(GL_TEXTURE_2D, tex)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, HUD_H, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, raw)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)

    # Switch to 2-D orthographic projection for the overlay pass
    glMatrixMode(GL_PROJECTION)
    glPushMatrix()
    glLoadIdentity()
    glOrtho(0, width, 0, height, -1, 1)
    glMatrixMode(GL_MODELVIEW)
    glPushMatrix()
    glLoadIdentity()

    glEnable(GL_TEXTURE_2D)
    glDisable(GL_DEPTH_TEST)
    glColor4f(1.0, 1.0, 1.0, 1.0)
    # Draw at screen bottom (y = 0 .. HUD_H)
    glBegin(GL_QUADS)
    glTexCoord2f(0.0, 0.0); glVertex2f(0,     0)
    glTexCoord2f(1.0, 0.0); glVertex2f(width, 0)
    glTexCoord2f(1.0, 1.0); glVertex2f(width, HUD_H)
    glTexCoord2f(0.0, 1.0); glVertex2f(0,     HUD_H)
    glEnd()
    glDisable(GL_TEXTURE_2D)
    glEnable(GL_DEPTH_TEST)

    glMatrixMode(GL_PROJECTION); glPopMatrix()
    glMatrixMode(GL_MODELVIEW);  glPopMatrix()
    glDeleteTextures([tex])

# ── Main ──────────────────────────────────────────────────────────────────────

def main():
    args   = parse_args()
    state  = ImuState()
    stop   = threading.Event()
    thread = threading.Thread(
        target=imu_poll_thread, args=(args, state, stop), daemon=True
    )
    thread.start()

    pygame.init()
    W, H = 960, 720
    pygame.display.set_mode((W, H), DOUBLEBUF | OPENGL)
    pygame.display.set_caption("Inertia UI")
    font = pygame.font.SysFont("monospace", 17)

    glEnable(GL_DEPTH_TEST)
    glEnable(GL_BLEND)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
    glClearColor(0.10, 0.10, 0.13, 1.0)
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    gluPerspective(45.0, W / H, 0.1, 50.0)
    glMatrixMode(GL_MODELVIEW)

    clock = pygame.time.Clock()

    try:
        while True:
            for event in pygame.event.get():
                if event.type == QUIT:
                    return
                if event.type == KEYDOWN:
                    if event.key == K_ESCAPE:
                        return
                    if event.key == K_r:
                        with state.lock:
                            state.phi[:] = 0.0

            with state.lock:
                phi  = state.phi.copy()
                w    = state.w.copy()
                g    = state.g.copy()
                temp = state.temperature
                err  = state.error

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
            draw_cube(phi)
            draw_hud(font, phi, w, g, temp, err, W, H)

            pygame.display.flip()
            clock.tick(60)
    finally:
        stop.set()
        thread.join(timeout=2)
        pygame.quit()


if __name__ == "__main__":
    main()
