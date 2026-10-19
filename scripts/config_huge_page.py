#!/usr/bin/env python3

"""Inspect or provision Linux huge pages using the actual NUMA node IDs.

--setup reserves up to a 2 GiB target per eligible node, preserving larger existing
pools. --clear (legacy alias --reset) explicitly sets the selected size's pools
to zero; it does not restore an earlier state. --drop-caches and --compact are
optional setup actions. Writes require root or noninteractive sudo. The runner
uses a KernelSettings transaction to restore its changes after benchmarking.
"""

import argparse
from pathlib import Path
import re
import subprocess
import sys

from host_config import KernelSettings, privilege_prefix
from print_host_info import get_mem_info, node_paths

HUGE_PAGE_OPTIONS = {2048: 1, 524288: 2, 1048576: 3, 16777216: 4}


def huge_page_paths():
    paths = {}
    for node in node_paths():
        nid = int(node.name[4:])
        for directory in (node / 'hugepages').glob('hugepages-*kB'):
            match = re.fullmatch(r'hugepages-(\d+)kB', directory.name)
            if match:
                paths[(nid, int(match.group(1)))] = directory / 'nr_hugepages'
    return paths


def check_huge_pages(post_fix=''):
    state = {}
    for key, path in sorted(huge_page_paths().items()):
        try:
            state[key] = int(path.read_text().strip())
            print(f'Node {key[0]} {key[1]} KiB huge pages {post_fix}: {state[key]}')
        except (OSError, ValueError):
            print(f'Warning: cannot read {path}', file=sys.stderr)
    return state


def pick_huge_page_size(memory=None):
    memory = get_mem_info(False) if memory is None else memory
    # Latency matrices use memory nodes with at least 1 GiB.
    nodes = [nid for nid, kib in memory.items() if kib >= 1024 ** 2]
    paths = huge_page_paths()
    sizes = [size for size in HUGE_PAGE_OPTIONS if size <= 1024 ** 2 and nodes and
             all((nid, size) in paths for nid in nodes)]
    return max(sizes, default=0)


def setup_huge_pages(settings, drop_caches=False, compact=False):
    memory = get_mem_info(False)
    size = pick_huge_page_size(memory)
    if not size:
        raise RuntimeError('No supported huge-page size is available on all eligible NUMA nodes')
    if drop_caches:
        settings.write('/proc/sys/vm/drop_caches', 1)
    if compact:
        settings.write('/proc/sys/vm/compact_memory', 1)
    paths = huge_page_paths()
    for nid, kib in memory.items():
        if kib < 1024 ** 2:
            continue
        target_kib = min(2 * 1024 ** 2, kib)
        count = target_kib // size
        path = paths[(nid, size)]
        current = int(path.read_text().strip())
        print(f'Reserve node {nid}: {max(current, count)} x {size} KiB huge pages', flush=True)
        settings.set(path, max(current, count))
    return size


def clear_huge_pages(settings):
    size = pick_huge_page_size()
    if not size:
        raise RuntimeError('No supported huge-page size found')
    for (nid, page_size), path in huge_page_paths().items():
        if page_size == size:
            settings.set(path, 0)


def init_parser():
    parser = argparse.ArgumentParser(description=__doc__)
    action = parser.add_mutually_exclusive_group()
    action.add_argument('--check', '-c', action='store_true')
    action.add_argument('--setup', '-s', action='store_true')
    action.add_argument('--clear', '--reset', '-r', dest='clear', action='store_true',
                        help='clear selected huge-page pools; does not restore previous state')
    parser.add_argument('--drop-caches', action='store_true')
    parser.add_argument('--compact', action='store_true')
    return parser


def main():
    parser = init_parser()
    args = parser.parse_args()
    if (args.drop_caches or args.compact) and not args.setup:
        parser.error('--drop-caches and --compact require --setup')
    check_huge_pages('current')
    if args.setup or args.clear:
        settings = KernelSettings(privilege_prefix())
        try:
            if args.setup:
                setup_huge_pages(settings, args.drop_caches, args.compact)
            else:
                clear_huge_pages(settings)
        except BaseException:
            settings.restore()
            raise
        check_huge_pages('after')


if __name__ == '__main__':
    try:
        main()
    except (OSError, subprocess.SubprocessError, ValueError, RuntimeError) as error:
        print(f'Error: {error}', file=sys.stderr)
        sys.exit(1)
