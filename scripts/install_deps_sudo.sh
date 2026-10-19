#!/usr/bin/env bash
# Install Linux build dependencies through apt-get, dnf, or yum using root/sudo.
# Optional reporting packages are installed only with --with-tools.
set -euo pipefail
with_tools=0
dry_run=0
for arg in "$@"; do
    case "$arg" in
        --with-tools) with_tools=1 ;;
        --dry-run) dry_run=1 ;;
        -h|--help)
            echo "Usage: $0 [--with-tools] [--dry-run]"
            echo "Install compiler, CMake, Make, Python 3, and NUMA development packages."
            exit 0 ;;
        *) echo "Unknown argument: $arg" >&2; exit 2 ;;
    esac
done
if [[ $(uname -s) != Linux ]]; then
    echo "This installer is for Linux. Use install_deps_conda.sh or your platform's package manager." >&2
    exit 1
fi
if command -v apt-get >/dev/null 2>&1; then
    manager=apt-get
    packages=(cmake make gcc g++ python3 libnuma-dev numactl)
    if (( with_tools )); then packages+=(sysstat pciutils procps); fi
elif command -v dnf >/dev/null 2>&1; then
    manager=dnf
    packages=(cmake make gcc gcc-c++ python3 numactl-devel numactl)
    if (( with_tools )); then packages+=(sysstat pciutils procps-ng perf); fi
elif command -v yum >/dev/null 2>&1; then
    manager=yum
    packages=(cmake3 make gcc gcc-c++ python3 numactl-devel numactl)
    if (( with_tools )); then packages+=(sysstat pciutils procps-ng perf); fi
else
    echo "No supported package manager found; use install_deps_conda.sh." >&2
    exit 1
fi
prefix=()
if (( EUID != 0 )); then
    prefix=(sudo)
    if (( ! dry_run )) && ! command -v sudo >/dev/null 2>&1; then
        echo "sudo is unavailable; use install_deps_conda.sh." >&2
        exit 1
    fi
fi
command=("${prefix[@]}" "$manager" install -y "${packages[@]}")
printf '%q ' "${command[@]}"
printf '\n'
if (( ! dry_run )); then "${command[@]}"; fi
if [[ $manager == yum ]]; then
    echo "If CMake is named cmake3, build with: make CMAKE=cmake3"
fi
if [[ $manager == apt-get ]] && (( with_tools )); then
    echo "Install perf separately using the kernel-tools package matching your distribution/kernel."
fi
