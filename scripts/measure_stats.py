#!/usr/bin/env python3

"""Report Linux CPU utilization and hardware performance counters.

Run mpstat and perf stat concurrently for --interval seconds. --pids selects
explicit PIDs; --pgrep matches full command lines, excluding this script and its
ancestors. Each selected PID gets a separate user-space counter report. No match
is an error; only omitting both selectors requests system-wide counters. Requires
mpstat, perf, and permission to access the requested performance counters.
"""

import argparse
import os
import subprocess
import sys
from typing import List


def measure_pmu(interval: int, pid: str) -> List[str]:
    scope = ["-p", pid] if pid else ["-a"]
    events = ("duration_time,cpu-clock:u,cycles:u,instructions:u" if pid else
              "duration_time,cpu-clock,cycles,instructions")
    return ["perf", "stat", "--no-big-num"] + scope + [
        "-e", events, "--", "sleep", str(interval)
    ]


def measure_cpu_util(interval: int) -> List[str]:
    return ["mpstat", "1", str(interval)]


def ancestor_pids() -> set:
    # A full-command match may include this script and the shell invoking it.
    excluded = set()
    pid = os.getpid()
    while pid and str(pid) not in excluded:
        excluded.add(str(pid))
        try:
            with open(f"/proc/{pid}/stat") as stat:
                pid = int(stat.read().rsplit(")", 1)[1].split()[1])
        except (OSError, ValueError, IndexError):
            break
    return excluded


def get_pids(args) -> List[str]:
    if args.pids is not None:
        pids = [pid.strip() for pid in args.pids.split(',')]
        if any(not pid or any(char not in "0123456789" for char in pid) or int(pid) <= 0 for pid in pids):
            raise ValueError("--pids requires positive process IDs separated by commas")
        return list(dict.fromkeys(str(int(pid)) for pid in pids))
    if args.pgrep is not None:
        if not args.pgrep.strip():
            raise ValueError("--pgrep requires a nonempty pattern")
        result = subprocess.run(
            ["pgrep", "-f", "--", args.pgrep], stdout=subprocess.PIPE,
            stderr=subprocess.PIPE, universal_newlines=True,
        )
        if result.returncode not in (0, 1):
            raise ValueError("pgrep failed: " + result.stderr.strip())
        excluded = ancestor_pids()
        pids = [pid for pid in result.stdout.split() if pid not in excluded]
        if not pids:
            raise ValueError(f"No processes matched {args.pgrep!r}; no counters collected")
        return pids
    return [""]


def main(args) -> int:
    processes = []
    try:
        pids = get_pids(args)
        # Start every counter collector before waiting for any report.
        reports = []
        for pid in pids:
            proc = subprocess.Popen(
                measure_pmu(args.interval, pid), stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT, universal_newlines=True,
            )
            processes.append(proc)
            reports.append((pid, proc))
        mpstat = subprocess.Popen(measure_cpu_util(args.interval))
        processes.append(mpstat)
        failed = mpstat.wait() != 0
        for pid, proc in reports:
            output, _ = proc.communicate()
            print(f"\nFor pid={pid}" if pid else "\nSystem-wide counters", flush=True)
            print(output, end="", flush=True)
            failed = proc.returncode != 0 or failed
        return 1 if failed else 0
    except (OSError, ValueError) as error:
        print(f"Error: {error}", file=sys.stderr)
        return 1
    except KeyboardInterrupt:
        return 130
    finally:
        for proc in processes:
            if proc.poll() is None:
                proc.terminate()
        for proc in processes:
            try:
                proc.wait(timeout=2)
            except subprocess.TimeoutExpired:
                proc.kill()
                proc.wait()
            if proc.stdout is not None:
                proc.stdout.close()


def positive_interval(value: str) -> int:
    interval = int(value)
    if interval <= 0:
        raise argparse.ArgumentTypeError("interval must be a positive number of seconds")
    return interval


def init_parser():
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument(
        "--interval", "-i", type=positive_interval, default=10,
        help="sampling duration in seconds for all collectors"
    )
    selectors = parser.add_mutually_exclusive_group()
    selectors.add_argument(
        "--pids", "-p", type=str, default=None,
        help="process IDs, separated by comma"
    )
    selectors.add_argument(
        "--pgrep", "-s", type=str, default=None,
        help="pgrep regular expression matched against full command lines"
    )
    return parser


if __name__ == "__main__":
    sys.exit(main(init_parser().parse_args()))
