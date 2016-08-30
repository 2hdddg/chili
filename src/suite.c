#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "suite.h"

/* Debugging */
#define DEBUG 0
#include "debug.h"

/* Constants */
const char *_once_before_name = "once_before";
const char *_once_after_name  = "once_after";
const char *_each_before_name = "each_before";
const char *_each_after_name  = "each_after";


/* Globals */
int _max;
struct chili_suite _suite;


/* List functions */
static int _add(char *symbol)
{
    char **test = _suite.tests + _suite.count;

    if (_suite.count >= _max){
        printf("Tests full\n");
        return -1;
    }

    *test = symbol;
    _suite.count++;

    debug_print("Found test: %s\n", symbol);

    return 1;
}


/* Eval functions */
static int _eval_fixture(char *symbol)
{
    if (strcmp(symbol, _once_before_name) == 0){
        _suite.once_before = _once_before_name;
        debug_print("Found fixture: once before\n");
        return 1;
    }
    if (strcmp(symbol, _once_after_name) == 0){
        _suite.once_after = _once_after_name;
        debug_print("Found fixture: once after\n");
        return 1;
    }
    if (strcmp(symbol, _each_before_name) == 0){
        _suite.each_before = _each_before_name;
        debug_print("Found fixture: each before\n");
        return 1;
    }
    if (strcmp(symbol, _each_after_name) == 0){
        _suite.each_after = _each_after_name;
        debug_print("Found fixture: each after\n");
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

/* Externals */
int chili_suite_begin(int max)
{
    int size = sizeof(char*) * max;
    char **tests = NULL;

    debug_print("Beginning suite with max %d entries\n", max);

    _max = max;
    memset(&_suite, 0, sizeof(struct chili_suite));

    tests = malloc(size);
    if (!tests){
        printf("Unable to allocate: %s", strerror(errno));
        return -1;
    }

    _suite.tests = tests;

    return 1;
}

int chili_suite_eval(char *symbol)
{
    int found;

    debug_print("Evaluating symbol %s\n", symbol);

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

int chili_suite_get(struct chili_suite **suite)
{
    *suite = &_suite;
    return 1;
}

void chili_suite_end()
{
    debug_print("Ending suite\n");

    free(_suite.tests);
    _suite.tests = NULL;
}
