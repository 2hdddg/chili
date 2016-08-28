""" Verifies chili process return code in scenarios like
    test success, test failure and test crashes.

    Purpose is to ensure proper usage in script environments
    like contious integration.
"""
import subprocess
import os
import sys

from runner import run, CHILI


def test_returns_0_on_test_success():
    return subprocess.call([CHILI, './chili_success.so']) == 0

def test_returns_non_0_when_no_test_suite():
    return subprocess.call([CHILI]) != 0

def test_returns_non_0_when_test_suite_doesnt_exist():
    return subprocess.call([CHILI]) != 0

if __name__ == "__main__":
    sys.exit(run(globals().values(), __file__))
