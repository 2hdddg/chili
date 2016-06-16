#pragma once

#include "suite.h"


/* Types */
struct chili_result {
    const char *name;
    int before;
    int test;
    int after;
    /* Path to stdout from test */
    const char *output;
};

typedef void (*chili_test_begin)(const char*);

int chili_run_begin(const char *path, chili_test_begin test_begin,
    struct chili_suite *suite);
/* Returns > 0 if test ran, 0 if no more tests(no result), < 0 failed to run test */
int chili_run_next(struct chili_result *result);
int chili_run_end();
