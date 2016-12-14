""" Verifies that the expected tests are executed and
    reported correctly.
"""
import subprocess
import os
import sys

from runner import run, chili_named


def test_named_executes_all_tests_even_in_case_of_failure():
    named_tests = [
        './chili_failure.so:test_failure1',
        './chili_failure.so:test_failure2',
        './chili_failure.so:test_failure3',
    ]
    report = chili_named(named_tests)

    all_executed = report.num_executed == 3
    all_failed = report.num_executed == report.num_failed
    return all_executed and all_failed

def test_named_stops_execution_on_suite_setup_error():
    named_tests = [
        './chili_suite_setup_error.so:test_success_but_never_runs',
        './chili_suite_setup_error.so:test_success_another_one_that_never_runs',
    ]
    report = chili_named(named_tests)

    none_executed = report.num_executed == 0
    return none_executed

def test_named_stops_execution_on_test_setup_error():
    named_tests = [
        './chili_test_setup_error.so:test_success_but_never_runs',
        './chili_test_setup_error.so:test_success_another_one_that_never_runs',
    ]
    report = chili_named(named_tests)

    one_error = report.num_errors == 1
    none_succeeded = report.num_succeeded == 0
    return one_error and none_succeeded

def test_named_executes_all_tests_even_when_test_crashes():
    named_tests = [
        './chili_crash.so:test_crash_one',
        './chili_crash.so:test_crash_two',
    ]
    report = chili_named(named_tests)

    all_executed = report.num_executed == 2
    all_errors = report.num_executed == report.num_errors
    return all_executed and all_errors

if __name__ == "__main__":
    sys.exit(run(globals().values(), __file__))

