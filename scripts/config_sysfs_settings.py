#!/usr/bin/env python3

import argparse
import os

from utils import run_proc_simple


hugepage_size_kb = 1048576
# hugepage_size_kb = 2048


def get_autonuma_sysfs() -> str:
    return "/proc/sys/kernel/numa_balancing"


def check_autonuma(pre_fix: str = "") -> int:
    cmd = ["cat", get_autonuma_sysfs()]
    output_lines = run_proc_simple(cmd).splitlines()
    autonuma_setting = 0
    if len(output_lines) > 0:
        items = output_lines[0].split()
        if len(items) > 0 and items[0].isdigit():
            autonuma_setting = int(items[0])
    print(f"{pre_fix} {get_autonuma_sysfs()} setting: {autonuma_setting}")
    return autonuma_setting


def setup_autonuma(value: int) -> None:
    cmd = ["echo", str(value), ">", get_autonuma_sysfs()]
    os.system(" ".join(cmd))
    check_autonuma("new")


def main(args):
    autonuma_setting = check_autonuma("current")
    if args.setup:
        setup_autonuma(value=1)
    if args.reset:
        setup_autonuma(value=0)


def init_parser():
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    subparsers = parser.add_subparsers(dest="command")
    topics = {
        "autonuma": "NUMA balancing",
    }
    for command, message in topics.items():
        cmd_parser = subparsers.add_parser(command, help=message)
        cmd_parser.add_argument(
            "--check", "-c", action="store_true", help="check current setting"
        )
        cmd_parser.add_argument(
            "--setup", "-s", action="store_true", help="setup setting"
        )
        cmd_parser.add_argument(
            "--reset", "-r", action="store_true", help="disable setting"
        )
    return parser


if __name__ == "__main__":
    parser = init_parser()
    args = parser.parse_args()
    main(args)
