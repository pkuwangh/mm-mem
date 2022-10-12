#!/usr/bin/env python3

import os
from typing import List
from utils import run_proc_simple, read_env
from print_host_info import color_str, print_cpu_info, print_mem_info


def get_bin_path(test_name: str) -> str:
    return os.path.join(read_env()["ROOT"], "bin", test_name)


def get_huge_page_sysfs(nid: int) -> str:
    return f"/sys/devices/system/node/node{nid}/hugepages/hugepages-1048576kB/nr_hugepages"


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
    print(color_str("trying to reserve huge pages ...", 33))
    cmd1 = ["echo", "1", ">", "/proc/sys/vm/drop_caches"]
    cmd2 = ["echo", "1", ">", "/proc/sys/vm/compact_memory"]
    run_proc_simple(cmd1)
    run_proc_simple(cmd2)
    # reserving 2x 1GB huge pages on each node
    for nid in range(num_numa_nodes):
        cmd = ["echo", "2", ">", get_huge_page_sysfs(nid)]
        os.system(" ".join(cmd))
    # check
    all_good = True
    num_huge_pages = check_huge_pages(num_numa_nodes, color_str("after", 33))
    for nid in range(len(num_huge_pages)):
        if num_huge_pages[nid] != 2:
            print(color_str(f"Fail to reserve huge pages on Node-{nid}", 31))
            all_good = False
    return all_good


def reset_huge_page(num_huge_pages: List[int]):
    print(color_str("reverting changes to huge pages settings ...", 33))
    for nid in range(len(num_huge_pages)):
        cmd = ["echo", str(num_huge_pages[nid]), ">", get_huge_page_sysfs(nid)]
        os.system(" ".join(cmd))
    check_huge_pages(len(num_huge_pages), color_str("reset", 33))


def run_idle_latency(num_numa_nodes: int):
    print(color_str("Running Idle Latency test ...", 32))
    num_huge_pages_orig = check_huge_pages(num_numa_nodes, color_str("before", 33))
    all_good = setup_huge_pages(num_numa_nodes)
    if all_good:
        cmd = [get_bin_path("cpu_idle_latency"), "-c", "524288", "-H", "2"]
    else:
        print(color_str("could not reserve huge pages", 33))
        print(color_str("using random-in-page; latency could be under-estimated", 33))
        cmd = [get_bin_path("cpu_idle_latency")]
    os.system(" ".join(cmd))
    reset_huge_page(num_huge_pages_orig)


def run_peak_bandwidth(num_numa_nodes: int):
    print(color_str("Running Peak Bandwidth test ...", 32))
    cmd = [get_bin_path("cpu_peak_bandwidth")]
    os.system(" ".join(cmd))


def run_memcpy(num_numa_nodes: int):
    print(color_str("Running MemCpy test ...", 32))
    cmd = [get_bin_path("cpu_memcpy")]
    os.system(" ".join(cmd))


def run_loaded_latency(num_numa_nodes: int):
    print(color_str("Running Loaded Latency test ...", 32))
    cmd = [get_bin_path("cpu_loaded_latency"), "-t", "4"]
    os.system(" ".join(cmd))


if __name__ == "__main__":
    num_numa_nodes = print_cpu_info()
    print_mem_info(num_numa_nodes)
    print(color_str("-------- Running MM-Mem --------", 35))
    run_idle_latency(num_numa_nodes)
    run_peak_bandwidth(num_numa_nodes)
    run_memcpy(num_numa_nodes)
    run_loaded_latency(num_numa_nodes)
