#!/bin/bash

export TMOUT=

alias lt='ls -lhrt'

cp -n ./env/env_vimrc ~/.vimrc
cp -n ./env/env_inputrc ~/.inputrc

lscpu | grep "NUMA node" | grep CPU
dmidecode -t 17 | grep -A3 -e "Size.*GB" | grep -v "Volatile" | grep "GB"
dmidecode -t 17 | grep "Configured Memory Speed" | head -1 | grep "Memory Speed"

uname -r
cat /etc/os-release | grep PRETTY | grep NAME
