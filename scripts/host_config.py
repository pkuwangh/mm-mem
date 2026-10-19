#!/usr/bin/env python3

"""Detect noninteractive administrative access and track reversible kernel writes.

The runner uses this module to update only requested integer settings and restore
their original values. sudo is never allowed to prompt. Dropping caches and
compacting memory are one-shot actions and cannot be undone.
"""

import os
from pathlib import Path
import shutil
import subprocess


def privilege_prefix():
    """Return [] for root, a sudo prefix, or None when access is unavailable."""
    if os.geteuid() == 0:
        return []
    if shutil.which('sudo'):
        try:
            result = subprocess.run(['sudo', '-n', 'true'], stdout=subprocess.DEVNULL,
                                    stderr=subprocess.DEVNULL, timeout=5)
            if result.returncode == 0:
                return ['sudo', '-n', '--']
        except (OSError, subprocess.TimeoutExpired):
            pass
    return None


class KernelSettings:
    def __init__(self, prefix):
        self.prefix = prefix
        self.original = {}

    def write(self, path, value):
        if self.prefix is None:
            raise PermissionError('Root or noninteractive sudo is required')
        value = str(int(value)) + '\n'
        if self.prefix:
            try:
                subprocess.run(self.prefix + ['tee', str(path)], input=value,
                               universal_newlines=True, stdout=subprocess.DEVNULL,
                               stderr=subprocess.PIPE, check=True)
            except subprocess.CalledProcessError as error:
                raise RuntimeError(f'Cannot write {path}: {error.stderr.strip()}') from error
        else:
            Path(path).write_text(value)

    def set(self, path, value):
        path = str(path)
        previous = int(Path(path).read_text().strip())
        if previous == value:
            return
        # Record before writing so partially completed changes can be restored.
        self.original.setdefault(path, previous)
        self.write(path, value)
        if int(Path(path).read_text().strip()) != value:
            raise RuntimeError(f'Kernel could not apply {path}={value}')

    def restore(self):
        errors = []
        for path, value in reversed(list(self.original.items())):
            try:
                if int(Path(path).read_text().strip()) == value:
                    del self.original[path]
                    continue
                self.write(path, value)
                if int(Path(path).read_text().strip()) != value:
                    raise RuntimeError('readback differs from original value')
                del self.original[path]
            except (OSError, subprocess.SubprocessError, ValueError, RuntimeError) as error:
                errors.append(f'{path}: {error}')
        if errors:
            raise RuntimeError('Could not restore host settings: ' + '; '.join(errors))
