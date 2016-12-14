#pragma once

#include <time.h>


int assert_str(const char *expected, const char *actual);
int assert_str_e(const char *expected, const char *actual,
                 const char *explanation);

int assert_int(int expected, int actual);

int assert_ret_success(int ret);
int assert_ret_fail(int ret);

int assert_not_null(void* p);
int assert_null(void* p);

int assert_timespec_less_than(struct timespec *a,
                              struct timespec *b);
