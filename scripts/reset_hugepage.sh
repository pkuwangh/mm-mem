#!/bin/bash

echo 0 > /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages
echo 0 >/sys/devices/system/node/node0/hugepages/hugepages-1048576kB/nr_hugepages

cat /proc/meminfo | grep -i huge

echo "2MB huge pages: " `cat /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages`
echo "1GB huge pages: " `cat /sys/devices/system/node/node0/hugepages/hugepages-1048576kB/nr_hugepages`
