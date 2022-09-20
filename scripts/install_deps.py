#!/usr/bin/env python3

import os
import platform
import shutil

cmd = []

if platform.system() == "Linux":
    pkg_list = ["gcc", "gcc-c++", "cmake3", "numactl-devel", "boost-devel"]
    if shutil.which("dnf"):
        pkg_manager = "dnf"
    elif shutil.which("yum"):
        pkg_manager = "yum"
    else:
        print("dnf | yum not available?!")
        exit(1)
    cmd = [pkg_manager, "install", "-y"] + pkg_list

elif platform.system() == "Darwin":
    pkg_list = ["cmake", "boost"]
    cmd = ["brew", "install"] + pkg_list

else:
    print(f"unknown platform: {platform.system()}")
    exit(1)

os.system(" ".join(cmd))

