#pragma once

struct chili_report {
    const char* name;
    int use_color;
    int is_interactive;
};


int chili_report_begin(struct chili_report *report);
void chili_report_test_begin(const char *name);
void chili_report_test(struct chili_result *result);
void chili_report_end();
