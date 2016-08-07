#include <stdio.h>

#include "run.h"
#include "redirect.h"
#include "report.h"


/* Globals */
static struct chili_report *_report;
static int _num_executed;
static int _num_failed;

const char *_stats_all_succeded = "Executed %d tests, %sall succeeded%s\n";
const char *_stats_all_failed = "Executed %d tests, %sall failed%s\n";
const char *_stats_some_failed = "Executed %d tests, %s%d failed%s\n";

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

static void _print_stats()
{
    if (_num_executed == 0){
        return;
    }

    if (_num_failed){
        if (_num_failed == _num_executed){
            printf(_stats_all_failed,
                _num_executed, _color_fail, _color_reset);
        }
        else{
            printf(_stats_some_failed,
                _num_executed, _color_fail, _num_failed, _color_reset);
        }
    }
    else{
        printf(_stats_all_succeded,
            _num_executed, _color_success, _color_reset);
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

/* Exports */
int chili_report_begin(struct chili_report *report)
{
    _report = report;
    _num_executed = 0;
    _num_failed = 0;

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
void chili_report_test(struct chili_result *result)
{
    int failed = result->before < 0 ||
        result->test <= 0 ||
        result->after < 0;
    /* Update numbers for stats */
    _num_executed++;
    _num_failed += failed ? 1 : 0;

    if (_report->use_cursor){
        /* Remove text shown while running test */
        printf("%s%s", _cursor_up, _clear_to_end);
        /* Remove previous stats */
        if (_num_executed > 1){
            printf("%s%s", _cursor_up, _clear_to_end);
        }

        if (failed){
            _print_failure(result);
        }
        _print_stats();
    }
    else{
        if (failed){
            _print_failure(result);
        }
    }
}

void chili_report_end()
{
    if (!_report->use_cursor){
        _print_stats();
    }
}
