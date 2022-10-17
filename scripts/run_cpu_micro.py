#!/usr/bin/env python3

import os
from typing import List
from utils import run_proc_simple, read_env
from print_host_info import color_str, print_cpu_info, print_mem_info
from config_huge_page import check_huge_pages, setup_huge_pages, reset_huge_pages


def reserve_huge_page(num_numa_nodes: int) -> bool:
    num_retry = 2
    while num_retry > 0:
        num_retry -= 1
        all_good = setup_huge_pages(num_numa_nodes)
        if all_good:
            return True
    print(color_str("could not reserve huge pages", 33))
    print(color_str("using random-in-page; latency could be under-estimated", 33))
    return False


def get_bin_path(test_name: str) -> str:
    return os.path.join(read_env()["ROOT"], "bin", test_name)


def run_idle_latency(num_numa_nodes: int, num_huge_pages_orig: int):
    print(color_str("---- Running Idle Latency test ...", 32))
    all_good = reserve_huge_page(num_numa_nodes)
    if all_good:
        cmd = [get_bin_path("cpu_idle_latency"), "--latency_matrix", "-p", "2", "-H", "2"]
    else:
        cmd = [get_bin_path("cpu_idle_latency"), "--latency_matrix"]
    os.system(" ".join(cmd))
    reset_huge_pages(num_huge_pages_orig)


def run_peak_bandwidth(num_numa_nodes: int):
    print(color_str("---- Running Peak Bandwidth test ...", 32))
    cmd = [get_bin_path("cpu_peak_bandwidth")]
    os.system(" ".join(cmd))


def run_memcpy(num_numa_nodes: int):
    print(color_str("---- Running MemCpy test - Large ...", 32))
    cmd = [get_bin_path("cpu_memcpy")]
    os.system(" ".join(cmd))
    print(color_str("---- Running MemCpy test - Medium ...", 32))
    cmd = [get_bin_path("cpu_memcpy"), "-f", "16384"]
    os.system(" ".join(cmd))


def run_loaded_latency(num_numa_nodes: int, num_huge_pages_orig: int):
    print(color_str("---- Running Loaded Latency test ...", 32))
    all_good = reserve_huge_page(num_numa_nodes)
    if all_good:
        cmd = [get_bin_path("cpu_loaded_latency"), "-p", "2", "-H", "2"]
    else:
        cmd = [get_bin_path("cpu_loaded_latency"),]
    os.system(" ".join(cmd))
    reset_huge_pages(num_huge_pages_orig)


if __name__ == "__main__":
    num_numa_nodes = print_cpu_info()
    print_mem_info(num_numa_nodes)
    num_huge_pages_orig = check_huge_pages(num_numa_nodes, color_str("before", 33))
    print(color_str("-------- Running MM-Mem --------", 35))
    run_idle_latency(num_numa_nodes, num_huge_pages_orig)
    run_peak_bandwidth(num_numa_nodes)
    run_memcpy(num_numa_nodes)
    run_loaded_latency(num_numa_nodes, num_huge_pages_orig)
