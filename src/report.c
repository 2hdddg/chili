#include <stdio.h>

#include "run.h"
#include "redirect.h"
#include "report.h"


/* Globals */
static struct chili_report *_report;

const char *_stats_all_succeded = "Executed %d tests, %sall succeeded%s\n";
const char *_stats_all_failed = "Executed %d tests, %sall failed%s\n";
const char *_stats_all_errors = "Executed %d tests, %sall with errors%s\n";
const char *_stats_some_failed = "Executed %d tests, %s%d failed%s\n";
const char *_stats_some_failed_errors = "Executed %d tests, "
                                        "%s%d failed, %d errors%s\n";
const char *_color_headline_ansi = "\x1b[34m";
const char *_color_success_ansi = "\x1b[32m";
const char *_color_fail_ansi = "\x1b[31;1m";
const char *_color_reset_ansi = "\033[0m";

/* Used in runtime */
const char *_color_headline;
const char *_color_success;
const char *_color_fail;
const char *_color_reset;

const char *_cursor_up = "\x1b[A";
const char *_clear_to_end = "\x1b[K";

static void _print_stats(struct chili_aggregated *aggregated)
{
    int fails = aggregated->num_failed;
    int errors = aggregated->num_errors;
    int successes = aggregated->num_succeeded;
    int total = aggregated->num_total;

    if (total == 0){
    }
    else if (total == successes){
        printf(_stats_all_succeded,
            total, _color_success, _color_reset);
    }
    else if (total == fails){
        printf(_stats_all_failed,
            total, _color_fail, _color_reset);
    }
    else if (total == errors){
        printf(_stats_all_errors,
            total, _color_fail, _color_reset);
    }
    else{
        if (errors == 0){
            printf(_stats_some_failed,
                total, _color_fail, fails, _color_reset);
        }
        else{
            printf(_stats_some_failed_errors,
                total, _color_fail, fails, errors,  _color_reset);
        }
    }
}

static void _print_failure(struct chili_result *result)
{
    printf("%sFailed:%s %s\n",
        _color_fail, _color_reset, result->name);

    chili_redirect_print(result->name,
        ">>> Capture start\n",
        "<<< Capture end\n");
}

static void _print_error(struct chili_result *result)
{
    if (result->before < 0){
        printf("%sError in test setup for%s %s\n",
            _color_fail, _color_reset, result->name);
    }
    if (result->after < 0){
        printf("%sError in test cleanup for%s %s\n",
            _color_fail, _color_reset, result->name);
    }
    if (result->test < 0){
        printf("%sError in test%s %s\n",
            _color_fail, _color_reset, result->name);
    }

    chili_redirect_print(result->name,
        ">>> Capture start\n",
        "<<< Capture end\n");
}

/* Exports */
int chili_report_begin(struct chili_report *report)
{
    _report = report;

    if (report->use_color){
        _color_headline = _color_headline_ansi;
        _color_success = _color_success_ansi;
        _color_fail = _color_fail_ansi;
        _color_reset = _color_reset_ansi;
    }
    else{
        _color_headline = _color_success =
            _color_fail = _color_reset = "";
    }

    printf("%sRunning suite %s%s\n",
        _color_headline, report->name, _color_reset);

    return 1;
}

void chili_report_test_begin(const char *name)
{
    if (_report->use_cursor){
        printf("Running test %s\n", name);
    }
}

/*
Interactive layout:

While running first test
    Running...

While running another test
    Stats
    Running...

While running a test after a failed one
    Failed test
    Stats
    Running...

Completed with two failures
    Failed test 1
    Failed test 2
    Stats

Completed no failures
    Stats
*/
void chili_report_test(struct chili_result *result,
                       struct chili_aggregated *aggregated)
{
    int failed = result->test == 0;
    int error = result->before < 0 ||
                result->test < 0 ||
                result->after < 0;

    if (_report->use_cursor){
        /* Remove text shown while running test */
        printf("%s%s", _cursor_up, _clear_to_end);
        /* Remove previous stats */
        if (aggregated->num_total > 1){
            printf("%s%s", _cursor_up, _clear_to_end);
        }

        if (failed){
            _print_failure(result);
        }
        if (error){
            _print_error(result);
        }
        _print_stats(aggregated);
    }
    else{
        if (failed){
            _print_failure(result);
        }
        if (error){
            _print_error(result);
        }
    }
}

void chili_report_end(struct chili_aggregated *aggregated)
{
    if (!_report->use_cursor){
        _print_stats(aggregated);
    }
}
