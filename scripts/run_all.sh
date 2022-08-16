#!/bin/bash

../scripts/setup_hugepage.sh

./build/micro_benchmark/test_idle_latency --region_size 524288 --access_pattern 2 --use_hugepage 2 --target_duration 10

./build/micro_benchmark/test_idle_latency --region_size 524288 --chunk_size 128 --access_pattern 1 --target_duration 10

./build/micro_benchmark/test_peak_bandwidth --region_size 131072 --target_duration 10

./build/micro_benchmark/test_loaded_latency --region_size 131072 --access_pattern 2 --use_hugepage 2 --read_write_mix 2 --target_duration 10
