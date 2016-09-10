from __future__ import print_function
import os
from subprocess import Popen, PIPE
from collections import namedtuple
import re

CHILI = '../../chili'

Result = namedtuple('Result', 
                    ['returncode', 'stdoutdata', 'stderrdata'])

Report = namedtuple('Report',
                    ['num_executed', 'num_succeeded', 'num_failed',
                     'num_errors', 'process_return'])

class CurDir:
    def __init__(self, testfile):
        path = os.path.abspath(testfile)
        self._dir = os.path.dirname(path)

    def __enter__(self):
        self._prev_dir = os.getcwd()
        os.chdir(self._dir)

    def __exit__(self, t, v, trace):
        os.chdir(self._prev_dir)

def run(instances, testfile):
    # Only functions
    tests = [f for f in instances if hasattr(f, '__call__')]
    # Only functions named test_
    tests = [f for f in tests if f.__name__.startswith('test_')]

    with CurDir(testfile):
        for t in tests:
            name = t.__name__[5:].replace('_', ' ').capitalize()
            print("%s\n%s" % (name, "Ok" if t() else "Fail"))


def chili(options=[], print_stdout=False, print_stderr=False):
    params = [CHILI]
    params.extend(options)

    process = Popen(params, stdout=PIPE, stderr=PIPE)
    stdoutdata, stderrdata = process.communicate()

    if print_stdout:
        print(stdoutdata)
    if print_stderr:
        print(stderrdata)

    result = Result(returncode = process.returncode,
                    stdoutdata = stdoutdata,
                    stderrdata = stderrdata)
    report = parse_report(result)
    return report

def parse_report(result):
    pattern = "Executed:\ (\d*)\,\ Succeeded:\ (\d*)\, Failed:\ (\d*)\, Errors:\ (\d*)"
    match = re.search(pattern, result.stdoutdata)

    if not match:
        # When process exited ok, we should find some
        # known output
        if result.returncode == 0:
            raise "Unable to parse"

        return Report(process_return=result.returncode,
                      num_executed=0, num_succeeded=0,
                      num_failed=0, num_errors=0)

    num_executed = int(match.group(1))
    num_succeeded = int(match.group(2))
    num_failed = int(match.group(3))
    num_errors = int(match.group(4))

    return Report(process_return=result.returncode,
                  num_executed=num_executed,
                  num_succeeded=num_succeeded,
                  num_failed=num_failed,
                  num_errors=num_errors)
