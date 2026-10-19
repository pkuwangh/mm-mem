"""Check dependency installers and privilege-aware runner without host changes."""

import contextlib
import io
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'scripts'))
import config_huge_page as huge
import config_sysfs_settings as numa
import host_config
import print_host_info as info
import run_cpu_micro as runner
import utils

RUN_BENCHMARK = runner.run_benchmark


class SettingsTest(unittest.TestCase):
    def test_privilege_detection(self):
        with patch.object(host_config.os, 'geteuid', return_value=0), \
                patch.object(host_config.subprocess, 'run') as run:
            self.assertEqual(host_config.privilege_prefix(), [])
            run.assert_not_called()
        with patch.object(host_config.os, 'geteuid', return_value=1000), \
                patch.object(host_config.shutil, 'which', return_value='/usr/bin/sudo'), \
                patch.object(host_config.subprocess, 'run') as run:
            run.return_value.returncode = 1
            self.assertIsNone(host_config.privilege_prefix())
            run.return_value.returncode = 0
            self.assertEqual(host_config.privilege_prefix(), ['sudo', '-n', '--'])
            self.assertEqual(run.call_args[0][0], ['sudo', '-n', 'true'])
        with patch.object(host_config.os, 'geteuid', return_value=1000), \
                patch.object(host_config.shutil, 'which', return_value=None):
            self.assertIsNone(host_config.privilege_prefix())

    def test_restore_original_after_multiple_writes(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'setting'
            path.write_text('7\n')
            settings = host_config.KernelSettings([])
            settings.set(path, 2)
            settings.set(path, 3)
            settings.restore()
            self.assertEqual(path.read_text(), '7\n')
            self.assertFalse(settings.original)
            with self.assertRaises(PermissionError):
                host_config.KernelSettings(None).write(path, 1)
            self.assertEqual(path.read_text(), '7\n')

    def test_sudo_write_uses_stdin_not_shell(self):
        with patch.object(host_config.subprocess, 'run') as run:
            host_config.KernelSettings(['sudo', '-n', '--']).write('/sys/test path', 3)
            self.assertEqual(run.call_args[0][0], ['sudo', '-n', '--', 'tee', '/sys/test path'])
            self.assertEqual(run.call_args[1]['input'], '3\n')
            self.assertTrue(run.call_args[1]['check'])

    def test_restore_continues_after_one_failure(self):
        with tempfile.TemporaryDirectory() as directory:
            paths = [Path(directory) / name for name in ('a', 'b')]
            for path in paths:
                path.write_text('1\n')
            settings = host_config.KernelSettings([])
            for path in paths:
                settings.set(path, 2)
            real_write = settings.write
            def write(path, value):
                if str(path) == str(paths[1]):
                    raise PermissionError('denied')
                real_write(path, value)
            with patch.object(settings, 'write', side_effect=write), self.assertRaises(RuntimeError):
                settings.restore()
            self.assertEqual(paths[0].read_text(), '1\n')
            self.assertEqual(paths[1].read_text(), '2\n')


class HugePagesTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.patch = patch.object(info, 'NODE_ROOT', self.root)
        self.patch.start()
        self.addCleanup(self.patch.stop)
        self.addCleanup(self.temp.cleanup)

    def node(self, nid, size=2048, count=0):
        node = self.root / ('node' + str(nid))
        node.mkdir(exist_ok=True)
        (node / 'meminfo').write_text(f'Node {nid} MemTotal: 4194304 kB\n')
        pages = node / 'hugepages' / f'hugepages-{size}kB'
        pages.mkdir(parents=True, exist_ok=True)
        path = pages / 'nr_hugepages'
        path.write_text(str(count) + '\n')
        return path

    def test_sparse_nodes_and_preserve_larger_pools(self):
        first = self.node(2, count=2048)
        second = self.node(7)
        self.assertEqual(set(info.get_mem_info(False)), {2, 7})
        settings = host_config.KernelSettings([])
        with contextlib.redirect_stdout(io.StringIO()):
            self.assertEqual(huge.setup_huge_pages(settings), 2048)
            state = huge.check_huge_pages()
        self.assertEqual(state, {(2, 2048): 2048, (7, 2048): 1024})
        settings.restore()
        self.assertEqual(first.read_text(), '2048\n')
        self.assertEqual(second.read_text(), '0\n')

    def test_empty_and_unsupported_pages(self):
        self.assertEqual(huge.pick_huge_page_size(), 0)
        self.node(4, size=65536)
        self.assertEqual(huge.pick_huge_page_size(), 0)
        with self.assertRaisesRegex(RuntimeError, 'No supported'):
            huge.setup_huge_pages(host_config.KernelSettings([]))

    def test_partial_failure_is_restorable(self):
        first = self.node(2)
        second = self.node(7)
        settings = host_config.KernelSettings([])
        real_write = settings.write
        def write(path, value):
            if str(path) == str(second) and value != 0:
                raise PermissionError('restricted node')
            real_write(path, value)
        with patch.object(settings, 'write', side_effect=write), \
                contextlib.redirect_stdout(io.StringIO()):
            with self.assertRaises(PermissionError):
                huge.setup_huge_pages(settings)
            settings.restore()
        self.assertEqual(first.read_text(), '0\n')
        self.assertEqual(second.read_text(), '0\n')

    def test_no_cache_drop_or_compaction_by_default(self):
        self.node(0)
        settings = host_config.KernelSettings([])
        with patch.object(settings, 'write', wraps=settings.write) as write, \
                contextlib.redirect_stdout(io.StringIO()):
            huge.setup_huge_pages(settings)
        self.assertTrue(all('/proc/' not in str(call[0][0]) for call in write.call_args_list))


class RunnerTest(unittest.TestCase):
    def setUp(self):
        self.stack = contextlib.ExitStack()
        self.addCleanup(self.stack.close)
        self.output = self.stack.enter_context(contextlib.redirect_stdout(io.StringIO()))
        self.errors = self.stack.enter_context(contextlib.redirect_stderr(io.StringIO()))
        for target, value in [('get_cpu_info', 1), ('get_mem_info', {2: 4194304}),
                              ('check_autonuma', 1)]:
            self.stack.enter_context(patch.object(runner, target, return_value=value))
        self.stack.enter_context(patch.object(runner.os, 'access', return_value=True))
        self.privilege = self.stack.enter_context(patch.object(runner, 'privilege_prefix', return_value=None))
        self.run = self.stack.enter_context(patch.object(runner, 'run_benchmark'))

    def args(self, *args):
        return runner.init_parser().parse_args(['--test', 'idle_latency'] + list(args))

    def test_unprivileged_fallback_without_writes(self):
        with patch.object(host_config.KernelSettings, 'write') as write:
            self.assertEqual(runner.main(self.args()), 0)
            write.assert_not_called()
        self.assertEqual(self.run.call_count, 1)
        self.assertIn('TLB/page-walk', self.errors.getvalue())

    def test_no_system_config_and_dry_run_do_not_probe_sudo(self):
        runner.main(self.args('--no-system-config'))
        runner.main(self.args('--dry-run'))
        self.privilege.assert_not_called()

    def test_existing_pages_do_not_elevate_or_write_when_disabled(self):
        with patch.object(runner, 'pick_huge_page_size', return_value=2048), \
                patch.object(host_config.KernelSettings, 'write') as write:
            runner.main(self.args('--no-system-config', '--hugepages', 'existing'))
            write.assert_not_called()
        self.privilege.assert_not_called()
        self.assertEqual(self.run.call_count, 2)
        self.assertIsNone(self.run.call_args[0][-1])

    def test_failure_and_interrupt_restore_settings(self):
        for error in [subprocess.CalledProcessError(1, ['benchmark']), KeyboardInterrupt()]:
            with tempfile.TemporaryDirectory() as directory:
                balancing = Path(directory) / 'balancing'
                pages = Path(directory) / 'pages'
                balancing.write_text('1\n')
                pages.write_text('0\n')
                self.privilege.return_value = []
                def configure(settings, *args):
                    settings.set(pages, 2)
                    return 2048
                self.run.side_effect = error
                with patch.object(numa, 'AUTONUMA_PATH', balancing), \
                        patch.object(runner, 'setup_huge_pages', side_effect=configure):
                    with self.assertRaises(type(error)):
                        runner.main(self.args())
                self.assertEqual(balancing.read_text(), '1\n')
                self.assertEqual(pages.read_text(), '0\n')

    def test_auto_setup_failure_falls_back_after_restoring(self):
        self.privilege.return_value = []
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'pages'
            path.write_text('0\n')
            def configure(settings, *args):
                settings.set(path, 2)
                raise RuntimeError('insufficient pages')
            with patch.object(runner, 'setup_autonuma'), \
                    patch.object(runner, 'setup_huge_pages', side_effect=configure):
                runner.main(self.args())
            self.assertEqual(path.read_text(), '0\n')
        self.assertEqual(self.run.call_count, 1)
        self.assertIn('insufficient pages', self.errors.getvalue())

    def test_commands_pass_arguments_and_propagate_errors(self):
        args = self.args('-t', '1', '-b', '4096')
        with patch.object(runner, 'ROOT', Path('/tmp/repo with spaces')), \
                patch.object(runner.subprocess, 'run') as run:
            RUN_BENCHMARK(args, 'cpu_idle_latency', ['--latency_matrix'])
            self.assertEqual(run.call_args[0][0], [
                '/tmp/repo with spaces/bin/cpu_idle_latency', '-t', '1',
                '-b', '4096', '--latency_matrix'])
            self.assertTrue(run.call_args[1]['check'])
            run.side_effect = subprocess.CalledProcessError(3, ['benchmark'])
            with self.assertRaises(subprocess.CalledProcessError):
                RUN_BENCHMARK(args, 'cpu_idle_latency')


class InstallerTest(unittest.TestCase):
    def test_dry_run_no_installation(self):
        for script, args in [('install_deps_sudo.sh', []),
                             ('install_deps_conda.sh', ['--manager', 'micromamba'])]:
            result = subprocess.run(['bash', str(ROOT / 'scripts' / script), '--dry-run'] + args,
                                    stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn('cmake', result.stdout)
            self.assertNotIn('boost', result.stdout)

    def test_conda_existing_environment_uses_install(self):
        with tempfile.TemporaryDirectory() as directory:
            manager = Path(directory) / 'micromamba'
            manager.write_text('#!/bin/sh\nif [ "$1" = run ]; then exit 0; fi\nprintf "%s\\n" "$@"\n')
            manager.chmod(0o755)
            env = dict(os.environ, PATH=directory + ':' + os.environ['PATH'])
            result = subprocess.run(['bash', str(ROOT / 'scripts/install_deps_conda.sh'), '--manager', 'micromamba'],
                                    env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn('micromamba install', result.stdout)
            self.assertIn('libnuma', result.stdout)

    def test_shared_helpers_propagate_failures(self):
        with self.assertRaises(subprocess.CalledProcessError):
            utils.run_proc_simple([sys.executable, '-c', 'raise SystemExit(3)'])


if __name__ == '__main__':
    unittest.main()
