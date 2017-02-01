
# Chili - testrunner for C in C

### What is Chili
A test runner for C programmers that makes it easy and fun to write nice
and slim unit tests with minimum hazzle. You write your Chili tests
completely without build dependencies.

Chili is written in C with minimal dependencies on external libraries.
The idea is that Chili could be used on target for embedded development.

### Installation
```bash
~$ make && make install
```
Chili is Linux only.

### Simplest possible unit test
**Create new file**: unittests.c

```C
#include <stdio.h>

int test_that_succeeds()
{
    printf("Returning 1 indicates success\n");
    return 1;
}
```
Tests should always be named with the prefix *test_*. Also note
that the funcion must NOT be static.

**Build the unit test:**
```bash
~$ gcc unittests.c -fPIC --shared -o unittests.so
```
This builds a shared library with the test as an exported
symbol. This what Chili finds and executes.

**Run the unit test:**
```bash
~$ chili all ./unittests.so
./unittests.so: test_that_succeeds: Success [0]
Executed: 1, Succeeded: 1, Failed: 0, Errors: 0
```
The *all* command executes all tests in the shared library,
in this case only one test. 
Note that the output of *printf* is not shown in the output above, 
Chili will default to suppress output of all succeeding tests.

### Unit test return values
Unit tests should always return an int and should not
take any parameters. Chili interprets the returned values the
following way:

* Positive values are interpreted as a successful/green test.
* Zero are interpreted as a failing/red test.
* Negative values are interpreted as an unexpected error occured
while executing the test. Chili will not execute any more tests
in this suite when it encounters a test that returns a negative
value.

A test could also crash during it's execution, this is reported
as a crash but Chili will continue executing other tests.

```C
#include <stdio.h>

int test_that_succeeds()
{
    printf("Positive indicates success\n");
    return 1;
}

int test_that_fails()
{
    printf("Zero indicates failure\n");
    return 0;
}

int test_that_crashes()
{
    int *x = (int*)0;
    printf("This will crash\n");
    *x = 0;
}

int test_with_error()
{
    printf("This is an error\n");
    return -1;
}
```
Build this into a shared library named unittests.so and let Chili execute the tests:

```bash
~$ chili all ./unittests.so
./unittests.so: test_that_succeeds: Success [0]
./unittests.so: test_that_fails: Failed [1]
>>>Capture start
Zero indicates failure
<<< Capture end
./unittests.so: test_that_crashes: Crashed [2]
>>> Capture start
This will crash
<<< Capture end
./unittests.so: test_with_error: Error [3]
>>> Capture start
This is an error
<<< Capture end
Executed: 4, Succeeded: 1, Failed: 1, Errors: 2
```
As shown above prints are only shown on tests with any type of problem.

### Suite setup functions

### Debugging a unit test
To debug a problematic unit test do like this. To debug the test *test_that_crashes* in the following module.

```C
#include <stdio.h>

int test_that_crashes()
{
    int *x = (int*)0;
    printf("This will crash\n");
    *x = 0;
}
```
To be able to debug, the tests and other linked modules need to be built with debug information:

```bash
~$ gcc unittests.c -fPIC --shared -g -o unittests.so
```

Debugging is done with Chilis debug command and the name of the test:
```bash
~$ chili debug ./unittests.so:test_that_crashes
GNU gdb (Ubuntu 7.11.1-0ubuntu1~16.04) 7.11.1
Copyright (C) 2016 Free Software Foundation, Inc
...
Attaching to process 1637
Reading symbols from ...
Reading symbols ./unittests.so..done
...
Breakpoint 1 at 0x7fa51cab1766: file unittest.c, line 3.
Continuing with signal SIGCONT.

Breakpoint 1, test_that_crashes () at unittest.c:3
3       int *x = (int*)0;
(gdb)
```
Gdb of course needs to be installed for this to work.

### Other usable commands

