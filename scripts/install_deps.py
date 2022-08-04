#!/usr/bin/env python3

import os
import shutil

pkg_list = ["gcc", "gcc-c++", "cmake", "numactl-devel", "boost-devel"]

if shutil.which("dnf"):
    pkg_manager = "dnf"
elif shutil.which("yum"):
    pkg_manager = "yum"
else:
    print("dnf | yum not available?!")
    exit(1)

cmd = [pkg_manager, "install", "-y"] + pkg_list
os.system(" ".join(cmd))

