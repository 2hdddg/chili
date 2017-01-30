#pragma once

#include <stdbool.h>
#include <sys/time.h>

#include "bind.h"

/* Initial state is uncertain.
 *
 * not_needed - When no fixture function exists.
 * error      - Fixture function executed and return < 0.
 * success    - Fixture function executed and return >= 0.
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

/* Initial state is not_started
 *
 * unknown_error - Execution of test failed in
 *                 an unexpected way.
 * crashed       - Test or fixture crashed.
 * timed_out     - Test or fixture took too long.
 * done          - Test and fixture executed.
 */
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
    /* Name of test */
    const char            *name;
    /* Name of library containing test */
    const char            *library;
    /* Unique identity of test */
    int                   identity;
    enum execution_result execution;
    enum fixture_result   before;
    enum test_result      test;
    enum fixture_result   after;
};

/**
 * @brief Aggregated result of all executed tests
 *        so far.
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

typedef void (*chili_progress)(const char *library_path,
                               const char *test_name);

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
                   chili_progress test_progress);

/**
 * @brief Debugs test.
 *
 * @param debugger Debugger module instance.
 * @param test     Bound test to debug.
 * @param fixture  Bound fixture.
 *
 * @return Negative on error
 *         Never return on success.
 */
int chili_run_debug(chili_handle debugger,
                    const struct chili_bind_test *test,
                    const struct chili_bind_fixture *fixture);

/**
 * @brief Runs after fixture.
 *
 * Applies fixture cleanup.
 *
 * @return Negative on error, positive on success.
 */
int chili_run_after(const struct chili_bind_fixture *fixture);

