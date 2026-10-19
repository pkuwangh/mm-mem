# Contributing

Submit changes through a pull request targeting `main`. For build instructions
and benchmark usage, see [README.md](README.md).

## Versions and releases

[CHANGELOG.md](CHANGELOG.md) is the source of version numbers and release notes.
Keep incremental changes under `## Latest`; no version bump is needed for each
pull request. Record the Git commit when comparing development benchmark runs
(`git describe --tags --always --dirty` identifies the local snapshot).

To prepare a release:

1. Move the Latest entries into a new `## MAJOR.MINOR.PATCH` section. Include
   `### Released` followed by `- YYYY-MM-DD` for the release date, and leave a
   fresh Latest section for development.
2. Use a patch version for compatible fixes, a minor version for compatible
   features, and a major version for incompatible changes. Document measurement
   changes that affect comparisons across versions.
3. Merge the release preparation pull request into `main`.
4. Tag that merged commit with the matching version, without a `v` prefix, and
   push the tag. For example, replacing the version and commit as appropriate:

   ```sh
   git tag -a 1.2.0 <release-commit> -m "Release 1.2.0"
   git push origin 1.2.0
   ```

The [release workflow](.github/workflows/release-from-tag.yml) reads the matching
changelog section from the tagged commit and creates a GitHub release. The tag
selects the version; the workflow does not bump versions or create tags. It
requires a release date and nonempty notes, and omits the Released subsection
from the published notes. A manual run can publish an existing tag; an existing
GitHub release is not overwritten. Historical tags must already contain a
matching changelog section to work with this workflow.

## Parser checks

The command-line utility has a standalone C++14 regression test. Run it without
NUMA or other platform dependencies:

```sh
mkdir -p build
g++ -std=c++14 -Wall -Wextra -Werror -I. tests/command_line_test.cc -o build/command_line_test
./build/command_line_test
```

## Branch benchmark checks

After building, run the CLI, sweep, and x86-64 disassembly checks:

```sh
python3 tests/check_branch_throughput.py build/release/cpu_micro/cpu_branch_throughput
```

These checks exercise all layouts, single-case selection, multiple workers,
and invalid inputs. On Linux x86-64, they also use `objdump` to verify branch
counts and aligned targets in the emitted kernels. Run on actual ARM hardware
before claiming ARM performance validation; cross-compilation only checks syntax.

## Python script checks

```sh
python3 -m unittest discover -s tests -p 'test_*.py'
bash -n scripts/install_deps_sudo.sh scripts/install_deps_conda.sh
```

The setup tests use temporary files and mock privilege/installer commands; they
never install packages or change kernel settings. For a real runner smoke check,
use `--no-system-config --no-matrices --test idle_latency -b 4096 -t 1`.
