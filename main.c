#include <stdio.h>

#include "symbols.h"
#include "suite.h"
#include "run.h"

static void _print_fixture(struct chili_suite *suite)
{
    printf("Fixture:");
    if (suite->once_before){
        printf("%s, ", suite->once_before);
    }
    if (suite->once_after){
        printf("%s", suite->once_after);
    }
    printf("\n");
}

static int _run_suite(const char *path, struct chili_suite *suite)
{
    int r, e;
    struct chili_result result;

    r = chili_run_begin(path, suite);
    printf("hello\n");
    if (r < 0){
        /* Tests arent safe to run when
         * initialization failed */
        return r;
    }

    do {
        r = chili_run_next(&result);
    } while (r != 0);

    //r = chili_run_tests();
    e = chili_run_end();

    /* Prefer error from test run as return value */
    return r < 0 ? r : e;
}

int main()
{
    int symbol_count;
    int i;
    int r;
    char *name;
    struct chili_suite *suite;
    const char *path = "./chili_ex_fixture.so";

    /* Initialize modules */
    r = chili_sym_begin(path, &symbol_count);
    if (r < 0){
        goto cleanup;
    }
    r = chili_suite_begin(symbol_count);
    if (r < 0){
        goto cleanup;
    }

    /* Evaluate symbols to find tests and setup */
    for (i = 0; i < symbol_count; i++){
        r = chili_sym_next(i, &name);
        if (r < 0){
            goto cleanup;
        }
        r = chili_suite_eval(name);
        if (r < 0){
            goto cleanup;
        }
    }

    chili_suite_get(&suite);
    r = _run_suite(path, suite);

cleanup:
    chili_suite_end();
    chili_sym_end();

    return r < 0 ? 1 : 0;
}
