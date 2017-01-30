#pragma once

#include <time.h>


int assert_str(const char *expected, const char *actual);
int assert_str_e(const char *expected, const char *actual,
                 const char *explanation);

int assert_int(int expected, int actual);

int assert_ret_success(int ret);
int assert_ret_fail(int ret);

int assert_ptr_not_null(void* p);
int assert_ptr_null(void* p);
int assert_ptr(const void *expected, const void *actual);

int assert_timespec_less_than(const struct timespec *a,
                              const struct timespec *b);

int assert_file_exists(const char *path);
int assert_file_doesnt_exists(const char *path);
int assert_file_not_modified_after(const char *path,
                                   const struct timespec *t);

