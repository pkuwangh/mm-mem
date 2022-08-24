#!/bin/bash

echo 1 > /proc/sys/vm/drop_caches
echo 1 > /proc/sys/vm/compact_memory

echo 2 > /sys/devices/system/node/node0/hugepages/hugepages-1048576kB/nr_hugepages
if [ -f /sys/devices/system/node/node1/hugepages/hugepages-1048576kB/nr_hugepages ]; then
  echo 2 > /sys/devices/system/node/node1/hugepages/hugepages-1048576kB/nr_hugepages
fi
if [ -f /sys/devices/system/node/node2/hugepages/hugepages-1048576kB/nr_hugepages ]; then
  echo 2 > /sys/devices/system/node/node2/hugepages/hugepages-1048576kB/nr_hugepages
fi

# echo 1024  > /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages

cat /proc/meminfo | grep -i huge

echo "1GB huge pages on node0: " `cat /sys/devices/system/node/node0/hugepages/hugepages-1048576kB/nr_hugepages`
if [ -f /sys/devices/system/node/node1/hugepages/hugepages-1048576kB/nr_hugepages ]; then
  echo "1GB huge pages on node1: " `cat /sys/devices/system/node/node1/hugepages/hugepages-1048576kB/nr_hugepages`
fi
if [ -f /sys/devices/system/node/node2/hugepages/hugepages-1048576kB/nr_hugepages ]; then
  echo "1GB huge pages on node2: " `cat /sys/devices/system/node/node2/hugepages/hugepages-1048576kB/nr_hugepages`
fi

# echo "2MB huge pages: " `cat /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages`
