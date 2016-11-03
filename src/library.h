#pragma once

#include "handle.h"
#include "run.h"


/**
 * @brief Initializes a library containing tests.
 *
 * Abstraction of a test library
 *
 * @param library_path Path to shared library containing
 *                     tests.
 * @param report_progress Callback to function that reports
 *                        progress while executing tests.
 * @param handle Instance handle, set on success.
 *
 * @return Negative on error.
 *         Positive on success.
 */
int chili_lib_create(const char *library_path,
                     chili_progress report_progress,
                     chili_handle *handle);

/**
 * @brief Prepares library for test execution.
 *
 * Should be called prior to executing any tests.
 * A succesful call to this function should always
 * have a matching call to the after function.
 *
 * @return Negative on error.
 *         Positive on success.
 */
int chili_lib_before_fixture(chili_handle handle);

/**
 * @brief Runs next test in library.
 *
 * @return Negative on error, positive on success, zero if
 *         no more tests exists in suite.
 *
 *         Note that a positive return value does not mean
 *         that the test passed or failed, just that the
 *         chili was able to execute it.
 */
int chili_lib_next_test(chili_handle handle,
                        int *index,
                        struct chili_times *times,
                        struct chili_result *result,
                        struct chili_aggregated *aggregated);

/**
 * @brief Runs test in library by name.
 *
 * @return Negative on error, positive on success.
 *
 *         Note that a positive return value does not mean
 *         that the test passed or failed, just that the
 *         chili was able to execute it.
 */
int chili_lib_named_test(chili_handle handle, const char *name);

/**
 * @brief Finishes test execution in this library.
 *
 * @return Negative on error.
 *         Positive on success.
 */
int chili_lib_after_fixture(chili_handle handle);

/**
 * @brief Prints all tests in library to stdout.
 *
 * Prints tests on format:
 * library_name:test_name1
 * library_name:test_name2
 *
 * @return Negative on error.
 *         Positive on success.
 */
int chili_lib_print_tests(chili_handle handle);

/**
 * @brief Frees allocated resources.
 *
 * @param handle Valid module handle.
*/
void chili_lib_destroy(chili_handle handle);
