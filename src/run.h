#pragma once

#include <stdbool.h>

#include "bind.h"

/* Initial state is uncertain.
 *
 * nothing_to_do - When no fixture function exists.
 * error         - Fixture function executed and return < 0.
 * success       - Fixture function executed and return >= 0.
*/
enum fixture_result {
    fixture_uncertain,
    fixture_not_needed,
    fixture_error,
    fixture_success
};

/* Initial state is uncertain.
 *
 * error   - Test function executed and returned < 0.
 * failure - Test function executed and returned 0.
 * success - Test function executed and returned > 0.
*/
enum test_result {
    test_uncertain,
    test_error,
    test_failure,
    test_success,
};

enum execution_result {
    execution_not_started,
    execution_unknown_error,
    execution_crashed,
    execution_timed_out,
    execution_done,
};

/**
 * @brief Represents execution of a single test case
 *
 * Members contain name and result of execution of
 * test setup, the test itself and cleanup.
 */
struct chili_result {
    const char            *name;
    enum execution_result execution;
    enum fixture_result   before;
    enum test_result      test;
    enum fixture_result   after;
};

/**
 * @brief Aggregated result of all executed tests
          so far.
*/
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
 * Applies fixture.
 *
 * @param before_failed Is set to true when suite setup fails.
 *                      When true, the return value is the
 *                      return value from the failed setup
 *                      function.
 * @return Negative on error, positive on success.
 */
int chili_run_begin(const struct chili_bind_fixture *fixture,
                    bool *before_failed);

/**
 * @brief Invokes test.
 *
 * Will invoke next test in the suite. Applies fixtures.
 *
 * @return Negative on error, positive on success.
 *         Success doesnt mean that the test succeeded only
 *         that no errors occured.
*/
int chili_run_next(struct chili_result *result,
                   struct chili_aggregated *aggregated,
                   struct chili_bind_test *test,
                   chili_test_begin test_begin);

/**
 * @brief Releases module
 *
 * Applies fixture cleanup.
 *
 * @param after_failed Is set to true when suite cleanup fails.
 *                     When true, the return value is the return
 *                     value from the failed setup function.
 *
 * @return Negative on error, positive on success.
 */
int chili_run_end(bool *after_failed);

