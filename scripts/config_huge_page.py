#!/usr/bin/env python3

import argparse
import os
from typing import Dict, List

from print_host_info import color_str, get_mem_info
from utils import run_proc_simple


hugepage_size_kb = 1048576


def get_huge_page_sysfs(nid: int, size: int) -> str:
    return (
        f"/sys/devices/system/node/node{nid}/"
        + f"hugepages/hugepages-{size}kB/nr_hugepages"
    )


def get_huge_page_sizes() -> List[int]:
    huge_page_sizes = []
    for item in os.listdir("/sys/devices/system/node/node0/hugepages"):
        if item.startswith("hugepages-") and item.endswith("kB"):
            huge_page_sizes.append(int(item[len("hugepages-") : -len("kB")]))
    return huge_page_sizes


def pick_huge_page_size() -> int:
    huge_page_sizes = get_huge_page_sizes()
    for size in reversed(sorted(huge_page_sizes)):
        if size <= (1 << 20):
            return size
    return 0


def check_huge_pages(post_fix: str = "") -> Dict[int, List[int]]:
    mem_nodes = get_mem_info()
    size = pick_huge_page_size()
    huge_page_state = {}
    for size in get_huge_page_sizes():
        num_huge_pages = []
        for nid in range(len(mem_nodes)):
            cmd = ["cat", get_huge_page_sysfs(nid, size)]
            output_lines = run_proc_simple(cmd).splitlines()
            nr_pages = 0
            if len(output_lines) > 0:
                items = output_lines[0].split()
                if len(items) > 0 and items[0].isdigit():
                    nr_pages = int(items[0])
            num_huge_pages.append(nr_pages)
        print(f"{size}kB huge pages - {post_fix}")
        print(" ".join([f"{x:>10d}" for x in num_huge_pages]))
        huge_page_state[size] = num_huge_pages
    return huge_page_state


def setup_huge_pages() -> (bool, int):
    mem_nodes = get_mem_info(do_print=False)
    target_size = pick_huge_page_size()
    if target_size == 0:
        print(color_str(f"Fail to get a valid huge page size", 31))
    target_num = int(2 * 1024 * 1024 / target_size)
    print(color_str(f"trying to reserve {target_num}x {target_size}kB huge pages ...", 33))
    cmd1 = ["echo", "1", ">", "/proc/sys/vm/drop_caches"]
    cmd2 = ["echo", "1", ">", "/proc/sys/vm/compact_memory"]
    os.system(" ".join(cmd1))
    os.system(" ".join(cmd2))
    # reserving target_num huge pages on each node
    for nid in range(len(mem_nodes)):
        if mem_nodes[nid] < 2:
            print(color_str(f"skipping node-{nid} because size is too small", 33))
            continue
        cmd = ["echo", str(target_num), ">", get_huge_page_sysfs(nid, target_size)]
        os.system(" ".join(cmd))
    # check
    all_good = True
    huge_page_state = check_huge_pages(color_str("after", 33))
    for size in huge_page_state:
        if target_size != size:
            continue
        for nid in range(len(huge_page_state[size])):
            if mem_nodes[nid] >= 2 and huge_page_state[size][nid] < target_num:
                print(color_str(f"Fail to reserve huge pages on Node-{nid}", 31))
                all_good = False
    return (all_good, target_size)


def reset_huge_pages(huge_page_state: Dict[int, List[int]]):
    print(color_str("reverting changes to huge pages settings ...", 33))
    for size in huge_page_state:
        for nid in range(len(huge_page_state[size])):
            cmd = [
                "echo",
                str(huge_page_state[size][nid]),
                ">",
                get_huge_page_sysfs(nid, size),
            ]
            os.system(" ".join(cmd))
    check_huge_pages(color_str("reset", 33))


def main(args):
    check_huge_pages("current")
    if args.setup:
        setup_huge_pages()
    if args.reset:
        reset_huge_pages(
            {pick_huge_page_size(): [0 for _ in get_mem_info(do_print=False)]}
        )


def init_parser():
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument(
        "--check", "-c", action="store_true", help="check current hugepage setup"
    )
    parser.add_argument(
        "--setup", "-s", action="store_true", help="reserve 2x 1GB huge pages per node"
    )
    parser.add_argument(
        "--reset", "-r", action="store_true", help="un-reserve all 1GB huge pages"
    )
    return parser


if __name__ == "__main__":
    parser = init_parser()
    args = parser.parse_args()
    main(args)
