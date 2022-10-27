#!/usr/bin/env python3

import argparse
import os

from utils import exec_cmd, read_env


def main(args):
    proj_path = read_env()["ROOT"].rstrip("/")
    proj_name = os.path.basename(proj_path)
    target_path = os.path.join(args.target_path, proj_name)
    for hostname in args.hostname.split(","):
        if "." in hostname:
            remote_name = f"root@{hostname}"
        else:
            remote_name = hostname
        remote_path = f"{remote_name}:{target_path}"
        # create target dir
        if not args.download:
            cmd = ["ssh", remote_name, "mkdir", "-p", target_path]
            exec_cmd(cmd, for_real=True, print_cmd=True)
        # copy the repo myself
        exclude_list = [".*swp", "__pycache__", "build", "bin", ".git"]
        exclude_clause = []
        for item in exclude_list:
            exclude_clause += ["--exclude", item]
        if args.download:
            cmd = ["rsync", "-rlcv"] + exclude_clause + [f"{remote_path}/", proj_path]
        else:
            cmd = ["rsync", "-rlcv"] + exclude_clause + [f"{proj_path}/", remote_path]
        exec_cmd(cmd, for_real=True, print_cmd=True)


def init_parser():
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument(
        "--hostname",
        "-r",
        type=str,
        required=True,
        help="target hostname; comma-separated for multiple hosts",
    )
    parser.add_argument(
        "--target-path", "-t", type=str, default="~/", help="target path"
    )
    parser.add_argument("--download", "-d", action="store_true", help="download")
    return parser


if __name__ == "__main__":
    parser = init_parser()
    args = parser.parse_args()
    my_name = os.path.basename(__file__)
    if my_name.startswith("upload"):
        args.download = False
    if my_name.startswith("download"):
        args.download = True
    main(args)
