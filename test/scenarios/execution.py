""" Verifies that the expected tests are executed and
    reported correctly.
"""
import subprocess
import os
import sys

from runner import run, chili


def test_executes_all_tests_even_in_case_of_failure():
    report = chili(['./chili_failure.so'])
    return report.num_executed == 3 and report.num_failed > 0

if __name__ == "__main__":
    sys.exit(run(globals().values(), __file__))
