#pragma once

#include "handle.h"


struct chili_suite {
    const char *once_before;
    const char *once_after;
    const char *each_before;
    const char *each_after;
    char **tests;
    int count;
};

/* @brief Creates skeleton for a suite.
 *
 * Symbols need to be evaluated to build a complete
 * suite.
 *
 * @param max_count Max number of symbols that will be
 *                  evaluated.
 * @param handle    Instance handle set on success.
 *
 * @return Negative on error.
 *         Positive on success.
*/
int chili_suite_create(int max_count, chili_handle *handle);

/* @brief Evaluates a symbol.
 *
 * Adds symbol to suite if it matches. Call once for each symbol.
 * Can only be called after chili_suite_begin.
 *
 * @param handle Valid module handle.
 * @param symbol Symbol name.
 *
 * @return Negative on error.
 *         Positive on success.
*/
int chili_suite_eval(chili_handle handle, char *symbol);

/* @brief Retrieves the suite.
 *
 * Should be called after all symbols have been evaluated.
 *
 * @param handle Valid module handle.
 * @param suite  Set to result of evaluation, pointer
 *               is valid until call to destroy.
 *
 * @return Negative on error.
 *         Positive on success.
*/
int chili_suite_get(chili_handle handle, const struct chili_suite **suite);

/* @brief Frees allocated resources.
 *
 * Pointer to suite is invalid after calling
 * this
 *
 * @param handle Valid module handle.
*/
void chili_suite_destroy(chili_handle handle);

