# MM-Mem

CPU memory benchmarking and characterization tools.

MM-Mem includes
- idle memory latency
- loaded memory latency
- peak bandwidth
- memcpy bandwidth
- NUMA latency/bandwidth matrix
- branch throughput

See [CONTRIBUTING.md](CONTRIBUTING.md) for contribution and release guidance,
and [CHANGELOG.md](CHANGELOG.md) for release history.

## Build
```
./scripts/install_deps.py
make
```

## Run

### Run All Tests
```
./scripts/run_cpu_micro.py
```

#### Example Output
```
-------- Running MM-Mem --------
---- Running Idle Latency test ...
trying to reserve huge pages ...
1GB huge pages - after
         2
threads:           1
region size in KB: 524288
chunk size in KB:  524288
stride size in B:  128
access pattern:    2 - random in full region
use hugepage:      3 - 1GB huge page
target duration:   10
Idle Latency        Node-0
Node-0              115.2

reverting changes to huge pages settings ...
1GB huge pages - reset
         0
---- Running Peak Bandwidth test ...
threads:           72
region size in KB: 131072
read/write mix:    100 - sweep read/write ratio
target duration:   10
timer <setup> elapsed: 507.5 ms
           all reads :  65478.45 MB/s |   63.94 GB/s
      3:1 read/write :  62539.91 MB/s |   61.07 GB/s
      2:1 read/write :  61574.67 MB/s |   60.13 GB/s
      1:1 read/write :  60788.73 MB/s |   59.36 GB/s

---- Running MemCpy test - Large ...
threads:           72
region size in KB: 131072
copy size in KB:   131072
target duration:   10
timer <setup> elapsed: 996.3 ms
Memcpy Bandwidth:   30965.55 MB/s |   30.24 GB/s

---- Running MemCpy test - Medium ...
threads:           72
region size in KB: 131072
copy size in KB:   16
target duration:   10
timer <setup> elapsed: 994.7 ms
Memcpy Bandwidth:   20472.51 MB/s |   19.99 GB/s

---- Running Loaded Latency test ...
trying to reserve huge pages ...
1GB huge pages - after
         2
threads:           72
region size in KB: 131072
chunk size in KB:  64
stride size in B:  128
access pattern:    1 - random in chunk
use hugepage:      0 - No huge page
read/write mix:    2 - 2:1 read/write
target duration:   10
timer <setup> elapsed: 523.3 ms
       delay   bandwidth     latency
           1        60.5      1031.9
           8        60.3       877.9
          32        60.4       662.3
          48        60.5       734.7
          64        60.5       638.1
          80        60.4       624.7
          88        61.1       431.4
          96        60.9       400.5
         104        60.5       339.5
         112        60.9       360.9
         128        59.6       319.0
         160        49.4       178.1
         192        41.3       154.9
         224        35.8       144.5
         256        31.4       138.8
         320        25.5       131.8
         384        21.4       123.9
         448        18.3       123.9
         512        16.4       119.4
         640        13.4       117.0
         768        11.4       110.5
        1024         8.8       112.9

reverting changes to huge pages settings ...
1GB huge pages - reset
         0
```

### Individual Test - Idle Latency
Option 1 - Use huge page and random-in-full-region pattern
```
./scripts/config_huge_page.py -s
./bin/cpu_idle_latency --region_size 524288 --access_pattern 2 --use_hugepage 3 --target_duration 10
./scripts/config_huge_page.py -r
```
This setup a 512MB region using 1GB pages, randomly pointer-chase through the whole region for 10 seconds.

Option 2 - Use regular 4K pages and random-in-chunk pattern
```
./bin/cpu_idle_latency --region_size 524288 --chunk_size 128 --access_pattern 1 --target_duration 10
```
This setup a 512MB region using 4KB pages, randomly pointer-chase through first 128KB chunk and then move to the next chunk.
128KB chunk size roughly matches what L1DTLB can cover; aggressive page prefetching and buddy prefetching can partially crack this.

### Individual Test - Peak Bandwidth
Use nproc number of threads, each streaming through a 128MB region; sweep different read/write ratios
```
./bin/cpu_peak_bandwidth --region_size 131072 --target_duration 10
```

### Individual Test - Loaded Latency
Use 1 latency thread to do random-in-chunk pointer-chasing; other nproc-1 threads generating variable load with 2:1 read/write mix
```
./scripts/config_huge_page.py -s
./bin/cpu_loaded_latency --region_size 131072 --access_pattern 2 --use_hugepage 3 --read_write_mix 2 --target_duration 10
./scripts/config_huge_page.py -r
```

Again, if huge page is not available, we can use random-in-chunk pattern
```
./bin/cpu_loaded_latency --region_size 131072 --chunk_size 128 --access_pattern 1 --read_write_mix 2 --target_duration 10
```

### Individual Test - Branch Throughput

```sh
./bin/cpu_branch_throughput -t 10
./bin/cpu_branch_throughput --branch_case 2 -t 5
```

This benchmark measures predictable **taken** branches using explicit x86-64 or
AArch64 assembly. It sweeps unconditional and conditional forward-branch chains
with 8, 32, or 128 branches and 16- or 64-byte target spacing. Branches skip the
padding between targets. Arithmetic occurs only once per chain, so dependent
additions no longer limit every branch as they did in the original C++ loop.

| Cases | Branch type | Target spacing | Chain lengths, in case order |
| --- | --- | --- | --- |
| 1–3 | Unconditional | 16 bytes | 8, 32, 128 |
| 4–6 | Unconditional | 64 bytes | 8, 32, 128 |
| 7–9 | Conditional, always taken | 16 bytes | 8, 32, 128 |
| 10–12 | Conditional, always taken | 64 bytes | 8, 32, 128 |

`--branch_case 0` (the default) runs the full sweep. `-t` is the total measurement
budget per worker, split equally among selected cases; warmup and batch completion
add some overhead. Each row reports:

- `case`, `thread`: case number and worker index.
- `chain`, `spacing`: forward branches per chain and target spacing in bytes.
- `GBr/s`: billions of taken branches per elapsed second for that worker.
- `Br/cycle`, `cycles/Br`: taken branches per measured user-space CPU cycle and
  its reciprocal. Linux uses a per-thread hardware cycle counter, without sudo
  when permissions allow. These columns show `n/a` when counters are unavailable
  or multiplexed; macOS currently reports timed throughput only.

Counts include the loop-back branch when taken and exclude its final not-taken
exit. They are derived from the assembly's known execution count, rather than a
hardware branch counter. Hardware cycles cover the measured batches and small
amounts of timer/control overhead. They are CPU cycles, not reference-clock ticks.

The summary shows the best observed GBr/s and Br/cycle independently across the
selected layouts. This is a practical estimate of peak predictable taken-branch
throughput, not a guarantee of the hardware maximum for every branch pattern.
It does not measure misprediction penalty, indirect branches, or not-taken branch
throughput. Use one pinned worker (the default) for a per-core comparison; with
`-n`, rows and maxima remain per worker rather than aggregate rates. Repeat runs
on an otherwise idle machine, and record the CPU, compiler, and VM environment.
Results are not directly comparable to the old arithmetic-dependent loop.
