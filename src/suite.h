#pragma once

struct chili_suite {
    const char *once_before;
    const char *once_after;
    const char *each_before;
    const char *each_after;
    char **tests;
    int count;
};

/* Starts building a suite of tests.
 *
 * max_count - number of symbols that will be
 *             evaluated
 *
 * Returns neg on error, pos on success.
*/
int chili_suite_begin(int max_count);

/* Evaluates a symbol. Adds symbol to suite
 * if it matches. Call once for each symbol.
 * Can only be called after chili_suite_begin.
 *
 * symbol - symbol name
 *
 * Returns neg on error, pos on success.
*/
int chili_suite_eval(char *symbol);

/* Retrieves the suite. Should be called after
 * all symbols have been evaluated.
 *
 * suite - set to result of evaluation, pointer
 *         is valid until call to chili_suite_end
 *
 * Returns neg on error, pos on success.
*/
int chili_suite_get(struct chili_suite **suite);

/* Frees allocated resources.
 * Pointer to suite is invalid after calling
 * this
*/
void chili_suite_end();
