#include <string.h>
#include <stdio.h>
#include <getopt.h>

#include "symbols.h"
#include "suite.h"
#include "run.h"
#include "redirect.h"
#include "report.h"


/* Defines */
#define PATH_LENGTH 255

/* Types */
struct commandline_options {
    const char *suite_path;
    /* Colorized output */
    int use_color;
    /* Minimize output by moving cursor and overwrite
     * "uninteresting" console output */
    int use_cursor;
    /* Redirect stdout while running tests to minimize
     * amount of console output. Output from failing
     * tests will be shown. */
    int use_redirect;
    char redirect_path[PATH_LENGTH];
};


/* Globals */
struct commandline_options _options;


static int _run_suite(const char *path,
    struct chili_suite *suite, struct chili_report *report)
{
    int r, e;
    struct chili_result result;

    /* Ignore redirect initialization error for now, not fatal */
    chili_redirect_begin(_options.use_redirect, _options.redirect_path);
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


static int _parse_args(int argc, char *argv[])
{
    int c;
    const char *short_options = "icmr:";
    const struct option long_options[] = {
        /*   Turns on color, cursor movements and redirect.
         *   Default path for redirect is ./chili_log */
        { "interactive", no_argument, 0, 'i' },
        { "color", no_argument, 0, 'c' },
        { "cursor", no_argument, 0, 'm' },
        { "redirect", required_argument, 0, 'r' },
    };
    int index;
    int len;

    do {
        c = getopt_long(argc, argv, short_options, long_options, &index);
        switch (c){
            case 'i':
                _options.use_color = 1;
                _options.use_cursor = 1;
                _options.use_redirect = 1;
                strcpy(_options.redirect_path, "./chili_log");
                break;
            case 'c':
                _options.use_color = 1;
                break;
            case 'm':
                _options.use_cursor = 1;
                break;
            case 'r':
                len = strlen(optarg);
                if (len >= PATH_LENGTH){
                    printf("Redirect path too long.\n");
                    return -1;
                }
                _options.use_redirect = 1;
                strcpy(_options.redirect_path, optarg);
        }
    } while (c != -1);

    if (optind < argc){
        _options.suite_path = argv[optind];
    }

    return 1;
}

int main(int argc, char *argv[])
{
    int symbol_count;
    int i;
    int r;
    char *name;
    struct chili_report report;
    struct chili_suite *suite;

    memset(&_options, 0, sizeof(struct commandline_options));
    if (_parse_args(argc, argv) < 0){
        return 0;
    }

    report.name = _options.suite_path;
    report.use_color = _options.use_color;
    report.use_cursor = _options.use_cursor;

    /* Initialize modules */
    r = chili_sym_begin(_options.suite_path, &symbol_count);
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
    r = _run_suite(_options.suite_path, suite, &report);

cleanup:
    chili_suite_end();
    chili_sym_end();

    return r < 0 ? 1 : 0;
}
