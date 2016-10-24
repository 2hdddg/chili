#pragma once

#include "handle.h"

int chili_lib_create(const char *path, chili_handle *handle);
int chili_lib_before_fixture(chili_handle handle);


/**
 * @brief Runs next test in suite.
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
int chili_lib_print_tests(chili_handle handle);
int chili_lib_named_test(chili_handle handle, const char *name);
int chili_lib_after_fixture(chili_handle handle);
void chili_lib_destroy(chili_handle handle);
