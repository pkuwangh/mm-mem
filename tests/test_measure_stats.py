"""Regression checks for process selection and concurrent stats collection."""

import contextlib
import io
from pathlib import Path
import subprocess
import sys
import unittest
from unittest.mock import Mock, patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "scripts"))
import measure_stats as stats


class MeasureStatsTest(unittest.TestCase):
    def args(self, *args):
        return stats.init_parser().parse_args(args)

    def test_pid_command(self):
        command = stats.measure_pmu(5, "123")
        self.assertEqual(command[command.index("-p") + 1], "123")
        self.assertNotIn("-a", command)
        self.assertIn("duration_time,cpu-clock:u,cycles:u,instructions:u", command)
        self.assertEqual(command[-3:], ["--", "sleep", "5"])
        self.assertIn("-a", stats.measure_pmu(5, ""))

    def test_explicit_pids(self):
        self.assertEqual(stats.get_pids(self.args('-p', '123, 456,123')), ['123', '456'])
        for pids in ['', '0', '-1', 'abc', '1,,2']:
            with self.assertRaises(ValueError):
                stats.get_pids(self.args('--pids=' + pids))
        self.assertEqual(stats.get_pids(self.args()), [''])

    def test_no_match_never_launches_collectors(self):
        result = subprocess.CompletedProcess([], 1, '', '')
        with patch.object(stats.subprocess, 'run', return_value=result), \
                patch.object(stats.subprocess, 'Popen') as launch, \
                contextlib.redirect_stderr(io.StringIO()):
            self.assertEqual(stats.main(self.args('-s', 'missing')), 1)
            launch.assert_not_called()

    def test_full_command_matching_and_self_exclusion(self):
        result = subprocess.CompletedProcess([], 0, '12\n34\n56\n', '')
        with patch.object(stats.subprocess, 'run', return_value=result) as run, \
                patch.object(stats, 'ancestor_pids', return_value={'12', '34'}):
            self.assertEqual(stats.get_pids(self.args('-s', 'cpu_branch_throughput')), ['56'])
            self.assertEqual(run.call_args[0][0], ['pgrep', '-f', '--', 'cpu_branch_throughput'])

    def test_pgrep_failure(self):
        result = subprocess.CompletedProcess([], 2, '', 'invalid regex')
        with patch.object(stats.subprocess, 'run', return_value=result):
            with self.assertRaisesRegex(ValueError, 'pgrep failed'):
                stats.get_pids(self.args('-s', '['))

    def test_selectors_and_interval_validation(self):
        for args in [('-p', '1', '-s', 'name'), ('-i', '0'), ('-i', '-2')]:
            with contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit):
                self.args(*args)

    def test_real_subprocess_cleanup(self):
        # Exercise actual pipe lifetimes, including Python 3.6's communicate().
        command = [sys.executable, '-c', 'print("counter report")']
        with patch.object(stats, 'measure_pmu', return_value=command), \
                patch.object(stats, 'measure_cpu_util', return_value=[sys.executable, '-c', 'pass']), \
                contextlib.redirect_stdout(io.StringIO()) as output:
            self.assertEqual(stats.main(self.args('-p', '123')), 0)
        self.assertIn('counter report', output.getvalue())

    def test_collectors_start_before_wait_and_failures_propagate(self):
        for failed_index in [None, 0, 2]:
            launched = []
            def launch(command, **kwargs):
                proc = Mock()
                index = len(launched)
                proc.returncode = 1 if index == failed_index else 0
                proc.poll.return_value = proc.returncode
                proc.communicate.return_value = ('report\n', None)
                def wait(**kwargs):
                    self.assertEqual(len(launched), 3)
                    return proc.returncode
                proc.wait.side_effect = wait
                launched.append(command)
                return proc
            with patch.object(stats.subprocess, 'Popen', side_effect=launch), \
                    contextlib.redirect_stdout(io.StringIO()):
                self.assertEqual(stats.main(self.args('-i', '5', '-p', '123,456')),
                                 0 if failed_index is None else 1)
            self.assertEqual(launched[0][-1], '5')
            self.assertEqual(launched[1][-1], '5')
            self.assertEqual(launched[2], ['mpstat', '1', '5'])


if __name__ == '__main__':
    unittest.main()
