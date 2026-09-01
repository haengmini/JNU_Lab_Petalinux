#!/usr/bin/env python3

import os
import mmap
import struct
import time


# ============================================================
# Configuration
# ============================================================

N = 16
ELEMENTS = N * N

CPU_RUNS = 1000
FPGA_RUNS = 1000


# ============================================================
# Reserved DDR layout
# ============================================================

A_PHYS = 0x70000000
B_PHYS = 0x70001000
C_PHYS = 0x70002000

A_OFFSET = 0x0000
B_OFFSET = 0x1000
C_OFFSET = 0x2000

CONTROL_MAP_SIZE = 0x10000
BUFFER_MAP_SIZE = 0x01000000


# ============================================================
# HLS register map
# ============================================================

REG_AP_CTRL = 0x00

REG_A       = 0x10
REG_B       = 0x1C
REG_C       = 0x28

REG_A_ROW   = 0x34
REG_A_COL   = 0x3C
REG_B_COL   = 0x44

AP_START = 1 << 0
AP_DONE  = 1 << 1
AP_IDLE  = 1 << 2
AP_READY = 1 << 3


# ============================================================
# Find UIO device by name
# ============================================================

def find_uio(name):

    base = "/sys/class/uio"

    for entry in os.listdir(base):

        if not entry.startswith("uio"):
            continue

        name_file = os.path.join(
            base,
            entry,
            "name"
        )

        with open(name_file, "r") as f:
            actual_name = f.read().strip()

        if actual_name == name:
            return "/dev/" + entry

    raise RuntimeError(
        "UIO device not found: " + name
    )


# ============================================================
# MMIO helpers
# ============================================================

def write32(mm, offset, value):

    struct.pack_into(
        "<I",
        mm,
        offset,
        value
    )


def read32(mm, offset):

    return struct.unpack_from(
        "<I",
        mm,
        offset
    )[0]


def write64(mm, offset, value):

    write32(
        mm,
        offset,
        value & 0xFFFFFFFF
    )

    write32(
        mm,
        offset + 4,
        value >> 32
    )


def write_int32(mm, offset, value):

    struct.pack_into(
        "<i",
        mm,
        offset,
        value
    )


def read_int32(mm, offset):

    return struct.unpack_from(
        "<i",
        mm,
        offset
    )[0]


# ============================================================
# CPU matrix multiplication
# ============================================================

def software_mmult(A, B, C):

    for i in range(N):

        for j in range(N):

            total = 0

            for k in range(N):

                total += (
                    A[i * N + k]
                    *
                    B[k * N + j]
                )

            C[i * N + j] = total


# ============================================================
# Run FPGA once
# ============================================================

def run_fpga(control):

    # Start accelerator
    write32(
        control,
        REG_AP_CTRL,
        AP_START
    )

    # Poll AP_DONE
    while True:

        status = read32(
            control,
            REG_AP_CTRL
        )

        if status & AP_DONE:
            return


# ============================================================
# Main
# ============================================================

control_dev = find_uio(
    "mmult-control"
)

buffer_dev = find_uio(
    "mmult-buffer"
)


print("========================================")
print(" ZCU104 Matrix Multiplication Benchmark")
print("========================================")
print()

print("Control device :", control_dev)
print("Buffer device  :", buffer_dev)
print()

print("Matrix size    : %d x %d" % (N, N))
print("CPU runs       :", CPU_RUNS)
print("FPGA runs      :", FPGA_RUNS)
print()


# ============================================================
# Open UIO devices
# ============================================================

control_fd = os.open(
    control_dev,
    os.O_RDWR | os.O_SYNC
)

buffer_fd = os.open(
    buffer_dev,
    os.O_RDWR | os.O_SYNC
)


control = mmap.mmap(
    control_fd,
    CONTROL_MAP_SIZE,
    flags=mmap.MAP_SHARED,
    prot=mmap.PROT_READ | mmap.PROT_WRITE
)


buffer = mmap.mmap(
    buffer_fd,
    BUFFER_MAP_SIZE,
    flags=mmap.MAP_SHARED,
    prot=mmap.PROT_READ | mmap.PROT_WRITE
)


# ============================================================
# Initialize matrices
# ============================================================

A = [
    i % 10
    for i in range(ELEMENTS)
]

B = [
    i % 10
    for i in range(ELEMENTS)
]

C_cpu = [
    0
    for _ in range(ELEMENTS)
]


for i in range(ELEMENTS):

    write_int32(
        buffer,
        A_OFFSET + i * 4,
        A[i]
    )

    write_int32(
        buffer,
        B_OFFSET + i * 4,
        B[i]
    )

    write_int32(
        buffer,
        C_OFFSET + i * 4,
        0
    )


# ============================================================
# CPU warm-up
# ============================================================

print("CPU warm-up...")

software_mmult(
    A,
    B,
    C_cpu
)


# ============================================================
# CPU timing
# ============================================================

print("Benchmarking CPU...")

cpu_times_ns = []


for _ in range(CPU_RUNS):

    start = time.perf_counter_ns()

    software_mmult(
        A,
        B,
        C_cpu
    )

    end = time.perf_counter_ns()

    cpu_times_ns.append(
        end - start
    )


cpu_avg_ns = (
    sum(cpu_times_ns)
    /
    len(cpu_times_ns)
)

cpu_min_ns = min(
    cpu_times_ns
)

cpu_max_ns = max(
    cpu_times_ns
)


# ============================================================
# Configure accelerator
# ============================================================

write64(
    control,
    REG_A,
    A_PHYS
)

write64(
    control,
    REG_B,
    B_PHYS
)

write64(
    control,
    REG_C,
    C_PHYS
)


write32(
    control,
    REG_A_ROW,
    N
)

write32(
    control,
    REG_A_COL,
    N
)

write32(
    control,
    REG_B_COL,
    N
)


status = read32(
    control,
    REG_AP_CTRL
)


print()
print(
    "Initial AP_CTRL = 0x%08X"
    % status
)

print(
    "  AP_IDLE  =",
    bool(status & AP_IDLE)
)

print(
    "  AP_READY =",
    bool(status & AP_READY)
)

print()


# ============================================================
# FPGA warm-up
# ============================================================

print("FPGA warm-up...")

run_fpga(
    control
)


# ============================================================
# FPGA timing
# ============================================================

print("Benchmarking FPGA...")

fpga_times_ns = []


for _ in range(FPGA_RUNS):

    start = time.perf_counter_ns()

    run_fpga(
        control
    )

    end = time.perf_counter_ns()

    fpga_times_ns.append(
        end - start
    )


fpga_avg_ns = (
    sum(fpga_times_ns)
    /
    len(fpga_times_ns)
)

fpga_min_ns = min(
    fpga_times_ns
)

fpga_max_ns = max(
    fpga_times_ns
)


# ============================================================
# Read FPGA result
# ============================================================

C_fpga = []


for i in range(ELEMENTS):

    value = read_int32(
        buffer,
        C_OFFSET + i * 4
    )

    C_fpga.append(
        value
    )


# ============================================================
# Correctness
# ============================================================

passed = (
    C_fpga == C_cpu
)


if not passed:

    for i in range(ELEMENTS):

        if C_fpga[i] != C_cpu[i]:

            print(
                "Mismatch at index %d:"
                " CPU=%d FPGA=%d"
                % (
                    i,
                    C_cpu[i],
                    C_fpga[i]
                )
            )

            break


# ============================================================
# Speedup
# ============================================================

speedup = (
    cpu_avg_ns
    /
    fpga_avg_ns
)


# ============================================================
# Results
# ============================================================

print()
print("========================================")
print(" TIMING RESULTS")
print("========================================")


print()
print("CPU (Python):")

print(
    "  Average : %.3f us"
    % (
        cpu_avg_ns / 1000.0
    )
)

print(
    "  Minimum : %.3f us"
    % (
        cpu_min_ns / 1000.0
    )
)

print(
    "  Maximum : %.3f us"
    % (
        cpu_max_ns / 1000.0
    )
)


print()
print("FPGA:")

print(
    "  Average : %.3f us"
    % (
        fpga_avg_ns / 1000.0
    )
)

print(
    "  Minimum : %.3f us"
    % (
        fpga_min_ns / 1000.0
    )
)

print(
    "  Maximum : %.3f us"
    % (
        fpga_max_ns / 1000.0
    )
)


print()
print(
    "Speedup (CPU / FPGA): %.3fx"
    % speedup
)


print()
print(
    "Result check:",
    "PASSED"
    if passed
    else "FAILED"
)


print("========================================")


# ============================================================
# Cleanup
# ============================================================

buffer.close()
control.close()

os.close(buffer_fd)
os.close(control_fd)