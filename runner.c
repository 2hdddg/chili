#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "runner.h"


/* Constants */
const char *_once_before_name = "once_before";
const char *_once_after_name  = "once_after";
const char *_each_before_name = "each_before";
const char *_each_after_name  = "each_after";


/* Types */
struct test {
    const char *name;
    int executed;
    int result;
};


/* Globals */
static int _once_before_found;
static int _once_after_found;
static int _each_before_found;
static int _each_after_found;

struct test *_tests;
int _max;
int _num;


/* List functions */
static int _add(const char *symbol)
{
    struct test *test;

    if (_num >= _max){
        printf("Tests full\n");
        return -1;
    }

    test = &_tests[_num];
    test->name = symbol;
    test->executed = 0;
    test->result = 0;

    _num++;

    return 1;
}


/* Eval functions */
static int _eval_fixture(const char *symbol)
{
    if (strcmp(symbol, _once_before_name) == 0){
        _once_before_found = 1;
        return 1;
    }
    if (strcmp(symbol, _once_after_name) == 0){
        _once_after_found = 1;
        return 1;
    }
    if (strcmp(symbol, _each_before_name) == 0){
        _each_before_found = 1;
        return 1;
    }
    if (strcmp(symbol, _each_after_name) == 0){
        _each_after_found = 1;
        return 1;
    }

    return 0;
}

static int _eval_test(const char *symbol)
{
    const char *test_ = "test_";
    const int len = 5; /* length of test_ */

    return strncmp(test_, symbol, len) == 0 ? 1 : 0;
}


/* Binding functions */


/* Externals */
int chili_run_begin(int max)
{
    int size = sizeof(struct test) * max;

    _once_before_found = 0;
    _once_after_found = 0;
    _each_before_found = 0;
    _each_after_found = 0;
    _max = max;
    _num = 0;

    _tests = malloc(size);
    if (!_tests){
        printf("Unable to allocate: %s", strerror(errno));
        return -1;
    }

    return 1;
}

int chili_run_eval(const char *symbol)
{
    int found;

    found = _eval_fixture(symbol);
    if (found){
        return 1;
    }

    found = _eval_test(symbol);
    if (found){
        return _add(symbol);
    }

    return 0;
}

int chili_run_tests()
{
    return 1;
}

void chili_run_end()
{
    free(_tests);
}
