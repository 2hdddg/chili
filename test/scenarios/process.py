""" Verifies chili process return code in scenarios like
    test success, test failure and test crashes.

    Purpose is to ensure proper usage in script environments
    like contious integration.
"""
import subprocess
import os
import sys

from runner import run, chili_test


def test_process_returns_0_on_test_success():
    report = chili_test(['./chili_success.so'])
    return report.process_return == 0 and report.num_succeeded > 0

def test_process_returns_non_0_on_test_failure():
    report = chili_test(['./chili_failure.so'])
    return report.process_return != 0 and report.num_failed > 0

def test_process_returns_non_0_on_suite_setup_error():
    report = chili_test(['./chili_suite_setup_error.so'])
    return report.process_return != 0 and report.num_executed == 0

def test_process_returns_non_0_when_no_test_suite():
    report = chili_test([])
    return report.process_return != 0

def test_process_returns_non_0_when_test_suite_doesnt_exist():
    report = chili_test(['./non_existent.so'])
    return report.process_return != 0

if __name__ == "__main__":
    sys.exit(run(globals().values(), __file__))
