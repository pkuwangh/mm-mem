#!/bin/bash

export TMOUT=

alias lt='ls -lhrt'

MM_MEM_ROOT="$(dirname "$(realpath "${BASH_SOURCE[0]}")")"
cp -n "${MM_MEM_ROOT}/env/env_vimrc" ~/.vimrc
cp -n "${MM_MEM_ROOT}/env/env_inputrc" ~/.inputrc

lscpu | grep "NUMA node" | grep CPU
dmidecode -t 17 | grep -A3 -e "Size.*GB" | grep -v "Volatile" | grep "GB"
dmidecode -t 17 | grep "Configured Memory Speed" | head -1 | grep "Memory Speed"

uname -r
cat /etc/os-release | grep PRETTY | grep NAME
