# Changelog

## Latest

### Fixed

- Fix stats collection for selected PIDs and long process names; fail on no
  matches instead of falling back to system-wide monitoring. Collect user-space
  process counters and CPU utilization concurrently for the requested duration,
  and report collector failures with a nonzero exit status.

### Changed

- Replace the arithmetic-dependent branch benchmark with x86-64/AArch64 taken-branch
  assembly sweeps, per-worker results, and optional Linux hardware cycles per branch.
  Add `--branch_case` to isolate a layout; `-t` is split across the sweep. Results
  and output format differ from the previous loop benchmark.

- Label branch throughput as `GBr/s` (billions of branches per second).
- Replace Boost.Program_options with a local C++14 command-line parser and remove
  Boost from build and installation requirements. Integer options reject negative
  unsigned values and overflow; option names, aliases, and defaults are preserved.

### Added

- Module docstrings describing each Python script, its usage, and system effects.

- Contributor guidance for coding agents in `AGENTS.md`.
- Contribution and release procedures in `CONTRIBUTING.md`.
- This changelog, including summaries of the existing releases.
- GitHub Actions workflow to publish release notes from the changelog when a
  matching release tag is pushed, or through a manual run for an existing tag.

## 1.1.0

### Released

- 2026-10-16

### Added

- Additional huge-page options for latency benchmarks.
- More sample points in the loaded-latency bandwidth curve.
- Environment setup scripts for Apple and BCM environments.

### Fixed

- Handle unusual NUMA node layouts without out-of-range indexing.
- Report worker startup and affinity failures instead of silently continuing.
- Keep worker CPU placement within the process's allowed CPU affinity mask.

## 1.0.0

### Released

- 2023-01-20

### Added

- Idle latency, peak bandwidth, loaded latency, and memcpy benchmarks.
- Branch throughput and NUMA information utilities.
- NUMA latency and bandwidth matrices, with worker CPU affinity binding.
- Linux and macOS build support.
- Python helpers for running benchmarks, inspecting hosts, managing huge pages,
  and collecting statistics.
