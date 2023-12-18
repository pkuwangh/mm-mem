#!/usr/bin/env python3

import argparse
import os
import sys
from typing import Dict, List

from config_huge_page import check_huge_pages, reset_huge_pages, setup_huge_pages
from config_sysfs_settings import check_autonuma, setup_autonuma
from print_host_info import color_str, get_cpu_info, get_mem_info
from utils import read_env


def get_huge_page_mapping(size: int) -> int:
    if size == (2 << 10):
        return 1
    elif size == (512 << 10):
        return 2
    elif size == (1 << 20):
        return 3
    elif size == (16 << 20):
        return 4
    else:
        print(color_str(f"Unexpected huge page size of {size}kB", 31))
        return 0


def reserve_huge_page() -> (bool, int):
    num_retry = 2
    while num_retry > 0:
        num_retry -= 1
        all_good, size = setup_huge_pages()
        if all_good and size > 0:
            return (True, get_huge_page_mapping(size))
    print(color_str("could not reserve huge pages to use full-random pattern", 33))
    sys.stdout.flush()
    return (False, 0)


def get_bin_path(test_name: str) -> str:
    return os.path.join(read_env()["ROOT"], "bin", test_name)


def run_idle_latency(huge_page_state: Dict[int, List[int]]):
    print(color_str("---- Running Idle Latency test ...", 32))
    sys.stdout.flush()
    # not using huge page
    cmd = [
            get_bin_path("cpu_idle_latency"),
            "--latency_matrix",
            "-t", str(args.target_duration),
          ]
    os.system(" ".join(cmd))
    # using huge page
    all_good, huge_page_option = reserve_huge_page()
    if all_good:
        cmd = [
                get_bin_path("cpu_idle_latency"),
                "--latency_matrix",
                "-t", str(args.target_duration),
                "-p", "2",
                "-H", str(huge_page_option),
               ]
        os.system(" ".join(cmd))
    reset_huge_pages(huge_page_state)


def run_peak_bandwidth(num_numa_nodes: int):
    print(color_str("---- Running Peak Bandwidth test ...", 32))
    sys.stdout.flush()
    cmd = [
            get_bin_path("cpu_peak_bandwidth"),
            "-t", str(args.target_duration),
          ]
    os.system(" ".join(cmd))
    if num_numa_nodes > 0:
        print(color_str("---- Running Bandwidth Matrix test - All Reads ...", 32))
        sys.stdout.flush()
        cmd = [
                get_bin_path("cpu_peak_bandwidth"),
                "--bandwidth_matrix",
                "-t", str(args.target_duration),
              ]
        os.system(" ".join(cmd))
        print(color_str("---- Running Bandwidth Matrix test - 1:1 Read/Write ...", 32))
        sys.stdout.flush()
        cmd = [
                get_bin_path("cpu_peak_bandwidth"),
                "--bandwidth_matrix",
                "-t", str(args.target_duration),
                "-m", "1",
              ]
        os.system(" ".join(cmd))


def run_memcpy(num_numa_nodes: int):
    print(color_str("---- Running MemCpy test - Large ...", 32))
    sys.stdout.flush()
    cmd = [
            get_bin_path("cpu_memcpy"),
            "-t", str(args.target_duration),
          ]
    os.system(" ".join(cmd))
    print(color_str("---- Running MemCpy test - Medium ...", 32))
    sys.stdout.flush()
    cmd = [
            get_bin_path("cpu_memcpy"),
            "-t", str(args.target_duration),
            "-f", "16384",
          ]
    os.system(" ".join(cmd))


def run_loaded_latency(huge_page_state: Dict[int, List[int]]):
    print(color_str("---- Running Loaded Latency test ...", 32))
    sys.stdout.flush()
    # not using huge page
    cmd = [
        get_bin_path("cpu_loaded_latency"),
        "-t", str(args.target_duration),
    ]
    os.system(" ".join(cmd))
    # using huge page
    all_good, huge_page_option = reserve_huge_page()
    if all_good:
        cmd = [
                get_bin_path("cpu_loaded_latency"),
                "-t", str(args.target_duration),
                "-p", "2",
                "-H", str(huge_page_option),
              ]
        os.system(" ".join(cmd))
    reset_huge_pages(huge_page_state)


def init_parser():
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument(
        "--target-duration", "-t", type=int, default=2,
        help="duration in seconds for each data point"
    )
    parser.add_argument(
        "--test", action="append", default=None,
        choices=["idle_latency", "loaded_latency", "bandwidth"],
        help="selective run certain tests; default to run all"
    )
    return parser


def main(args):
    num_numa_nodes = get_cpu_info()
    get_mem_info(num_numa_nodes)
    huge_page_state = check_huge_pages(color_str("before", 33))
    autonuma_state = check_autonuma(color_str("before", 33))
    # change autonuma setting here and hugepage setting in each test
    if autonuma_state > 0:
        setup_autonuma(value=0)
    print(color_str("-------- Running MM-Mem --------", 35))
    sys.stdout.flush()
    if args.test is None or "idle_latency" in args.test:
        run_idle_latency(huge_page_state)
    if args.test is None or "bandwidth" in args.test:
        run_peak_bandwidth(num_numa_nodes)
        run_memcpy(num_numa_nodes)
    if args.test is None or "loaded_latency" in args.test:
        run_loaded_latency(huge_page_state)
    if autonuma_state > 0:
        setup_autonuma(value=autonuma_state)


if __name__ == "__main__":
    parser = init_parser()
    args = parser.parse_args()
    main(args)

