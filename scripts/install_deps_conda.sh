#!/usr/bin/env bash
# Create a user-owned conda-forge build environment without sudo.
# Accepts an existing conda, mamba, or micromamba installation; does not bootstrap it.
set -euo pipefail
env_name=mm-mem
manager=""
dry_run=0
while (( $# )); do
    case "$1" in
        --name|--manager)
            if (( $# < 2 )); then echo "Missing value for $1" >&2; exit 2; fi
            if [[ $1 == --name ]]; then env_name=$2; else manager=$2; fi
            shift 2 ;;
        --dry-run) dry_run=1; shift ;;
        -h|--help)
            echo "Usage: $0 [--name mm-mem] [--manager conda|mamba|micromamba] [--dry-run]"
            exit 0 ;;
        *) echo "Unknown argument: $1" >&2; exit 2 ;;
    esac
done
if [[ ! $env_name =~ ^[a-zA-Z0-9_][a-zA-Z0-9_.-]*$ ]]; then
    echo "Use a simple environment name, not a filesystem path." >&2; exit 2
fi
if [[ -z $manager ]]; then
    for candidate in micromamba mamba conda; do
        if command -v "$candidate" >/dev/null 2>&1; then manager=$candidate; break; fi
    done
fi
case "$manager" in
    conda|mamba|micromamba) ;;
    *) echo "Install Miniforge or micromamba first, or select --manager." >&2; exit 1 ;;
esac
if (( ! dry_run )) && ! command -v "$manager" >/dev/null 2>&1; then
    echo "$manager is not available on PATH." >&2; exit 1
fi
packages=(cmake make c-compiler cxx-compiler python)
case $(uname -s) in
    Linux) packages+=(libnuma numactl) ;;
    Darwin) ;;
    *) echo "Unsupported platform." >&2; exit 1 ;;
esac
operation=create
if (( ! dry_run )) && "$manager" run --name "$env_name" true >/dev/null 2>&1; then
    operation=install
fi
command=("$manager" "$operation" --name "$env_name" --override-channels --channel conda-forge --strict-channel-priority -y "${packages[@]}")
printf '%q ' "${command[@]}"
printf '\n'
if (( ! dry_run )); then "${command[@]}"; fi
activation_manager=$manager
if [[ $manager == mamba ]]; then activation_manager=conda; fi
echo "Next: $activation_manager activate $env_name"
echo 'Configure a fresh build directory after activation; see README.md.'
echo 'This environment does not grant permission to change host settings or access restricted perf counters.'
