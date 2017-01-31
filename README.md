
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
### Example unit test
**Create new file**: unittests.c

```C
#include <stdio.h>

int test_that_succeeds()
{
    printf("Returning 1 indicates success");
    return 1;
}
```
Tests should always be named with the prefix *test_*. Also note
that the funcion must NOT be static.

**Build the unittest:**
```bash
~$ gcc unittests.c -fPIC --shared -o unittests.so
```
Chili requires that a share library is built.

**Run the unittest:**
```bash
~$ chili all ./unittests.so
./unittests.so: test_that_succeeds: Success [0]
Executed: 1, Succeeded: 1, Failed: 0, Errors: 0
```
The *all* command executes all tests in the shared library,
in this case only one test. 
Note that the output of *printf* is not shown in the output above, 
Chili will default to suppress output of all succeeding tests.
