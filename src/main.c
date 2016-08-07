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
    /* Path to shared library containing tests */
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
    /* Path to directory where test stdout will be put */
    char redirect_path[CHILI_REDIRECT_MAX_PATH];
};


/* Globals */
struct commandline_options _options;


static int _run_suite(const char *path,
    struct chili_suite *suite, struct chili_report *report)
{
    int r, e;
    struct chili_result result;

    r = chili_redirect_begin(_options.use_redirect,
        _options.redirect_path);
    if (r < 0){
        return r;
    }

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

    memset(&_options, 0, sizeof(struct commandline_options));

    do {
        c = getopt_long(argc, argv, short_options, 
                long_options, &index);
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
                if (len >= CHILI_REDIRECT_MAX_PATH){
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
    else{
        printf("No shared library file specified.\n");
        return -1;
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
    do {
        i = chili_sym_next(&name);
        if (i < 0){
            goto cleanup;
        }
        r = chili_suite_eval(name);
        if (r < 0){
            goto cleanup;
        }
    } while (i > 0);

    chili_suite_get(&suite);
    r = _run_suite(_options.suite_path, suite, &report);

cleanup:
    chili_suite_end();
    chili_sym_end();

    return r < 0 ? 1 : 0;
}
