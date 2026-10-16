# Repository guidance

## Purpose

MM-Mem is a CPU memory benchmarking and characterization toolkit. The author
originally wrote it while working at Meta to test ARM CPUs because Intel MLC
could not run on ARM. Preserve measurement correctness and ARM support when
modernizing the code.

## Layout

- `cpu_micro/`: benchmark executables, configuration, workers, and memory kernels.
- `cpu_micro/kernels_delay_bandwidth/`: specialized delayed-bandwidth kernels
  used by the loaded-latency benchmark.
- `common/`: timing, memory allocation, NUMA topology, and worker management.
- `scripts/`: Python 3 benchmark orchestration and host configuration helpers.
- `CMakeLists.txt`, `Makefile`: C++14 builds; release binaries install to `bin/`.
- `CHANGELOG.md`: version headings, release dates, and user-visible history.
- `CONTRIBUTING.md`: contribution and release procedures.
- `.github/workflows/release-from-tag.yml`: publish releases from tagged changelog sections.

## Building and checking changes

Dependencies include a C++14 compiler, CMake, Make, and
Linux libnuma development files. `scripts/install_deps.py` installs system
packages; inspect it before running it. macOS uses pthread without
libnuma.

```sh
make
make debug
```

The release build is in `build/release/`; the debug build is in `build/debug/`.
For an explicitly bounded release build, use:

```sh
cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PWD"
cmake --build build/release --parallel 2
cmake --install build/release
```

Run the standalone parser regression test in [CONTRIBUTING.md](CONTRIBUTING.md)
when changing command-line parsing. For C++ changes, build the
affected targets and run a short, bounded benchmark relevant to the change on
available hardware. Inspect `--help` for supported options. For example:

```sh
./bin/cpu_idle_latency --num_threads 1 --region_size 4096 --target_duration 1
```

A successful smoke run establishes basic execution, not performance equivalence.
Report which architecture and platform were checked and any untested paths.
Documentation-only changes need link, command, and diff review rather than a
full benchmark run.

## Measurement correctness

- Preserve CPU affinity and NUMA placement constraints, including restricted
  cpusets. Worker failures must remain visible to the caller.
- Treat access patterns, read/write ratios, cache-line assumptions, timing,
  allocation, and accounting as part of benchmark behavior. Explain changes
  that affect comparability in the changelog.
- Keep architecture-specific code guarded and preserve existing platform paths.
  Do not claim ARM or macOS validation from an x86 Linux build.
- For performance comparisons, record the commit/version, CPU, OS, compiler,
  command, thread count, memory placement, and huge-page settings. Compare under
  equivalent conditions and repeat measurements before drawing conclusions.
- The full runner (`scripts/run_cpu_micro.py`) can reserve huge pages and change
  automatic NUMA balancing. Use individual binaries for routine smoke checks;
  run host configuration helpers only when those system changes are intended.

## Change workflow

- Follow the surrounding C++ and Python style and keep changes focused.
- Preserve command-line options and output formats unless a deliberate interface
  change is part of the task.
- Add user-visible changes under Latest in `CHANGELOG.md`. When preparing a
  release, move those entries into a `## MAJOR.MINOR.PATCH` section with a
  `### Released` subsection containing `- YYYY-MM-DD`. Keep Latest for ongoing
  development. There is no separate version file; see [CONTRIBUTING.md](CONTRIBUTING.md) for release steps.
- Submit changes through a pull request targeting `main`; do not push directly
  to `main`. Do not create or publish release tags unless requested.
- Keep build products, benchmark output, and machine-specific configuration out
  of commits. `build/` and `bin/` are already ignored.
