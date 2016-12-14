#include <stdio.h>
#include <string.h>

#include "assert.h"

int assert_str(const char *expected, const char *actual)
{
    if (expected == NULL && actual == NULL){
        return 1;
    }

    if (actual == NULL){
        printf("Expected: '%s' but was NULL\n", expected);
        return 0;
    }

    if (expected == NULL){
        printf("Expected NULL but was: '%s'\n", actual);
        return 0;
    }

    if (strcmp(expected, actual) == 0){
        return 1;
    }

    printf("Expected: '%s' but was: '%s'\n", expected, actual);
    return 0;
}

int assert_str_e(const char *expected, const char *actual,
                 const char *explanation)
{
    int ret = assert_str(expected, actual);
    if (ret <= 0){
        printf("%s", explanation);
    }
    return ret;
}

int assert_int(int expected, int actual)
{
    if (expected != actual){
        printf("Expected: %d but was: %d\n", expected, actual);
        return 0;
    }
    return 1;
}

int assert_null(void* p)
{
    if (p != NULL){
        printf("Value is not NULL when expected NULL\n");
        return 0;
    }
    return 1;
}

int assert_not_null(void* p)
{
    if (p == NULL){
        printf("Value is NULL when expected not NULL\n");
        return 0;
    }
    return 1;
}

int assert_ret_success(int ret)
{
    if (ret >= 0){
        return 1;
    }

    printf("Expected function to succeed but "
           "return value was: %d\n", ret);
    return 0;
}

int assert_ret_fail(int ret)
{
    if (ret < 0){
        return 1;
    }

    printf("Expected function to fail but "
           "return value was: %d\n", ret);
    return 0;
}

int assert_timespec_less_than(struct timespec *a,
                              struct timespec *b)
{
    if (a->tv_sec < b->tv_sec ||
        (a->tv_sec == b->tv_sec && a->tv_nsec < b->tv_nsec)){
        return 1;
    }

    printf("Expected %lld.%.9ld to be less "
           "than %lld.%.9ld.",
           (long long)a->tv_sec, a->tv_nsec,
           (long long)b->tv_sec, a->tv_nsec);

    return 0;
}
