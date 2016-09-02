#include <string.h>
#include <stdio.h>
#include <getopt.h>
#include <stdbool.h>

#include "symbols.h"
#include "suite.h"
#include "run.h"
#include "redirect.h"
#include "report.h"

/* Debugging */
#define DEBUG 1
#include "debug.h"


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

enum parsed_commandline {
    run_suite,
    display_usage,
    /* Errors */
    error_no_suite_specified,
    error_redirect_path_too_long,
};

struct aggr_result {
    int num_succeeded;
    int num_failed;
};


/* Globals */
struct commandline_options _options;


static int _run_suite(const char *path,
                      struct chili_suite *suite,
                      struct chili_report *report,
                      struct aggr_result *aggr_result)
{
    int r;
    struct chili_result result;

    r = chili_redirect_begin(_options.use_redirect,
                             _options.redirect_path);
    if (r < 0){
        return r;
    }

    r = chili_report_begin(report);
    if (r < 0){
        chili_redirect_end();
        return r;
    }

    r = chili_run_begin(path, chili_report_test_begin, suite);
    if (r < 0){
        /* Tests arent safe to run when
         * initialization failed */
        chili_report_end();
        chili_redirect_end();
        return r;
    }

    do {
        /*   0 if there were no more tests,
         * > 0 if setup, test and teardown ran without error,
         * < 0 if any of above encountered an error */
        r = chili_run_next(&result);

        /* Report even if success, failure or error */
        if (r != 0){
            chili_report_test(&result);
        }

        if (r > 0){
            if (result.test > 0){
                aggr_result->num_succeeded++;
            }
            else{
                aggr_result->num_failed++;
            }
        }
    } while (r > 0); /* Stop executing on error */

    if (r < 0){
        chili_run_end();
        chili_report_end();
        chili_redirect_end();
        return r;
    }

    r = chili_run_end();
    chili_report_end();
    chili_redirect_end();
    return r;
}


static enum parsed_commandline _parse_args(int argc, char *argv[])
{
    int c;
    const char *short_options = "icmrh:";
    const struct option long_options[] = {
        /*   Turns on color, cursor movements and redirect.
         *   Default path for redirect is ./chili_log */
        { "interactive", no_argument,       0, 'i' },
        { "color",       no_argument,       0, 'c' },
        { "cursor",      no_argument,       0, 'm' },
        { "redirect",    required_argument, 0, 'r' },
        { "help",        no_argument,       0, 'h' },
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
                    return error_redirect_path_too_long;
                }
                _options.use_redirect = 1;
                strcpy(_options.redirect_path, optarg);
                break;
            case 'h':
                return display_usage;
        }
    } while (c != -1);

    if (optind < argc){
        _options.suite_path = argv[optind];
    }
    else{
        return error_no_suite_specified;
    }

    return run_suite;
}

void _display_usage()
{
    puts("chili - test runner\n"
         "Usage: chili [OPTIONS]... FILE\n"
         "FILE is a shared library containing tests\n"
         "to be invoked.\n"
         "\n"
         "Options are:\n"
         "  -c --color         use colored output\n"
         "  -m --cursor        minimize output on console,\n"
         "                     moves cursor\n"
         "  -r --redirect\n"
         "  -i --interactive   short for -c -m -r\n");
}

int main(int argc, char *argv[])
{
    int symbol_count;
    int i;
    int r;
    char *name;
    struct chili_report report;
    struct chili_suite *suite;
    struct aggr_result aggr_result = { 0 };

    switch (_parse_args(argc, argv)){
        case error_redirect_path_too_long:
            printf("Redirect path is too long.\n");
            return 1;

        case error_no_suite_specified:
            printf("Specify path to shared library "
                   "containing test suite.\n");
            return 1;

        case display_usage:
            _display_usage();
            return 1;

        case run_suite:
            break;
    }

    report.name = _options.suite_path;
    report.use_color = _options.use_color;
    report.use_cursor = _options.use_cursor;

    /* Initialize modules */
    r = chili_sym_begin(_options.suite_path, &symbol_count);
    if (r < 0){
        goto cleanup_sym;
    }
    r = chili_suite_begin(symbol_count);
    if (r < 0){
        goto cleanup_suite;
    }

    /* Evaluate symbols to find tests and setup */
    do {
        i = chili_sym_next(&name);
        if (i < 0){
            goto cleanup_suite;
        }
        else if (i > 0){
            r = chili_suite_eval(name);
            if (r < 0){
                goto cleanup_suite;
            }
        }
        else{
            break;
        }
    } while (true);

    chili_suite_get(&suite);
    r = _run_suite(_options.suite_path, suite, &report, &aggr_result);

cleanup_suite:
    chili_suite_end();
cleanup_sym:
    chili_sym_end();

    /* Errors triumphs */
    if (r < 0){
        return r;
    }

    debug_print("Exiting process\n"
                "\tnum_succeeded: %d\n"
                "\tnum_failed: %d\n",
                aggr_result.num_succeeded,
                aggr_result.num_failed);

    return aggr_result.num_failed;
}
