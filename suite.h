#pragma once

struct chili_suite {
    const char *once_before;
    const char *once_after;
    const char *each_before;
    const char *each_after;
    char **tests;
    int count;
};

int chili_suite_begin(int max_count);
int chili_suite_eval(char *symbol);
int chili_suite_get(struct chili_suite **suite);
void chili_suite_end();
