#pragma once
#include <stdbool.h>

#include "redirect.h"


struct chili_test_options {
    /* Colorized output */
    bool use_color;
    /* Minimize output by moving cursor and overwrite
     * "uninteresting" console output */
    bool use_cursor;
    /* Redirect stdout while running tests to minimize
     * amount of console output. Output from failing
     * tests will be shown. */
    bool use_redirect;
    /* Print stats that is harder to parse but nicer
     * to read. */
    bool nice_stats;
    /* Path to directory where test stdout will be put */
    char redirect_path[CHILI_REDIRECT_MAX_PATH];
};

/**
 * @brief Runs all tests
 *
 * Runs all tests in specified shared libraries.
 *
 * @param library_paths Array of paths to shared library containing
 *                      tests.
 * @param num_libraries Number of entries in array.
 * @param options       Options to use when running tests.
 *
 * @return Negative on error.
 *         Zero when all tests succeeded.
 *         Positive on test error/failure.
 */
int chili_command_all(const char **library_paths,
                      int num_libraries,
                      const struct chili_test_options *options);

/**
 * @brief Runs named tests
 *
 * Runs named tests read from file.
 * Tests are executed in the order they are read from
 * file.
 *
 * @param names_path    Path to file containing names
 *                      of tests to run. If set to null
 *                      stdin is used instead.
 * @param options       Options to use when running tests.
 *
 * @return Negative on error.
 *         Zero when all tests succeeded.
 *         Positive on test error/failure.
 */
int chili_command_named(const char *names_path,
                        const struct chili_test_options *options);

/**
 * @brief Debugs the named test
 *
 * Attaches debugger to the test right before it
 * is executed.
 *
 * @param chili_path    Path to chili executable used to
 *                      invoke the test.
 * @param test_name     Name of test to debug, should include
 *                      path and name of shared library containing
 *                      the test.
 *
 * @return Negative on error.
 *         Otherwise, no return at all, debugger takes over
 *         the process.
 */
int chili_command_debug(const char *chili_path,
                        char *test_name);


/**
 * @brief Prints list of tests in suite.
 *
 * @param library_paths Array of paths to shared library containing
 *                      tests.
 * @param num_libraries Number of entries in array.
 *
 * @return Negative on error.
 */
int chili_command_list(const char **library_paths,
                       int num_libraries);

