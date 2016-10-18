""" Verifies that the expected tests are executed and
    reported correctly.
"""
import subprocess
import os
import sys

from runner import run, chili_test


def test_executes_all_tests_even_in_case_of_failure():
    report = chili_test(['./chili_failure.so'])

    all_executed = report.num_executed == 3
    all_failed = report.num_executed == report.num_failed
    return all_executed and all_failed

def test_stops_execution_on_suite_setup_error():
    report = chili_test(['./chili_suite_setup_error.so'])

    none_executed = report.num_executed == 0
    return none_executed

def test_stops_execution_on_test_setup_error():
    report = chili_test(['./chili_test_setup_error.so'])

    one_error = report.num_errors == 1
    none_succeeded = report.num_succeeded == 0
    return one_error and none_succeeded

def test_stops_execution_on_test_teardown_error():
    report = chili_test(['./chili_test_teardown_error.so'])

    one_executed = report.num_executed == 1
    one_error = report.num_errors == 1
    none_succeeded = report.num_succeeded == 0
    return one_executed and one_error and none_succeeded

def test_stops_execution_on_test_teardown_2_error():
    report = chili_test(['./chili_test_teardown_error2.so'])

    two_executed = report.num_executed == 2
    one_error = report.num_errors == 1
    one_succeeded = report.num_succeeded == 1
    return two_executed and one_error and one_succeeded

def test_executes_all_tests_even_when_test_crashes():
    report = chili_test(['./chili_crash.so'])

    all_executed = report.num_executed == 2
    all_errors = report.num_executed == report.num_errors
    return all_executed and all_errors

if __name__ == "__main__":
    sys.exit(run(globals().values(), __file__))

