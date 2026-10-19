# Changelog

## Latest

### Fixed

- Fix process matching and stats collection.
- Fix handling of sparse NUMA nodes and improve error reporting.

### Changed

- Raise the timing-overrun warning threshold to 2%.
- Remove the Boost dependency.
- Improve branch throughput tests and report branches per cycle.
- Support running benchmarks without sudo.
- Restore system settings after benchmarks.

### Added

- Sudo and Conda/micromamba dependency installers.
- Script descriptions and contributor guides.
- Automated releases using tags and this changelog.

## 1.1.0

### Released

- 2026-10-16

### Added

- More huge-page options and loaded-latency sample points.
- Setup scripts for Apple and BCM systems.

### Fixed

- Fix NUMA node indexing and CPU placement.
- Report worker startup failures.

## 1.0.0

### Released

- 2023-01-20

### Added

- Latency, bandwidth, memcpy, and branch throughput benchmarks.
- NUMA utilities and latency/bandwidth matrices.
- Linux and macOS build support.
- Scripts for setup, benchmarks, and stats collection.
