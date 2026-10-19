#!/usr/bin/env python3

"""Run CPU memory benchmarks with automatic, noninteractive privilege detection.

Root or usable sudo enables temporary NUMA-balancing and huge-page setup; ordinary
users run ordinary-page tests with a latency-comparability warning. Host settings
are restored on completion, benchmark failure, Ctrl-C, and SIGTERM. --no-system-config
prevents host changes; --hugepages existing uses pre-provisioned pages without
changing reservations. Cache dropping and compaction are opt-in. --dry-run only
prints a plan. SIGKILL and machine failure cannot trigger restoration.
"""

import argparse
import os
from pathlib import Path
import shlex
import signal
import subprocess
import sys

from config_huge_page import HUGE_PAGE_OPTIONS, pick_huge_page_size, setup_huge_pages
from config_sysfs_settings import check_autonuma, setup_autonuma
from host_config import KernelSettings, privilege_prefix
from print_host_info import get_cpu_info, get_mem_info

ROOT = Path(__file__).resolve().parents[1]
LATENCY_WARNING = (
    'Huge-page latency tests skipped. Without huge pages, measurements can include '
    'more TLB/page-walk overhead or use a different access pattern; they are not '
    'directly comparable to full-region huge-page latency measurements.'
)


def warn(message):
    print('Warning: ' + message, file=sys.stderr, flush=True)


def run_benchmark(args, name, options=(), privileged=None):
    command = (privileged or []) + [str(ROOT / 'bin' / name)]
    command += ['-t', str(args.target_duration)]
    if args.num_threads is not None and name != 'cpu_idle_latency':
        command += ['-n', str(args.num_threads)]
    if args.region_size is not None:
        command += ['-b', str(args.region_size)]
    command += list(options)
    print('+ ' + ' '.join(shlex.quote(arg) for arg in command), flush=True)
    if not args.dry_run:
        subprocess.run(command, check=True)


def run_tests(args, selected, matrices, huge_size, prefix):
    if 'idle_latency' in selected:
        options = ['--latency_matrix'] if matrices else []
        run_benchmark(args, 'cpu_idle_latency', options)
        if huge_size:
            run_benchmark(args, 'cpu_idle_latency', options +
                          ['-p', '2', '-H', str(HUGE_PAGE_OPTIONS[huge_size])], prefix)
    if 'bandwidth' in selected:
        run_benchmark(args, 'cpu_peak_bandwidth')
        if matrices:
            for mix in ('0', '1'):
                run_benchmark(args, 'cpu_peak_bandwidth', ['--bandwidth_matrix', '-m', mix])
        run_benchmark(args, 'cpu_memcpy')
        run_benchmark(args, 'cpu_memcpy', ['-f', '16384'])
    if 'loaded_latency' in selected:
        run_benchmark(args, 'cpu_loaded_latency')
        if huge_size:
            run_benchmark(args, 'cpu_loaded_latency',
                          ['-p', '2', '-H', str(HUGE_PAGE_OPTIONS[huge_size])], prefix)


def main(args):
    selected = args.test or ['idle_latency', 'bandwidth', 'loaded_latency']
    latency_requested = bool(set(selected) & {'idle_latency', 'loaded_latency'})
    if sys.platform != 'linux':
        raise RuntimeError('The suite runner currently requires Linux; run supported binaries individually')
    if (args.drop_caches or args.compact) and (
            args.no_system_config or args.hugepages != 'auto' or not latency_requested):
        raise ValueError('--drop-caches/--compact require automatic huge-page setup for latency tests')
    threads = args.num_threads or len(os.sched_getaffinity(0))
    if 'loaded_latency' in selected and threads < 2:
        raise ValueError('Loaded latency needs at least two worker threads; select another --test')
    required = set()
    if 'idle_latency' in selected:
        required.add('cpu_idle_latency')
    if 'bandwidth' in selected:
        required.update(['cpu_peak_bandwidth', 'cpu_memcpy'])
    if 'loaded_latency' in selected:
        required.add('cpu_loaded_latency')
    for name in sorted(required):
        if not args.dry_run and not os.access(str(ROOT / 'bin' / name), os.X_OK):
            raise RuntimeError(f'Missing executable bin/{name}; build the repository first')

    nodes = get_cpu_info()
    get_mem_info()
    print(f'OS base page size: {os.sysconf("SC_PAGE_SIZE")} bytes')
    try:
        print('Transparent huge pages: ' +
              Path('/sys/kernel/mm/transparent_hugepage/enabled').read_text().strip())
    except OSError:
        print('Transparent huge pages: unavailable')
    autonuma = check_autonuma('current')
    matrices = nodes > 0 and not args.no_matrices
    if not matrices:
        print('NUMA matrices disabled; reporting ordinary allocation measurements')

    # Dry runs deliberately do not invoke sudo, even for probing credentials.
    prefix = None if args.no_system_config or args.dry_run else privilege_prefix()
    settings = KernelSettings(prefix)
    huge_settings = KernelSettings(prefix)
    huge_size = 0
    failed = False
    try:
        if args.dry_run:
            print('Dry run: no benchmarks, sudo probes, or host changes will be performed.')
            if latency_requested and args.hugepages != 'off':
                huge_size = pick_huge_page_size()
                print('Huge-page commands below require usable existing pages or successful privileged setup.')
        else:
            if prefix is not None:
                print('Host configuration enabled using ' + ('noninteractive sudo' if prefix else 'root'))
                if autonuma is not None and autonuma != 0:
                    try:
                        setup_autonuma(0, settings)
                    except (OSError, subprocess.SubprocessError, RuntimeError) as error:
                        settings.restore()
                        warn(f'Could not disable NUMA balancing: {error}')
            elif not args.no_system_config:
                warn('Root/noninteractive sudo unavailable; host settings will remain unchanged. '
                     'If permitted, run sudo -v before starting, or configure the host separately.')
            if latency_requested and args.hugepages == 'auto' and prefix is not None:
                try:
                    huge_size = setup_huge_pages(huge_settings, args.drop_caches, args.compact)
                except (OSError, subprocess.SubprocessError, ValueError, RuntimeError) as error:
                    huge_settings.restore()
                    warn(f'Could not provision huge pages: {error}')
            elif latency_requested and args.hugepages == 'existing':
                huge_size = pick_huge_page_size()
                if not huge_size:
                    raise RuntimeError('No supported huge-page size found for --hugepages existing')
                # Existing mode deliberately runs as the caller and does not provision pages.
                prefix = None
        if latency_requested and not huge_size:
            warn(LATENCY_WARNING)
        run_tests(args, selected, matrices, huge_size, prefix)
    finally:
        # Complete restoration even if another Ctrl-C arrives during cleanup.
        previous = {}
        if not args.dry_run:
            for sig in (signal.SIGINT, signal.SIGTERM):
                previous[sig] = signal.signal(sig, signal.SIG_IGN)
        try:
            for transaction in (huge_settings, settings):
                try:
                    transaction.restore()
                except (OSError, subprocess.SubprocessError, RuntimeError) as error:
                    warn(str(error))
                    failed = True
        finally:
            for sig, handler in previous.items():
                signal.signal(sig, handler)
        if failed:
            raise RuntimeError('Host settings were not fully restored; inspect the warnings above')
    return 0


def positive_int(value):
    number = int(value)
    if number <= 0:
        raise argparse.ArgumentTypeError('must be positive')
    return number


def init_parser():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('--target-duration', '-t', type=positive_int, default=2,
                        help='seconds per benchmark data point, not total suite duration')
    parser.add_argument('--test', action='append', choices=['idle_latency', 'loaded_latency', 'bandwidth'])
    parser.add_argument('--num-threads', '-n', type=positive_int,
                        help='workers for bandwidth/memcpy/loaded latency; idle latency stays at one')
    parser.add_argument('--region-size', '-b', type=positive_int, help='per-worker region size in KiB')
    parser.add_argument('--no-system-config', action='store_true', help='never use sudo or change host settings')
    parser.add_argument('--hugepages', choices=['auto', 'off', 'existing'], default='auto')
    parser.add_argument('--no-matrices', action='store_true', help='omit NUMA placement matrices')
    parser.add_argument('--drop-caches', action='store_true', help='drop page caches before huge-page setup')
    parser.add_argument('--compact', action='store_true', help='compact memory before huge-page setup')
    parser.add_argument('--dry-run', action='store_true')
    return parser


def interrupted(signum, frame):
    raise KeyboardInterrupt


if __name__ == '__main__':
    signal.signal(signal.SIGTERM, interrupted)
    try:
        sys.exit(main(init_parser().parse_args()))
    except KeyboardInterrupt:
        print('Interrupted; restoration attempted.', file=sys.stderr)
        sys.exit(130)
    except (OSError, subprocess.SubprocessError, ValueError, RuntimeError) as error:
        print(f'Error: {error}', file=sys.stderr)
        sys.exit(1)
