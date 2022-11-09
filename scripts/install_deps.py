#!/usr/bin/env python3

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
        pkg_list = ["cmake3", "gcc", "gcc-c++", "numactl-devel", "boost-devel"]
    elif pkg_manager == "apt":
        pkg_list = [
            "numactl",
            "cmake",
            "gcc",
            "g++",
            "libnuma-dev",
            "libboost-program-options-dev",
        ]
    cmd = ["sudo", pkg_manager, "install", "-y"] + pkg_list

elif platform.system() == "Darwin":
    pkg_list = ["cmake", "boost"]
    cmd = ["brew", "install"] + pkg_list

else:
    print(f"unknown platform: {platform.system()}")
    exit(1)

os.system(" ".join(cmd))
