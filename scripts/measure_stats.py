#!/usr/bin/env python3

import argparse
import subprocess
from typing import List

from utils import run_proc_simple


def measure_pmu(interval: int, pid: str) -> List[str]:
    cmd = [
        "perf", "stat", "--no-big-num",
        (f"-p {pid}" if pid else "-a"),
        "-e",
        "duration_time,cpu-clock,cycles,instructions",
        "--", "sleep", str(interval)
    ]
    return cmd


def measure_cpu_util(interval: int) -> List[str]:
    cmd = ["mpstat", "1", str(interval)]
    return cmd


def get_pids(args) -> List[str]:
    if args.pids:
        return args.pids.split(',')
    elif args.pgrep:
        pids = []
        pgrep_out = run_proc_simple(["pgrep", args.pgrep], for_real=True, print_cmd=False)
        for line in pgrep_out.splitlines():
            pids.append(line)
        if len(pids) == 0:
            pids.append("")
        return pids
    else:
        return [""]


def main(args):
    pids = get_pids(args)
    per_pid_interval = int(args.interval / len(pids))
    # mpstat
    cmd_mpstat = measure_cpu_util(per_pid_interval)
    proc_mpstat = subprocess.Popen(cmd_mpstat, shell=False)
    proc_mpstat.wait()
    # perf
    for pid in pids:
        if pid:
            ps_out = run_proc_simple(
                    ["ps", "-p", pid, "-o", "comm="], for_real=True, print_cmd=False
            )
            print(f"\nFor pid={pid} process=[{ps_out.rstrip()}]")
        cmd_perf= measure_pmu(per_pid_interval, pid)
        proc_perf = subprocess.Popen(cmd_perf, shell=False)
        proc_perf.wait()


def init_parser():
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument(
        "--interval", "-i", type=int, default=10,
        help="reporting interval"
    )
    parser.add_argument(
        "--pids", "-p", type=str, default=None,
        help="process IDs, separated by comma"
    )
    parser.add_argument(
        "--pgrep", "-s", type=str, default=None,
        help="string for pgrep command"
    )
    return parser


if __name__ == "__main__":
    parser = init_parser()
    args = parser.parse_args()
    main(args)
