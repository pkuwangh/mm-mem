#!/bin/bash

echo 1 > /proc/sys/vm/drop_caches
echo 1 > /proc/sys/vm/compact_memory

echo 2 > /sys/devices/system/node/node0/hugepages/hugepages-1048576kB/nr_hugepages
echo 1024  > /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages

cat /proc/meminfo | grep -i huge

echo "1GB huge pages: " `cat /sys/devices/system/node/node0/hugepages/hugepages-1048576kB/nr_hugepages`
echo "2MB huge pages: " `cat /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages`
