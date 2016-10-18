#include <stdbool.h>

#include "symbols.h"
#include "report.h"
#include "run.h"
#include "command.h"

/* Debugging */
#define DEBUG 0
#include "debug.h"


static bool _continue_testing(struct chili_result *result,
                              struct chili_aggregated *aggregated)
{
    bool error_occured = result->before == fixture_error ||
                         result->after == fixture_error ||
                         result->test == test_error ||
                         result->execution == execution_unknown_error;

    return error_occured ? false : true;
}

static int _run_suite(const struct chili_test_options *options,
                      struct chili_report *report,
                      struct chili_aggregated *aggregated)
{
    int r;
    bool before_failed;
    bool after_failed;
    struct chili_result result;
    struct chili_bind_test test;
    struct chili_times times;

    times.timeout.tv_nsec = 0;
    times.timeout.tv_sec = 10;

    r = chili_redirect_begin(options->use_redirect,
                             options->redirect_path);
    if (r < 0){
        return r;
    }

    r = chili_report_begin(report);
    if (r < 0){
        chili_redirect_end();
        return r;
    }

    r = chili_run_begin(chili_bind_get_fixture(),
                        &before_failed);
    if (r < 0){
        /* Tests arent safe to run when
         * initialization failed */
        if (before_failed){
            chili_report_suite_begin_fail(r);
        }
        chili_report_end(aggregated);
        chili_redirect_end();
        return r;
    }

    do {
        /*   0 if there were no more tests,
         * > 0 if setup, test and teardown ran without error,
         * < 0 if any of above encountered an error */
        r = chili_bind_next_test(&test);
        if (r <=0){
            /* Fatal error, can not continue or last test
             * has already executed, no more (0)  */
            break;
        }

        r = chili_run_next(&result, aggregated, &test,
                           &times, chili_report_test_begin);
        if (r < 0){
            /* Fatal error, can not continue */
            break;
        }

        chili_report_test(&result, aggregated);

        if (!_continue_testing(&result, aggregated)){
            break;
        }
    } while (true); /* Stop executing on error */

    /* Preserve error from above */
    if (r < 0){
        chili_run_end(&after_failed);
    }
    else {
        r = chili_run_end(&after_failed);
    }
    if (after_failed){
        chili_report_suite_end_fail(r);
    }

    chili_report_end(aggregated);
    chili_redirect_end();
    return r;
}

static const char *_bool_str(bool b)
{
    return b ? "true" : "false";
}

static int _initialize_modules(const char *suite_path)
{
    int symbol_count;
    int r;

    r = chili_sym_begin(suite_path, &symbol_count);
    if (r < 0){
        return r;
    }
    r = chili_suite_begin(symbol_count);
    if (r < 0){
        chili_sym_end();
    }
    return r;
}

static int _get_suite(struct chili_suite **suite)
{
    int r;
    char *name;

    /* Evaluate symbols to find tests and setup */
    do {
        r = chili_sym_next(&name);
        if (r < 0){
            return r;
        }
        else if (r > 0){
            r = chili_suite_eval(name);
            if (r < 0){
                return r;
            }
        }
        else{
            break;
        }
    } while (true);

    chili_suite_get(suite);
    return 1;
}

static void _deinitialize_modules()
{
    chili_suite_end();
    chili_sym_end();
}

int chili_command_test(const char *suite_path,
                       const struct chili_test_options *options)
{
    int r;
    struct chili_report report;
    struct chili_suite *suite;
    struct chili_aggregated aggregated = { 0 };

    debug_print("Running test command with options:\n"
                "\tsuite_path: %s\n"
                "\tuse_color: %s\n"
                "\tuse_cursor: %s\n"
                "\tuse_redirect: %s\n"
                "\tnice_stats: %s\n"
                "\tredirect_path: %s\n",
                suite_path,
                _bool_str(options->use_color),
                _bool_str(options->use_cursor),
                _bool_str(options->use_redirect),
                _bool_str(options->nice_stats),
                options->redirect_path);

    report.name = suite_path;
    report.use_color = options->use_color;
    report.use_cursor = options->use_cursor;
    report.nice_stats = options->nice_stats;

    r = _initialize_modules(suite_path);
    if (r < 0){
        return r;
    }

    r = _get_suite(&suite);
    if (r < 0){
        goto cleanup_suite;
    }

    r = chili_bind_begin(suite_path, suite);
    if (r < 0){
        goto cleanup_suite;
    }
    r = _run_suite(options, &report, &aggregated);
    chili_bind_end();

cleanup_suite:
    _deinitialize_modules();

    /* Errors triumphs */
    if (r < 0){
        return r;
    }

    debug_print("Test command ended:\n"
                "\tnum_succeeded: %d\n"
                "\tnum_failed: %d\n"
                "\tnum_errors: %d\n",
                aggregated.num_succeeded,
                aggregated.num_failed,
                aggregated.num_errors);

    return aggregated.num_failed > 0 ? 0 : 1;
}

int chili_command_list(const char *suite_path)
{
    int r;
    struct chili_suite *suite;

    r = _initialize_modules(suite_path);
    if (r < 0){
        return r;
    }

    r = _get_suite(&suite);
    if (r < 0){
        goto cleanup_suite;
    }

    for (int i = 0; i < suite->count; i++){
        /* Print on format that can be consumed by
         * runner
         */
        printf("%s:%s\n", suite_path, suite->tests[i]);
    }

cleanup_suite:
    _deinitialize_modules();

    return r;
}
