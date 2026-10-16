#!/usr/bin/env python3

"""Install MM-Mem build dependencies using the host's package manager.

On Linux, select dnf, yum, or apt and invoke sudo to install CMake, GCC, and NUMA
packages. On macOS, install CMake through Homebrew. This script executes package
installation immediately when run or imported; it has no command-line options."""

import os
import platform
import shutil

cmd = []

if platform.system() == "Linux":
    if shutil.which("dnf"):
        pkg_manager = "dnf"
    elif shutil.which("yum"):
        pkg_manager = "yum"
    elif shutil.which("apt"):
        pkg_manager = "apt"
    else:
        print("dnf | yum | apt not available?!")
        exit(1)
    if pkg_manager == "dnf" or pkg_manager == "yum":
        pkg_list = ["cmake3", "gcc", "gcc-c++", "numactl-devel"]
    elif pkg_manager == "apt":
        pkg_list = [
            "numactl",
            "cmake",
            "gcc",
            "g++",
            "libnuma-dev",
        ]
    cmd = ["sudo", pkg_manager, "install", "-y"] + pkg_list

elif platform.system() == "Darwin":
    pkg_list = ["cmake"]
    cmd = ["brew", "install"] + pkg_list

else:
    print(f"unknown platform: {platform.system()}")
    exit(1)

os.system(" ".join(cmd))
