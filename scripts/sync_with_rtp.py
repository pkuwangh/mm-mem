#!/usr/bin/env python3

import argparse
import os

from utils import exec_cmd, run_proc, read_env


def main(args):
    proj_path = read_env()['ROOT'].rstrip('/')
    proj_name = os.path.basename(proj_path)
    target_path = os.path.join(args.target_path, proj_name)
    for hostname in args.hostname.split(','):
        if hostname.startswith('twsh') or hostname.startswith('rtptest'):
            remote_path = f'root@{hostname}:{target_path}'
        else:
            remote_path = f'{hostname}:{target_path}'
        # copy the repo myself
        if args.download:
            cmd = [
                'rsync', '-rvl',
                '--exclude', '.*swp',
                '--exclude', 'build',
                f'{remote_path}/', proj_path
            ]
        else:
            cmd = [
                'rsync', '-rvl',
                '--exclude', '.*swp',
                '--exclude', '.git',
                f'{proj_path}/', remote_path
            ]
        exec_cmd(cmd, for_real=True, print_cmd=True)


def init_parser():
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('--hostname', '-r', type=str, required=True,
        help='target hostname; comma-separated for multiple hosts')
    parser.add_argument('--target-path', '-t', type=str, default="/root/haowang3/",
        help='target path')
    parser.add_argument('--download', '-d', action='store_true',
        help='download')
    return parser


if __name__ == '__main__':
    parser = init_parser()
    args = parser.parse_args()
    my_name = os.path.basename(__file__)
    if my_name.startswith('upload'):
        args.download = False
    if my_name.startswith('download'):
        args.download = True;
    main(args)
