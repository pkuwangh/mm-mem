# Changelog

## Latest

### Added

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
