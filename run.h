#pragma once

#include "suite.h"

struct chili_result {
    const char *name;
    int before;
    int test;
    int after;
};

int chili_run_begin(const char *path, struct chili_suite *suite);
/* Returns > 0 if test ran, 0 if no more tests(no result), < 0 failed to run test */
int chili_run_next(struct chili_result *result);
int chili_run_end();
