#pragma once

#include <stdbool.h>

#include "suite.h"


/**
 * @brief Represents execution of a single test case
 *
 * Members contain name and result of execution of
 * test setup, the test itself and cleanup. The results
 * are < 0 if an error occured, 0 if failed, > 0 if
 * successful.
 */
struct chili_result {
    const char *name;
    int before;
    int test;
    int after;
};

struct chili_aggregated {
    int num_succeeded;
    int num_failed;
    int num_errors;
    int num_total;
};

typedef void (*chili_test_begin)(const char*);

/**
 * @brief Initializes module.
 *
 * Needs a path to shared library containing tests and a
 * suite containing names of symbols for fixtures and tests.
 * Applies fixture.
 *
 * @return Negative on error, positive on success.
 */
int chili_run_begin(const char *path,
                    struct chili_suite *suite);

/**
 * @brief Invokes test.
 *
 * Will invoke next test in the suite. Applies fixtures.
 *
 * @return Negative on error, positive on success and zero if there were
 *         no more tests to execute. Success doesnt mean that the test
 *         succeeded only that no errors occured.
*/
int chili_run_next(struct chili_result *result,
                   struct chili_aggregated *aggregated,
                   chili_test_begin test_begin);

/**
 * @brief Releases module
 *
 * Applies fixture.
 *
 * @return Negative on error, positive on success.
 */
int chili_run_end();

