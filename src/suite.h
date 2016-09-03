#pragma once

struct chili_suite {
    const char *once_before;
    const char *once_after;
    const char *each_before;
    const char *each_after;
    char **tests;
    int count;
};

/* @brief Starts building a suite of tests.
 *
 * @param max_count Number of symbols that will be
 *        evaluated.
 *
 * @return Negative on error, positive on success.
*/
int chili_suite_begin(int max_count);

/* @brief Evaluates a symbol.
 *
 * Adds symbol to suite if it matches. Call once for each symbol.
 * Can only be called after chili_suite_begin.
 *
 * @param symbol - symbol name
 *
 * Returns neg on error, pos on success.
*/
int chili_suite_eval(char *symbol);

/* @brief Retrieves the suite. 
 *
 * Should be called after all symbols have been evaluated.
 *
 * @param suite Set to result of evaluation, pointer
 *        is valid until call to chili_suite_end
 *
 * Returns neg on error, pos on success.
*/
int chili_suite_get(struct chili_suite **suite);

/* @brief Frees allocated resources.
 *
 * Pointer to suite is invalid after calling
 * this
*/
void chili_suite_end();

