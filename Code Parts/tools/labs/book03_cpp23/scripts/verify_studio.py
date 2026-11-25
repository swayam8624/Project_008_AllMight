#!/usr/bin/env python3
"""Independent P31 fixture verifier; standard library only.

Usage: python3 verify_studio.py path/to/new-output-directory
The reference is analytic geometry, not the C++ rasterizer or triangulator.
"""
import csv
import hashlib
import math
from pathlib import Path
import sys


def require(condition, description):
    if not condition:
        raise ValueError(description)


def encode(x):
    y = 12.92 * x if x <= .0031308 else 1.055 * x ** (1 / 2.4) - .055
    return math.floor(255 * y + .5)


def verify(directory):
    # P31 deliberately emits a fixed P6 header, not arbitrary PPM syntax.
    header = b"P6\n320 240\n255\n"
    reference = (directory / "reference.ppm").read_bytes()
    reconstructed = (directory / "reconstructed.ppm").read_bytes()
    require(reference == reconstructed, "PPM files differ")
    palette = [bytes(map(encode, c)) for c in [(.035, .05, .065),
               (.65, .43, .22), (.50, .12, .08), (.08, .40, .65)]]
    expected = bytearray(header)
    counts = [0] * 4
    for y in range(240):
        for x in range(320):
            u, v = x + .5, y + .5
            index = 0
            if 40 <= v < 100 and abs(u - 160) < (v - 40) * 4 / 3:
                index = 2
            if 80 <= u < 240 and 100 <= v < 200:
                index = 1
            if 130 <= u < 190 and 135 <= v < 175:
                index = 3
            counts[index] += 1
            expected.extend(palette[index])
    require(reference == expected, "RGB oracle disagreement")
    require(counts == [56000, 13600, 4800, 2400], "coverage count")
    vertices, faces = [], []
    for line in (directory / "reconstructed.obj").read_text().splitlines():
        fields = line.split()
        if fields and fields[0] == "v":
            vertices.append(tuple(map(float, fields[1:])))
        if fields and fields[0] == "f":
            faces.append(tuple(map(int, fields[1:])))
    require(len(vertices) == 9 and all(len(p) == 3 and all(map(math.isfinite, p))
            for p in vertices), "OBJ vertices")
    require(faces == [(1,2,3),(1,3,4),(1,5,2),(6,7,8),(6,8,9)], "OBJ faces")
    with (directory / "tracks.csv").open(newline="") as stream:
        rows = list(csv.DictReader(stream))
    require(len(rows) == 27, "track count")
    observations = {}
    for row in rows:
        key = int(row["landmark"]), int(row["camera"])
        require(key not in observations, "duplicate observation")
        observations[key] = float(row["u"]), float(row["v"])
    require(set(observations) == {(i,j) for i in range(9) for j in range(3)},
            "observation keys")
    require(all(math.isfinite(x) for uv in observations.values() for x in uv), "finite observations")
    truth_pixels = [(80,100,4),(240,100,4),(240,200,4),(80,200,4),(160,40,4),
                    (130,135,3.8),(190,135,3.8),(190,175,3.8),(130,175,3.8)]
    errors, truth_errors, residuals = [], [], []
    for i, p in enumerate(vertices):
        u, v = observations[i,0]
        u_truth, v_truth, z_truth = truth_pixels[i]
        require(abs(u-u_truth)<1e-10 and abs(v-v_truth)<1e-10, "fixture first view")
        z = 64 / (u - observations[i,1][0])
        expected_point = ((u-160)*z/160, (v-120)*z/160, z)
        errors.append(math.dist(p, expected_point))
        truth_point = ((u_truth-160)*z_truth/160, (v_truth-120)*z_truth/160, z_truth)
        truth_errors.append(math.dist(p, truth_point))
        for j, (tx,ty,tz) in enumerate([(0,0,0),(-.4,0,0),(0,-.3,0)]):
            x,y,zp = p[0]+tx,p[1]+ty,p[2]+tz
            require(zp > 0, "positive depth")
            u,v = observations[i,j]
            residuals.append((160*x/zp+160-u)**2 + (160*y/zp+120-v)**2)
    rms = math.sqrt(sum(residuals)/len(residuals))
    require(max(errors)<1e-10 and max(truth_errors)<1e-10 and rms<1e-10, "geometry oracle")
    with (directory / "metrics.csv").open(newline="") as stream:
        metrics = list(csv.DictReader(stream))
    require(len(metrics)==1, "metrics row count")
    m = metrics[0]
    require([int(m[k]) for k in ("vertices","faces","observations")]==[9,5,27], "metrics counts")
    require(abs(float(m["max_3d_m"])-max(truth_errors))<1e-12 and
            abs(float(m["rms_px"])-rms)<1e-12, "metrics disagreement")
    print("P31 independent PASS", "pixels=76800", "counts="+str(counts),
          "max_stereo_error="+str(max(errors)), "rms_pixels="+str(rms),
          "sha256="+hashlib.sha256(reference).hexdigest())


if __name__ == "__main__":
    if len(sys.argv) != 2:
        raise SystemExit("usage: verify_studio.py OUTPUT_DIRECTORY")
    verify(Path(sys.argv[1]))
