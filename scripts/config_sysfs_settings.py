#!/usr/bin/env python3

"""Inspect or explicitly enable/disable Linux automatic NUMA balancing.

Use 'autonuma --check', '--setup' to enable, or '--reset' to disable. Writes
require root or noninteractive sudo and are verified by reading the setting back.
The runner uses the same helpers with a KernelSettings transaction for cleanup.
"""

import argparse
from pathlib import Path
import subprocess
import sys

from host_config import KernelSettings, privilege_prefix

AUTONUMA_PATH = Path('/proc/sys/kernel/numa_balancing')


def check_autonuma(pre_fix=''):
    try:
        value = int(AUTONUMA_PATH.read_text().strip())
    except (OSError, ValueError):
        value = None
    print(f'{pre_fix} NUMA balancing: {value if value is not None else "unavailable"}')
    return value


def setup_autonuma(value, settings):
    settings.set(AUTONUMA_PATH, value)


def init_parser():
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest='command')
    command = subparsers.add_parser('autonuma')
    action = command.add_mutually_exclusive_group()
    action.add_argument('--check', '-c', action='store_true', help='print current setting')
    action.add_argument('--setup', '-s', action='store_true', help='enable NUMA balancing')
    action.add_argument('--reset', '-r', action='store_true', help='disable NUMA balancing')
    return parser


def main():
    parser = init_parser()
    args = parser.parse_args()
    if args.command is None:
        parser.error('choose the autonuma subcommand')
    check_autonuma('current')
    if args.setup or args.reset:
        settings = KernelSettings(privilege_prefix())
        setup_autonuma(1 if args.setup else 0, settings)
        check_autonuma('new')


if __name__ == '__main__':
    try:
        main()
    except (OSError, subprocess.SubprocessError, ValueError, RuntimeError) as error:
        print(f'Error: {error}', file=sys.stderr)
        sys.exit(1)
