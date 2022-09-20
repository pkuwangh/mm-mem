#!/bin/bash

export TMOUT=

alias lt='ls -lhrt'

MM_MEM_ROOT="$(dirname "$(realpath "${BASH_SOURCE[0]:-${(%):-%x}}")")"
cp -n "${MM_MEM_ROOT}/env/env_vimrc" ~/.vimrc
cp -n "${MM_MEM_ROOT}/env/env_inputrc" ~/.inputrc

if [[ "$(uname)" == "Darwin" ]]; then
    cat /etc/os-release | grep "VERSION"
    BREW_ROOT="$(brew --prefix)"
    export CPATH="$BREW_ROOT/include"
    export LIBRARY_PATH="$BREW_ROOT/lib"
else
    lscpu | grep "NUMA node" | grep CPU
    dmidecode -t 17 | grep -A3 -e "Size.*GB" | grep -v "Volatile" | grep "GB"
    dmidecode -t 17 | grep "Configured Memory Speed" | head -1 | grep "Memory Speed"
    cat /etc/os-release | grep PRETTY | grep NAME
fi

uname -a
