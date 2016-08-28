from __future__ import print_function
import os

CHILI = '../../chili'

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
            print("Verifying: %s -> %s" % (t.__name__, "ok" if t() else "fail"))
