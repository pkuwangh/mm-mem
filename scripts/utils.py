#!/usr/bin/env python3

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
        os.system(cmd_str)


def _launch_proc(cmd, cwd, stdout, stderr, env):
    return subprocess.Popen(
        cmd,
        cwd=cwd,
        stdout=stdout,
        stderr=stderr,
        env=env,
    )


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
    exec_env = os.environ.copy()
    if for_real:
        for k, v in env.items():
            exec_env[k] = v
        if outfile:
            with open(outfile, "wt") as fp:
                proc = _launch_proc(cmd, cwd, fp, fp, exec_env)
                proc.wait()
            return None
        else:
            proc = _launch_proc(cmd, cwd, subprocess.PIPE, subprocess.STDOUT, exec_env)
            (stdout, _) = proc.communicate()
            return stdout.decode("utf-8")
    return None


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
