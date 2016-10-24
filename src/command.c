#include <stdbool.h>

#include "report.h"
#include "library.h"
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

static int _run_suite(chili_handle lib_handle,
                      const struct chili_test_options *options,
                      struct chili_report *report,
                      struct chili_aggregated *aggregated)
{
    int r;
    struct chili_result result;
    struct chili_times times;
    int index = 0;

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

    r = chili_lib_before_fixture(lib_handle);
    if (r < 0){
        /* Tests arent safe to run when
         * initialization failed */
        chili_report_suite_begin_fail(r);
        chili_report_end(aggregated);
        chili_redirect_end();
        return r;
    }

    do {
        /*   0 if there were no more tests,
         * > 0 if setup, test and teardown ran without error,
         * < 0 if any of above encountered an error */
        r = chili_lib_next_test(lib_handle,
                                &index,
                                &times,
                                &result,
                                aggregated);
        if (r == 0){
            break;
        }
        chili_report_test(&result, aggregated);

        if (!_continue_testing(&result, aggregated)){
            break;
        }
    } while (true); /* Stop executing on error */

    /* Preserve error from above */
    if (r < 0){
        chili_lib_after_fixture(lib_handle);
    }
    else {
        r = chili_lib_after_fixture(lib_handle);
        if (r < 0){
            chili_report_suite_end_fail(r);
        }
    }

    chili_report_end(aggregated);
    chili_redirect_end();
    return r;
}

static const char *_bool_str(bool b)
{
    return b ? "true" : "false";
}

int chili_command_test(const char *library_path,
                       const struct chili_test_options *options)
{
    int r;
    struct chili_report report;
    struct chili_aggregated aggregated = { 0 };
    chili_handle lib_handle;

    debug_print("Running test command with options:\n"
                "\tsuite_path: %s\n"
                "\tuse_color: %s\n"
                "\tuse_cursor: %s\n"
                "\tuse_redirect: %s\n"
                "\tnice_stats: %s\n"
                "\tredirect_path: %s\n",
                library_path,
                _bool_str(options->use_color),
                _bool_str(options->use_cursor),
                _bool_str(options->use_redirect),
                _bool_str(options->nice_stats),
                options->redirect_path);

    report.name = library_path;
    report.use_color = options->use_color;
    report.use_cursor = options->use_cursor;
    report.nice_stats = options->nice_stats;

    r = chili_lib_create(library_path, &lib_handle);
    if (r < 0){
        return r;
    }

    r = _run_suite(lib_handle, options, &report, &aggregated);
    chili_lib_destroy(lib_handle);

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

int chili_command_list(const char *library_path)
{
    int r;
    chili_handle lib_handle;

    r = chili_lib_create(library_path, &lib_handle);
    if (r < 0){
        return r;
    }
    r = chili_lib_print_tests(lib_handle);
    chili_lib_destroy(lib_handle);

    return r;
}
