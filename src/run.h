#pragma once

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
                    chili_test_begin test_begin,
                    struct chili_suite *suite);

/**
 * @brief Invokes test.
 *
 * Will invoke next test in the suite. Applies fixtures.
 *
 * @return Negative on error, positive on success and zero if there were
 *         no more tests to execute.
*/
int chili_run_next(struct chili_result *result);

/**
 * @brief Releases module
 *
 * Applies fixture.
 *
 * @return Negative on error, positive on success.
 */
int chili_run_end();

