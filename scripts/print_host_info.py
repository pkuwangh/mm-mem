#!/usr/bin/env python3

"""Report CPU and NUMA memory information without requiring external utilities.

Linux topology and memory capacity come from sysfs/procfs, retaining actual node
IDs. Optional GPU and network PCI inventory uses lspci when installed. Missing
inventory tools do not prevent CPU benchmarks from running.
"""

import os
from pathlib import Path
import re
import shutil
import subprocess

NODE_ROOT = Path('/sys/devices/system/node')


def color_str(input_str: str, color_code: int) -> str:
    return f'\033[0;{color_code}m{input_str}\033[0m'


def node_paths():
    return sorted((path for path in NODE_ROOT.glob('node[0-9]*')
                   if re.fullmatch(r'node\d+', path.name)),
                  key=lambda path: int(path.name[4:]))


def get_cpu_info(do_print=True) -> int:
    nodes = node_paths()
    if do_print:
        print(f'Total CPUs: {os.cpu_count()}')
        if hasattr(os, 'sched_getaffinity'):
            print(f'Allowed CPUs: {len(os.sched_getaffinity(0))}')
        for node in nodes:
            try:
                print(f'NUMA {node.name} CPUs: {(node / "cpulist").read_text().strip()}')
            except OSError:
                print(f'NUMA {node.name} CPUs: unavailable')
    return len(nodes)


def get_mem_info(do_print=True):
    """Return {actual_node_id: total_memory_KiB}, excluding unreadable nodes."""
    memory = {}
    for node in node_paths():
        try:
            match = re.search(r'MemTotal:\s+(\d+)\s+kB', (node / 'meminfo').read_text())
            if match:
                memory[int(node.name[4:])] = int(match.group(1))
        except OSError:
            pass
    if do_print:
        if not memory:
            print('NUMA memory information unavailable')
        for nid, kib in memory.items():
            print(f'Node {nid} memory: {kib / (1024 ** 2):.2f} GiB')
    return memory


def pci_devices():
    if not shutil.which('lspci'):
        print('PCI inventory unavailable (optional lspci is not installed)')
        return []
    result = subprocess.run(['lspci'], stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                            universal_newlines=True)
    if result.returncode:
        print('PCI inventory unavailable: ' + result.stderr.strip())
        return []
    return result.stdout.splitlines()


def get_gpu_info():
    for line in pci_devices():
        if 'NVIDIA' in line and ('VGA' in line or '3D controller' in line):
            print(line)


def get_nic_info():
    for line in pci_devices():
        if 'Ethernet' in line or 'InfiniBand' in line:
            print(line)


if __name__ == '__main__':
    get_cpu_info()
    get_mem_info()
    get_gpu_info()
    get_nic_info()
