#pragma once

#include <stdbool.h>

#include "run.h"

struct chili_report {
    const char* name;
    bool use_color;
    bool use_cursor;
    bool nice_stats;
};

int chili_report_begin(struct chili_report *report);
void chili_report_test_begin(const char *library,
                             const char *name);
void chili_report_suite_begin_fail(int r);
void chili_report_test(struct chili_result *result,
                       struct chili_aggregated *aggregated);
void chili_report_suite_end_fail(int r);
void chili_report_end(struct chili_aggregated *aggregated);
