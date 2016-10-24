#pragma once

#include <stdbool.h>
#include <sys/time.h>

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

struct chili_times {
    struct timespec timeout;
    struct timespec progress_tick;
};

typedef void (*chili_test_begin)(const char*);

/**
 * @brief Runs before fixture.
 *
 * @param fixture       Set of functions to call when suite is setup,
 *                      cleaned up and also functions to call before
 *                      each test and after each test.
 * @return Negative on error, positive on success.
 */
int chili_run_before(const struct chili_bind_fixture *fixture);

/**
 * @brief Invokes test.
 *
 * @param times         Timing configurations used when tests and
 *                      fixtures are executed.
 * @return Negative on error, positive on success.
 *         Success doesnt mean that the test succeeded only
 *         that no errors occured.
*/
int chili_run_test(struct chili_result *result,
                   struct chili_aggregated *aggregated,
                   const struct chili_bind_test *test,
                   const struct chili_bind_fixture *fixture,
                   const struct chili_times *times,
                   chili_test_begin test_begin);

/**
 * @brief Runs after fixture.
 *
 * Applies fixture cleanup.
 *
 * @return Negative on error, positive on success.
 */
int chili_run_after(const struct chili_bind_fixture *fixture);

