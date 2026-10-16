#!/usr/bin/env python3
"""Check branch benchmark output and x86-64 emitted branch chains.

Usage: python3 tests/check_branch_throughput.py [path/to/cpu_branch_throughput]
Requires a built executable; Linux x86-64 assembly checks also require objdump.
"""

from pathlib import Path
import platform
import re
import subprocess
import sys

binary = str(Path(sys.argv[1] if len(sys.argv) > 1 else
                  'build/release/cpu_micro/cpu_branch_throughput').resolve())


def run(*args):
    return subprocess.run([binary] + list(args), stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE, universal_newlines=True, timeout=30)


def rows(output):
    return [line.split() for line in output.splitlines()
            if re.match(r'^\s*\d+\s+\d+\s+(unconditional|conditional)\s', line)]


sweep = run('-t', '1')
assert sweep.returncode == 0, sweep.stderr
results = rows(sweep.stdout)
assert [int(row[0]) for row in results] == list(range(1, 13))
for row in results:
    assert float(row[5]) > 0, row
    if row[6] != 'n/a':
        assert float(row[6]) > 0 and abs(float(row[6]) * float(row[7]) - 1) < 0.02, row
assert 'Best observed thread 0:' in sweep.stdout
selected = run('-t', '1', '--branch_case', '2', '-n', '2')
assert selected.returncode == 0, selected.stderr
assert [(row[0], row[1]) for row in rows(selected.stdout)] == [('2', '0'), ('2', '1')]
for args in [('-t', '0'), ('-n', '0'), ('--branch_case', '13')]:
    result = run(*args)
    assert result.returncode != 0 and 'must be' in result.stderr

if platform.system() == 'Linux' and platform.machine() == 'x86_64':
    assembly = subprocess.check_output(
        ['objdump', '-d', '-C', '--no-show-raw-insn', binary], universal_newlines=True)
    kernels = re.findall(
        r'<void mm_worker::branch_chain<(\d+)u, (\d+)u, (true|false)>\(unsigned long\)>:\n(.*?)(?=\n\n)',
        assembly, re.S)
    assert len(kernels) == 12
    for count, alignment, conditional, body in kernels:
        jumps = re.findall(r'^\s*([0-9a-f]+):\s+(jne|jmp)\s+([0-9a-f]+)', body, re.M)
        assert len(jumps) == int(count) + 1
        for address, opcode, target in jumps[:-1]:
            assert int(target, 16) > int(address, 16) + 2
            assert int(target, 16) % (1 << int(alignment)) == 0
            assert opcode == ('jne' if conditional == 'true' else 'jmp')
        assert int(jumps[-1][2], 16) < int(jumps[-1][0], 16)
print('Branch throughput CLI, sweep, and applicable assembly checks passed.')
