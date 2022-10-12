#!/usr/bin/env python3

from utils import run_proc_simple

def color_str(input_str: str, color_code: int) -> str:
    return f"\033[0;{color_code}m{input_str}\033[0m"


# cpu
def print_cpu_info() -> int:
    for_real = True
    print_cmd = False
    num_numa_nodes = 1
    cmd = ["lscpu"]
    stdout = run_proc_simple(cmd, for_real, print_cmd)
    for line in stdout.splitlines():
        if line.startswith("CPU(s)"):
            items = line.split()
            print(color_str(f"Total CPUs:\t\t{items[1]}", 32))
        elif line.startswith("NUMA node(s)"):
            num_numa_nodes = int(line.split()[-1])
        elif line.startswith("NUMA") and "CPU" in line:
            print(line)
    return num_numa_nodes


# memory
def print_mem_info(num_numa_nodes: int):
    for_real = True
    print_cmd = False
    cmd = ["lsmem"]
    stdout = run_proc_simple(cmd, for_real, print_cmd)
    for line in stdout.splitlines():
        if line.startswith("Total online memory"):
            print(color_str(line, 32))
    cmd = ["numastat", "-m"]
    stdout = run_proc_simple(cmd, for_real, print_cmd)
    columns = [f"Node {x}" for x in range(num_numa_nodes)]
    columns.append("Total")
    print(" ".join([f"{x:>10s}" for x in columns]))
    for line in stdout.splitlines():
        if line.startswith("MemTotal"):
            items = line.split()
            mem_total = [round(float(x) / 1024) for x in items[1:]]
            print(" ".join([f"{x:>9d}G" for x in mem_total]))


# gpu
def print_gpu_info(num_numa_nodes: int):
    for_real = True
    print_cmd = False
    nvidia_gpus = []
    cmd = ["lspci"]
    stdout = run_proc_simple(cmd, for_real, print_cmd)
    for line in stdout.splitlines():
        if "NVIDIA" in line:
            nvidia_gpus.append(line)
    print(color_str(f"Total NVidia GPUs: {len(nvidia_gpus)}", 32));
    per_node_gpus = {x : [] for x in range(num_numa_nodes)}
    cmd = ["lspci", "-vv"]
    stdout = run_proc_simple(cmd, for_real, print_cmd)
    index = 0
    curr_device = ""
    for line in stdout.splitlines():
        if index < len(nvidia_gpus) and (nvidia_gpus[index] in line):
            curr_device = line.rstrip()
            index += 1
        if curr_device:
            if "NUMA node" in line:
                items = line.split()
                per_node_gpus[int(items[-1])].append(curr_device)
                curr_device = ""
            elif "Kernel driver" in line:
                per_node_gpus[0].append(curr_device)
                curr_device = ""
    for k, v in per_node_gpus.items():
        if (len(v) > 0):
            print(f"Node-{k:<5d}{v[0]}")
        for x in v[1:]:
            print(" " * 10 + x)


# nic
def print_nic_info(num_numa_nodes: int):
    for_real = True
    print_cmd = False
    nics = []
    cmd = ["lspci"]
    stdout = run_proc_simple(cmd, for_real, print_cmd)
    for line in stdout.splitlines():
        if "Ethernet" in line:
            nics.append(line)
    print(color_str(f"Total NICs: {len(nics)}", 32));
    per_node_nics = {x : [] for x in range(num_numa_nodes)}
    cmd = ["lspci", "-vv"]
    stdout = run_proc_simple(cmd, for_real, print_cmd)
    index = 0
    curr_device = ""
    for line in stdout.splitlines():
        if index < len(nics) and (nics[index] in line):
            curr_device = line.rstrip()
            index += 1
        if curr_device:
            if "NUMA node" in line:
                items = line.split()
                per_node_nics[int(items[-1])].append(curr_device)
                curr_device = ""
            elif "Kernel driver" in line:
                per_node_nics[0].append(curr_device)
                curr_device = ""
    for k, v in per_node_nics.items():
        if (len(v) > 0):
            print(f"Node-{k:<5d}{v[0]}")
        for x in v[1:]:
            print(" " * 10 + x)


if __name__ == "__main__":
    num_numa_nodes = print_cpu_info()
    print_mem_info(num_numa_nodes)
    print_gpu_info(num_numa_nodes)
    print_nic_info(num_numa_nodes)
