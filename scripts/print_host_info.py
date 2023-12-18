#!/usr/bin/env python3

from utils import run_proc_simple


def color_str(input_str: str, color_code: int) -> str:
    return f"\033[0;{color_code}m{input_str}\033[0m"


# cpu
def get_cpu_info(do_print=True) -> int:
    for_real = True
    print_cmd = False
    num_numa_nodes = 1
    cmd = ["lscpu"]
    stdout = run_proc_simple(cmd, for_real, print_cmd)
    for line in stdout.splitlines():
        if line.startswith("CPU(s)"):
            items = line.split()
            if do_print:
                print(color_str(f"Total CPUs:\t\t{items[1]}", 32))
        elif line.startswith("NUMA node(s)"):
            num_numa_nodes = int(line.split()[-1])
        elif line.startswith("NUMA") and "CPU" in line:
            if do_print:
                print(line)
    return num_numa_nodes


# memory
def get_mem_info(do_print=True):
    for_real = True
    print_cmd = False
    cmd = ["lsmem"]
    stdout = run_proc_simple(cmd, for_real, print_cmd)
    for line in stdout.splitlines():
        if line.startswith("Total online memory"):
            if do_print:
                print(color_str(line, 32))
    cmd = ["numastat", "-m"]
    stdout = run_proc_simple(cmd, for_real, print_cmd)
    columns = [f"Node {x}" for x in range(get_cpu_info(do_print=False))]
    columns.append("Total")
    if do_print:
        print(" ".join([f"{x:>10s}" for x in columns]))
    for line in stdout.splitlines():
        if line.startswith("MemTotal"):
            items = line.split()
            mem_total = [round(float(x) / 1024) for x in items[1:]]
            if do_print:
                print(" ".join([f"{x:>9d}G" for x in mem_total]))
            return mem_total[:-1]
    return []


# gpu
def get_gpu_info():
    for_real = True
    print_cmd = False
    nvidia_gpus = []
    cmd = ["lspci"]
    stdout = run_proc_simple(cmd, for_real, print_cmd)
    for line in stdout.splitlines():
        if "controller: NVIDIA" in line:
            nvidia_gpus.append(line)
    print(color_str(f"Total NVidia GPUs: {len(nvidia_gpus)}", 32))
    gpus = []
    cmd = ["lspci", "-vv"]
    stdout = run_proc_simple(cmd, for_real, print_cmd)
    index = 0
    curr_device = ""
    for line in stdout.splitlines():
        if index < len(nvidia_gpus) and (nvidia_gpus[index] in line):
            curr_device = line.rstrip()
            index += 1
        if curr_device:
            gpus.append(curr_device)
            curr_device = ""
    for gpu in gpus:
        print(gpu)


# nic
def get_nic_info():
    for_real = True
    print_cmd = False
    nics = []
    cmd = ["lspci"]
    stdout = run_proc_simple(cmd, for_real, print_cmd)
    for line in stdout.splitlines():
        if "Ethernet" in line or "Infiniband" in line:
            nics.append(line)
    print(color_str(f"Total NICs: {len(nics)}", 32))
    nics = []
    cmd = ["lspci", "-vv"]
    stdout = run_proc_simple(cmd, for_real, print_cmd)
    index = 0
    curr_device = ""
    for line in stdout.splitlines():
        if index < len(nics) and (nics[index] in line):
            curr_device = line.rstrip()
            index += 1
        if curr_device:
            nics.append(curr_device)
            curr_device = ""
    for nic in nics:
        print(nic)


if __name__ == "__main__":
    get_cpu_info()
    get_mem_info()
    get_gpu_info()
    get_nic_info()
