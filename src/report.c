#include <stdio.h>

#include "run.h"
#include "redirect.h"
#include "report.h"


/* Globals */
static struct chili_report *_report;


const char *_stats = "%sExecuted: %d, Succeeded: %d, Failed: %d, Errors: %d%s\n";

/* Nice stats */
const char *_stats_nothing = "%sNo tests executed%s\n";
const char *_stats_all_succeded = "%sExecuted %d tests, all succeeded%s\n";
const char *_stats_all_failed = "%sExecuted %d tests, all failed%s\n";
const char *_stats_all_errors = "%sExecuted %d tests, all with errors%s\n";
const char *_stats_some_failed = "%sExecuted %d tests, %d failed%s\n";
const char *_stats_some_failed_errors = "%sExecuted %d tests, "
                                        "%d failed, %d succeeded, %d errors%s\n";
/* Ansi escape codes for colors and stuff */
const char *_color_headline_ansi = "\x1b[1m\x1b[34m";
const char *_color_success_ansi = "\x1b[32m";
const char *_color_fail_ansi = "\x1b[31;1m";
const char *_color_reset_ansi = "\033[0m";

const char *_cursor_up = "\x1b[A";
const char *_clear_to_end = "\x1b[K";

/* Used at runtime, set to empty strings if colors
 * are disabled */
const char *_color_headline;
const char *_color_success;
const char *_color_fail;
const char *_color_reset;


static void _print_nice_stats(struct chili_aggregated *aggregated)
{
    int fails = aggregated->num_failed;
    int errors = aggregated->num_errors;
    int successes = aggregated->num_succeeded;
    int total = aggregated->num_total;

    if (total == 0){
        printf(_stats_nothing, _color_fail, _color_reset);
    }
    else if (total == successes){
        printf(_stats_all_succeded, _color_success,
            total, _color_reset);
    }
    else if (total == fails){
        printf(_stats_all_failed, _color_fail,
            total, _color_reset);
    }
    else if (total == errors){
        printf(_stats_all_errors, _color_fail,
            total, _color_reset);
    }
    else{
        if (errors == 0){
            printf(_stats_some_failed, _color_fail,
                total, fails, _color_reset);
        }
        else{
            printf(_stats_some_failed_errors, _color_fail,
                total, fails, successes, errors,  _color_reset);
        }
    }
}

static void _print_stats(struct chili_aggregated *aggregated)
{
    if (_report->nice_stats){
        _print_nice_stats(aggregated);
        return;
    }

    const char *color = aggregated->num_errors || aggregated->num_failed ?
        _color_fail : _color_success;

    /* Simple stats */
    printf(_stats, color, aggregated->num_total,
          aggregated->num_succeeded, aggregated->num_failed,
          aggregated->num_errors, _color_reset);
}

static void _print_test(struct chili_result *result)
{
    bool print_captured_output = true;

    if (result->before == fixture_error){
        printf("%s%s: Test fixture setup error%s\n",
               _color_fail, result->name, _color_reset);
    }
    else if (result->after == fixture_error){
        printf("%s%s: Test fixture teardown error%s\n",
               _color_fail, result->name, _color_reset);
    }
    else{
        switch (result->test){
            case test_uncertain:
                printf("%s%s: Uncertain result%s\n",
                       _color_fail, result->name, _color_reset);
                break;
            case test_error:
                printf("%s%s: Error%s\n",
                       _color_fail, result->name, _color_reset);
                break;
            case test_failure:
                printf("%s%s: Failed%s\n",
                       _color_fail, result->name, _color_reset);
                break;
            case test_success:
                printf("%s%s: Success%s\n",
                       _color_success, result->name, _color_reset);
                print_captured_output = false;
                break;
        }
    }

    if (print_captured_output){
        chili_redirect_print(result->name,
            ">>> Capture start\n",
            "<<< Capture end\n");
    }
}

static void _print_result(struct chili_result *result)
{
    bool print_captured_output = true;

    switch (result->execution){
        case execution_not_started:
            printf("%s%s: Not started%s\n",
                   _color_fail, result->name, _color_reset);
            break;
        case execution_unknown_error:
            printf("%s%s: Unknown error%s\n",
                   _color_fail, result->name, _color_reset);
            break;
        case execution_crashed:
            printf("%s%s: Crashed%s\n",
                   _color_fail, result->name, _color_reset);
            break;
        case execution_timed_out:
            printf("%s%s: Timed out%s\n",
                   _color_fail, result->name, _color_reset);
            break;
        case execution_done:
            _print_test(result);
            print_captured_output = false;
            break;
    }

    if (print_captured_output){
        chili_redirect_print(result->name,
            ">>> Capture start\n",
            "<<< Capture end\n");
    }
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

void chili_report_suite_begin_fail(int r)
{
    printf("%sError in suite setup%s\n", _color_fail, _color_reset);
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
    if (_report->use_cursor){
        /* Remove text shown while running test */
        printf("%s%s", _cursor_up, _clear_to_end);
        /* Remove previous stats */
        if (aggregated->num_total > 1){
            printf("%s%s", _cursor_up, _clear_to_end);
        }
    }

    _print_result(result);

    if (_report->use_cursor){
        _print_stats(aggregated);
    }
}

void chili_report_suite_end_fail(int r)
{
    printf("%sError in suite teardown%s\n", _color_fail, _color_reset);
}

void chili_report_end(struct chili_aggregated *aggregated)
{
    if (!_report->use_cursor){
        _print_stats(aggregated);
    }
}
