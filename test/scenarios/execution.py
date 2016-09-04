""" Verifies that the expected tests are executed and
    reported correctly.
"""
import subprocess
import os
import sys

from runner import run, chili


def test_executes_all_tests_even_in_case_of_failure():
    report = chili(['./chili_failure.so'])

    all_executed = report.num_executed == 3
    all_failed = report.num_executed == report.num_failed
    return all_executed and all_failed

def test_stops_execution_on_suite_setup_error():
    report = chili(['./chili_suite_setup_error.so'])

    none_executed = report.num_executed == 0
    return none_executed

if __name__ == "__main__":
    sys.exit(run(globals().values(), __file__))
