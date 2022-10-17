#!/usr/bin/env python3

import argparse
import os
from typing import List
from utils import run_proc_simple, read_env
from print_host_info import color_str, print_cpu_info, print_mem_info


hugepage_size_kb = 1048576
# hugepage_size_kb = 2048


def get_huge_page_sysfs(nid: int) -> str:
    return (f"/sys/devices/system/node/node{nid}/" +
            f"hugepages/hugepages-{hugepage_size_kb}kB/nr_hugepages")


def check_huge_pages(num_numa_nodes: int, post_fix: str = "") -> List[int]:
    num_huge_pages = []
    for nid in range(num_numa_nodes):
        cmd = ["cat", get_huge_page_sysfs(nid)]
        output_lines = run_proc_simple(cmd).splitlines()
        nr_pages = 0
        if len(output_lines) > 0:
            items = output_lines[0].split()
            if len(items) > 0 and items[0].isdigit():
                nr_pages = int(items[0])
        num_huge_pages.append(nr_pages)
    print(f"1GB huge pages - {post_fix}")
    print(" ".join([f"{x:>10d}" for x in num_huge_pages]))
    return num_huge_pages


def setup_huge_pages(num_numa_nodes: int) -> bool:
    target_num = int(2 * 1024 * 1024 / hugepage_size_kb)
    print(color_str("trying to reserve huge pages ...", 33))
    cmd1 = ["echo", "1", ">", "/proc/sys/vm/drop_caches"]
    cmd2 = ["echo", "1", ">", "/proc/sys/vm/compact_memory"]
    os.system(" ".join(cmd1))
    os.system(" ".join(cmd2))
    # reserving 2x 1GB huge pages on each node
    for nid in range(num_numa_nodes):
        cmd = ["echo", str(target_num), ">", get_huge_page_sysfs(nid)]
        os.system(" ".join(cmd))
    # check
    all_good = True
    num_huge_pages = check_huge_pages(num_numa_nodes, color_str("after", 33))
    for nid in range(len(num_huge_pages)):
        if num_huge_pages[nid] != target_num:
            print(color_str(f"Fail to reserve huge pages on Node-{nid}", 31))
            all_good = False
    return all_good


def reset_huge_pages(num_huge_pages: List[int]):
    print(color_str("reverting changes to huge pages settings ...", 33))
    for nid in range(len(num_huge_pages)):
        cmd = ["echo", str(num_huge_pages[nid]), ">", get_huge_page_sysfs(nid)]
        os.system(" ".join(cmd))
    check_huge_pages(len(num_huge_pages), color_str("reset", 33))


def main(args):
    num_numa_nodes = print_cpu_info(do_print=False)
    print_mem_info(num_numa_nodes)
    check_huge_pages(num_numa_nodes, "current")
    if args.setup:
        setup_huge_pages(num_numa_nodes)
    if args.reset:
        reset_huge_pages([0 for _ in range(num_numa_nodes)])


def init_parser():
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("--check", "-c", action="store_true",
        help="check current hugepage setup")
    parser.add_argument("--setup", "-s", action="store_true",
        help="reserve 2x 1GB huge pages per node")
    parser.add_argument("--reset", "-r", action="store_true",
        help="un-reserve all 1GB huge pages")
    return parser


if __name__ == "__main__":
    parser = init_parser()
    args = parser.parse_args()
    main(args)
