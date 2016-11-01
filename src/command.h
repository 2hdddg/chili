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
 * @brief Runs tests
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
int chili_command_test(const char **library_paths,
                       int num_libraries,
                       const struct chili_test_options *options);

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

