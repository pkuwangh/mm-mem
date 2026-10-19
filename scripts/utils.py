#!/usr/bin/env python3

"""Provide shared command-execution and repository-path helpers for scripts.

Wrap argument-list commands and subprocesses with optional command logging, dry runs,
environment overrides, and captured or file-redirected output. read_env returns
the repository root derived from this file's location and the user's HOME.
This module is intended for import and has no standalone command-line action."""

import os
import subprocess
from typing import Dict, List, Optional


def exec_cmd(
    cmd: List[str],
    for_real: bool,
    print_cmd: bool = True,
) -> None:
    cmd_str = " ".join(cmd)
    if print_cmd:
        print(cmd_str)
    if for_real:
        subprocess.run(cmd, check=True)


def run_proc(
    cmd: List[str],
    cwd: str,
    env: Dict[str, str],
    outfile: Optional[str],
    for_real: bool,
    print_cmd: bool = True,
) -> Optional[str]:
    env_setting = [f"{k}={v}" for k, v in env.items()]
    if print_cmd:
        print(" ".join(env_setting + cmd))
    if not for_real:
        return None
    exec_env = os.environ.copy()
    exec_env.update(env)
    if outfile:
        with open(outfile, "wt") as output:
            subprocess.run(cmd, cwd=cwd, env=exec_env, stdout=output,
                           stderr=subprocess.STDOUT, check=True)
        return None
    result = subprocess.run(cmd, cwd=cwd, env=exec_env, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, check=True, universal_newlines=True)
    return result.stdout


def run_proc_simple(
    cmd: List[str],
    for_real: bool = True,
    print_cmd: bool = False,
) -> Optional[str]:
    return run_proc(cmd, ".", {}, None, for_real, print_cmd)


def read_env() -> Dict[str, str]:
    # default env values
    env_vars = {}
    env_vars["ROOT"] = "/".join(os.path.abspath(__file__).split("/")[:-2])
    env_vars["HOME"] = os.getenv("HOME")
    return env_vars
