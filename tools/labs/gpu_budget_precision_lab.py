#!/usr/bin/env python3
"""Scalar witnesses for the book's GPU timeline/budget examples, not a GPU test."""
from dataclasses import dataclass
import math
import struct
import unittest


def rounded(value, code):
    # 'e' is IEEE binary16 storage; 'f' is binary32 storage. Packing performs a
    # storage conversion. Host arithmetic between conversions remains Python float.
    # This cannot certify shader flags, device subnormals or matrix instructions.
    return struct.unpack('<' + code, struct.pack('<' + code, float(value)))[0]


@dataclass(frozen=True)
class Frame:
    number: int
    slot: int
    cpu_start: float
    submitted: float
    gpu_start: float
    retired: float


def timeline(cpu_ms, gpu_ms, slots, frame_count):
    # One ordered GPU queue; retirement covers every consumer of a frame slot.
    # No transfer, presentation, separate queues or host-recording overhead.
    if (not math.isfinite(cpu_ms) or not math.isfinite(gpu_ms) or
            cpu_ms <= 0 or gpu_ms <= 0 or
            not isinstance(slots, int) or isinstance(slots, bool) or slots <= 0 or
            not isinstance(frame_count, int) or frame_count < 0):
        raise ValueError('positive finite durations/slots and nonnegative frame count required')
    slot_retired = [0.0] * slots
    cpu_time = gpu_time = 0.0
    result = []
    for number in range(frame_count):
        slot = number % slots
        # Reusing the index grants no ownership: wait for that slot's old reader.
        cpu_start = max(cpu_time, slot_retired[slot])
        submitted = cpu_start + cpu_ms
        gpu_start = max(submitted, gpu_time)
        retired = gpu_start + gpu_ms
        if not all(math.isfinite(t) for t in (submitted, retired)):
            raise OverflowError('timeline exceeds representable duration')
        result.append(Frame(number, slot, cpu_start, submitted, gpu_start, retired))
        slot_retired[slot] = retired
        cpu_time, gpu_time = submitted, retired
    return result


def overlap(first, second):
    # Adjacent half-open ranges do not overlap.
    return first[0] < second[1] and second[0] < first[1]


def can_reuse(request, live_ranges, completed):
    # One scalar token is valid only for one ordered/joined completion timeline.
    return all(token <= completed or not overlap(request, interval)
               for interval, token in live_ranges)


class BudgetPrecisionChecks(unittest.TestCase):
    def test_buffer_timeline_and_throughput(self):
        frames = timeline(2, 6, 3, 4)
        self.assertEqual([(f.cpu_start, f.submitted, f.gpu_start, f.retired)
                          for f in frames],
                         [(0, 2, 2, 8), (2, 4, 8, 14),
                          (4, 6, 14, 20), (8, 10, 20, 26)])
        twice = timeline(2, 6, 2, 3)
        self.assertEqual([f.retired for f in twice], [8, 14, 20])
        # More slots did not change the bottleneck's completion cadence here.
        self.assertEqual(twice[2].cpu_start, 8)
        self.assertEqual(frames[2].cpu_start, 4)

    def test_interval_retirement_not_modulo(self):
        live = [((0, 256), 11), ((256, 512), 12)]
        self.assertFalse(can_reuse((0, 128), live, 10))
        self.assertTrue(can_reuse((0, 128), live, 11))
        self.assertFalse(can_reuse((0, 300), live, 11))
        self.assertTrue(can_reuse((0, 300), live, 12))
        self.assertFalse(overlap((0, 256), (256, 512)))

    def test_transfer_and_roofline_units(self):
        # Binary MiB/GiB for the transfer, decimal GB/s/TFLOP/s for the
        # illustrative roofline. Mixing these conventions silently changes results.
        transfer_seconds = 64 * 2**20 / (16 * 2**30)
        self.assertEqual(transfer_seconds, 1 / 256)
        self.assertEqual(transfer_seconds * 1000, 3.90625)
        peak, bandwidth = 2e12, 200e9
        intensity = 2 / 12
        self.assertEqual(peak / bandwidth, 10)
        self.assertAlmostEqual(min(peak, bandwidth * intensity) / 1e9, 100 / 3)
        # 2*n^3 conventional FLOP and 12*n^2 bytes under ideal input reuse.
        self.assertAlmostEqual((2 * 128**3) / (12 * 128**2), 128 / 6)
        self.assertEqual(max(1 + 2, 3) + 4, 7)  # Ideal U/C versus S overlap.

    def test_storage_precision_and_association(self):
        self.assertEqual(rounded(65504, 'e'), 65504)
        self.assertEqual(rounded(2**-24, 'e'), 2**-24)
        with self.assertRaises(OverflowError):
            rounded(70000, 'e')
        # Ties-to-even: 2049 is between binary16 neighbors 2048 and 2050.
        self.assertEqual(rounded(2048 + 1, 'e'), 2048)
        half_acc = 2048.0
        wide_acc = 2048.0
        for _ in range(8):
            half_acc = rounded(half_acc + 1, 'e')
            wide_acc = rounded(wide_acc + 1, 'f')
        self.assertEqual(half_acc, 2048)
        self.assertEqual(wide_acc, 2056)
        a, b, c = 2**30, -(2**30), 1.0
        left = rounded(rounded(a + b, 'f') + c, 'f')
        right = rounded(a + rounded(b + c, 'f'), 'f')
        self.assertEqual((left, right), (1, 0))
        # This demonstrates explicit storage rounding, not execution of a
        # compiler's fast-math optimization on a GPU.

    def test_matrix_example(self):
        a, b = [[1, 2], [3, 4]], [[5, 6], [7, 8]]
        result = [[sum(a[i][k] * b[k][j] for k in range(2))
                   for j in range(2)] for i in range(2)]
        self.assertEqual(result, [[19, 22], [43, 50]])
        # A scalar oracle knows the result, not a backend's lane-fragment layout.

    def test_invalid_timeline_inputs(self):
        for cpu, gpu, slots in [(0, 1, 3), (2, -1, 3), (2, 6, 0),
                                (float('nan'), 6, 3), (2, float('inf'), 3)]:
            with self.assertRaises(ValueError):
                timeline(cpu, gpu, slots, 1)


if __name__ == '__main__':
    unittest.main(verbosity=2)
