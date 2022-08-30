#!/usr/bin/env python3

import argparse
import subprocess
from typing import List


def measure_pmu(interval: int) -> List[str]:
    cmd = [
        "perf", "stat", "-a", "--no-big-num", "-e",
        "duration_time,cpu_cycles,inst_retired,op_retired,"
        "br_mis_pred_retired,br_retired",
        "--", "sleep", str(interval)
    ]
    return cmd


def measure_cpu_util(interval: int) -> List[str]:
    cmd = ["mpstat", str(interval), "1"]
    return cmd


def main(args):
    procs = []
    # perf
    cmd_perf= measure_pmu(args.interval)
    proc_perf = subprocess.Popen(cmd_perf, shell=False)
    procs.append(proc_perf)
    # mpstat
    cmd_mpstat = measure_cpu_util(args.interval)
    proc_mpstat = subprocess.Popen(cmd_mpstat, shell=False)
    procs.append(proc_mpstat)
    # done
    for p in procs:
        p.wait()


def init_parser():
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument(
        "--interval", "-i", type=int, default=10,
        help="reporting interval"
    )
    return parser


if __name__ == "__main__":
    parser = init_parser()
    args = parser.parse_args()
    main(args)
