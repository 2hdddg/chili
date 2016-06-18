#include <string.h>
#include <stdio.h>
#include <getopt.h>

#include "symbols.h"
#include "suite.h"
#include "run.h"
#include "redirect.h"
#include "report.h"

/* Types */
struct commandline_options {
    int use_color;
    int is_interactive;
};


/* Globals */
struct commandline_options _options;


static int _run_suite(const char *path,
    struct chili_suite *suite, struct chili_report *report)
{
    int r, e;
    struct chili_result result;

    /* Ignore redirect initialization error for now, not fatal */
    chili_redirect_begin();
    r = chili_report_begin(report);
    if (r < 0){
        return r;
    }

    r = chili_run_begin(path, chili_report_test_begin, suite);
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
    chili_redirect_end();

    /* Prefer error from test run as return value */
    return r < 0 ? r : e;
}


static void _parse_args(int argc, char *argv[])
{
    /* Arguments:
        --color -c
        --interactive -i
    */
    int c;
    const char *short_options = "ci";
    const struct option long_options[] = {
        { "color", no_argument, 0, 'c' },
        { "interactive", no_argument, 0, 'i' },
    };
    int index;

    do {
        c = getopt_long(argc, argv, short_options, long_options, &index);
        switch (c){
            case 'c':
                _options.use_color = 1;
                break;
            case 'i':
                _options.is_interactive = 1;
                break;
        }
    } while (c != -1);
}

int main(int argc, char *argv[])
{
    int symbol_count;
    int i;
    int r;
    char *name;
    struct chili_report report;
    struct chili_suite *suite;
    const char *path = argv[1];

    memset(&_options, 0, sizeof(struct commandline_options));
    _parse_args(argc, argv);

    report.name = path;
    report.use_color = _options.use_color;
    report.is_interactive = _options.is_interactive;

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
    r = _run_suite(path, suite, &report);

cleanup:
    chili_suite_end();
    chili_sym_end();

    return r < 0 ? 1 : 0;
}
