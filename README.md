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

Building and running ordinary-allocation benchmarks do not require sudo. On Linux,
install a C++14 compiler, CMake 3.16+, Make, Python 3.6+, and libnuma headers/library
using one of the following paths.

### System packages with sudo

```sh
./scripts/install_deps_sudo.sh
make JOBS=4
```

The Bash installer uses apt-get, dnf, or yum. It invokes sudo only when needed;
`--dry-run` prints the command, and `--with-tools` adds optional reporting tools.
On older yum systems where CMake is installed as `cmake3`, use `make CMAKE=cmake3`.

### Conda or micromamba without sudo

Start with an existing [Miniforge](https://github.com/conda-forge/miniforge) or
[micromamba](https://mamba.readthedocs.io/en/latest/installation/micromamba-installation.html)
installation, then run:

```sh
./scripts/install_deps_conda.sh --manager micromamba --name mm-mem
micromamba activate mm-mem
cmake -S . -B build/conda -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PWD"
cmake --build build/conda --parallel 4
cmake --install build/conda
```

Use `--manager conda` and `conda activate mm-mem` for Conda. Without `--manager`,
the script selects micromamba, mamba, or conda from PATH. It creates the named
environment, or installs dependencies into it if it already exists. Packages
come from conda-forge: C/C++ compilers, CMake, Make, Python, and Linux libnuma/numactl.
Activate the environment before configuring: compiler activation selects the
compiler, and CMake searches the active environment for libnuma. Use a fresh
build directory when switching toolchains. The environment must remain available
when running the installed binaries.

The Conda installer also selects build packages on macOS, omitting libnuma; an
Apple SDK/Command Line Tools installation is still needed. The Python memory-suite
runner currently requires Linux. A Conda environment does not grant permission
to configure huge pages, change NUMA balancing, or access restricted perf counters.

### Optional reporting tools

`mpstat` (sysstat), `perf`, `pgrep` (procps), and `lspci` (pciutils) are used by
specific reporting scripts, not required to run the memory benchmarks. Install
`perf` through the distribution's appropriate kernel-tools package. Missing
`lspci` only skips PCI inventory.

## Run

### Run the memory benchmark suite

```sh
./scripts/run_cpu_micro.py
```

The runner detects root or usable noninteractive sudo (`sudo -n`). It never prompts
for a password. If you have password-based sudo access and want automatic host
setup, first authenticate with `sudo -v`, then run the script normally.
If cached sudo credentials expire during a long run, cleanup may fail; the runner
reports any settings it could not restore.

- With administrative access, it temporarily disables automatic NUMA balancing
  when enabled and attempts to provision huge pages for additional full-region
  random-latency tests. Huge-page binaries also use sudo when needed for allocation
  permissions. Setup failures are reported; ordinary-allocation tests still run.
- Without access, it leaves host settings unchanged, runs ordinary-allocation tests,
  and warns that latency results can include more TLB/page-walk overhead or use a
  different access pattern. These results are not directly comparable to the
  full-region huge-page variant.
- It restores settings it changed on normal completion, benchmark failure, Ctrl-C,
  or SIGTERM. SIGKILL or a machine failure cannot run cleanup. This temporary setup
  changes host-wide settings, so use `--no-system-config` on shared systems.
- Cache dropping and memory compaction are disabled by default. Opt in with
  `--drop-caches` and `--compact`; their effects cannot be undone by restoration.

Useful variants:

```sh
# Always avoid sudo and host changes, even when administrative access is available.
./scripts/run_cpu_micro.py --no-system-config

# Use existing huge-page pools, without provisioning them or changing host settings.
# The caller must already have permission to allocate those pages.
./scripts/run_cpu_micro.py --no-system-config --hugepages existing

# Small idle-latency smoke run without NUMA placement matrices.
./scripts/run_cpu_micro.py --no-system-config --no-matrices --test idle_latency -b 4096 -t 1

# Inspect the planned benchmark commands without running or changing anything.
./scripts/run_cpu_micro.py --dry-run
```

`-t` is seconds per data point, not total suite duration. `--num-threads` controls
bandwidth/memcpy/loaded-latency workers; idle latency uses one worker. `--region-size`
is KiB per worker. The runner reports OS base page size, transparent-huge-page
configuration, and NUMA balancing. Ordinary allocations may use transparent huge
pages according to host policy; they are not guaranteed to use 4 KiB pages.

NUMA matrices require memory-placement operations permitted by the kernel and
container/cpuset policy. Failures are reported, never relabeled as successful NUMA
measurements. Use `--no-matrices` to omit them. Explicit `--hugepages existing`
allocation failures and benchmark failures return a nonzero exit status.

The standalone `config_huge_page.py --setup` and
`config_sysfs_settings.py autonuma --setup/--reset` helpers remain available for
explicit host administration. `config_huge_page.py --clear` (also `--reset`)
**clears the selected huge-page pools**; it does not restore prior reservations.
Prefer the runner for temporary configuration and automatic restoration.

### Individual Test - Idle Latency
Option 1 - Let the runner provision huge pages when privileged access is available
```sh
./scripts/run_cpu_micro.py --test idle_latency -t 10
```
This runs the ordinary-allocation test and, when setup succeeds, the full-region
random test with the largest supported huge-page size up to 1 GiB.

Option 2 - Use ordinary allocation and a random-in-chunk pattern
```
./bin/cpu_idle_latency --region_size 524288 --chunk_size 128 --access_pattern 1 --target_duration 10
```
This allocates a 512 MiB region, pointer-chases randomly through each 128 KiB
chunk, and then moves to the next chunk. Page backing follows OS policy.
128KB chunk size roughly matches what L1DTLB can cover; aggressive page prefetching and buddy prefetching can partially crack this.

### Individual Test - Peak Bandwidth
Use nproc number of threads, each streaming through a 128MB region; sweep different read/write ratios
```
./bin/cpu_peak_bandwidth --region_size 131072 --target_duration 10
```

### Individual Test - Loaded Latency
Use 1 latency thread to do random-in-chunk pointer-chasing; other nproc-1 threads generating variable load with 2:1 read/write mix
```
./scripts/run_cpu_micro.py --test loaded_latency -t 10
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
