# mm-mem
Memory benchmarking and characterization tools

## Build
```
  ./scripts/install_deps.py
  cd build/
  make
```

## Run
### Idle Latency
Option 1 - Use regular 4K pages and random-in-chunk pattern
```
  ./bin/cpu_idle_latency --region_size 524288 --chunk_size 128 --access_pattern 1 --target_duration 10
```
This setup a 512MB region using 4KB pages, randomly pointer-chase through first 128KB chunk and then move to the next chunk.
128KB chunk size roughly matches what L1DTLB can cover; aggressive page prefetching and buddy prefetching can partially crack this.

Option 2 - Use huge page and random-in-full-region pattern
```
  ../scripts/setup_hugepage.sh
  ./bin/cpu_idle_latency --region_size 524288 --access_pattern 2 --use_hugepage 2 --target_duration 10
```
This setup a 512MB region using 1GB pages, randomly pointer-chase through the whole region for 10 seconds.

### Peak Bandwidth
Use nproc number of threads, each streaming through a 128MB region; sweep different read/write ratios
```
  ../scripts/reset_hugepage.sh
  ./bin/cpu_peak_bandwidth --region_size 131072 --target_duration 10
```

### Loaded Latency
Use 1 latency thread to do random-in-chunk pointer-chasing; other nproc-1 threads generating variable load with 2:1 read/write mix
```
  ./bin/cpu_loaded_latency --region_size 131072 --chunk_size 128 --access_pattern 1 --read_write_mix 2 --target_duration 10
```

Again, we can use huge page for latency thread
```
  ../scripts/setup_hugepage.sh
  ./bin/cpu_loaded_latency --region_size 131072 --access_pattern 2 --use_hugepage 2 --read_write_mix 2 --target_duration 10
```
