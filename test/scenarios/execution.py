""" Verifies chili process return code in scenarios like
    test success, test failure and test crashes.

    Purpose is to ensure proper usage in script environments
    like contious integration.
"""
import subprocess
import os
import sys

from runner import run, chili


def test_process_returns_0_on_test_success():
    report = chili(['./chili_success.so'])
    return report.process_return == 0 and report.num_succeeded > 0

def test_process_returns_non_0_on_test_failure():
    report = chili(['./chili_failure.so'])
    return report.process_return != 0 and report.num_failed > 0

def test_process_returns_number_of_failed_tests():
    report = chili(['./chili_failure.so'])
    return report.process_return == 3 and report.num_failed == 3

def test_process_returns_non_0_when_no_test_suite():
    report = chili([])
    return report.process_return != 0

def test_process_returns_non_0_when_test_suite_doesnt_exist():
    report = chili(['./non_existent.so'])
    return report.process_return != 0

if __name__ == "__main__":
    sys.exit(run(globals().values(), __file__))
