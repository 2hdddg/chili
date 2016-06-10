#include <stdio.h>

#include "symbols.h"
#include "suite.h"
#include "run.h"
#include "report.h"


static int _run_suite(const char *path, struct chili_suite *suite)
{
    int r, e;
    struct chili_result result;

    r = chili_report_begin();
    if (r < 0){
        return r;
    }

    r = chili_run_begin(path, suite);
    if (r < 0){
        /* Tests arent safe to run when
         * initialization failed */
        return r;
    }

    do {
        /* Returns 0 when there is no more tests */
        r = chili_run_next(&result);
        if (r != 0){
            chili_report_test(&result);
        }
    } while (r != 0);

    e = chili_run_end();
    chili_report_end();

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
