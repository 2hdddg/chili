#include <string.h>
#include <stdio.h>

#include "suite.h"


chili_handle _handle;
const int _max_count = 3;
int _r;

int each_before()
{
    _r = chili_suite_create(_max_count, &_handle);

    return _r;
}

int each_after()
{
    chili_suite_destroy(_handle);
    return 1;
}

/* Verifies simplest case of creation
 */
int test_suite_create()
{
    chili_handle handle = NULL;
    const int max_count = 10;
    int r;

    r = chili_suite_create(max_count, &handle);
    chili_suite_destroy(handle);

    return r > 0 && handle != NULL;
}

/* Verifies that 'once before' is added to suite
 * and that rest of suite is empty.
 */
int test_suite_eval_once_before()
{
    const struct chili_suite *suite;

    chili_suite_eval(_handle, "once_before");

    chili_suite_get(_handle, &suite);
    return suite->once_before != NULL &&
           suite->once_after == NULL &&
           suite->each_before == NULL &&
           suite->each_after == NULL &&
           suite->count == 0;
}

/* Verifies that 'once after' is added to suite
 * and that rest of suite is empty.
 */
int test_suite_eval_once_after()
{
    const struct chili_suite *suite;

    chili_suite_eval(_handle, "once_after");

    chili_suite_get(_handle, &suite);
    return suite->once_before == NULL &&
           suite->once_after != NULL &&
           suite->each_before == NULL &&
           suite->each_after == NULL &&
           suite->count == 0;
}

/* Verifies that 'each before' is added to suite
 * and that rest of suite is empty.
 */
int test_suite_eval_each_before()
{
    const struct chili_suite *suite;

    chili_suite_eval(_handle, "each_before");

    chili_suite_get(_handle, &suite);
    return suite->once_before == NULL &&
           suite->once_after == NULL &&
           suite->each_before != NULL &&
           suite->each_after == NULL &&
           suite->count == 0;
}

/* Verifies that 'each after' is added to suite
 * and that rest of suite is empty.
 */
int test_suite_eval_each_after()
{
    const struct chili_suite *suite;

    chili_suite_eval(_handle, "each_after");

    chili_suite_get(_handle, &suite);
    return suite->once_before == NULL &&
           suite->once_after == NULL &&
           suite->each_before == NULL &&
           suite->each_after != NULL &&
           suite->count == 0;
}

/* Verifies that test is added to suite and
 * that the rest of suite is empty.
 */
int test_suite_eval_test()
{
    const struct chili_suite *suite;

    chili_suite_eval(_handle, "test_whatever");

    chili_suite_get(_handle, &suite);
    return suite->once_before == NULL &&
           suite->once_after == NULL &&
           suite->each_before == NULL &&
           suite->each_after == NULL &&
           suite->count == 1 &&
           strcmp(suite->tests[0], "test_whatever") == 0;
}

/* Verifies return value from succesful eval
 */
int test_suite_eval_success()
{
    int r;

    r = chili_suite_eval(_handle, "test_should_fit");

    return r > 0;
}

/* Verifies return value when evaluating
 * something that isn't fixture or test.
 */
int test_suite_eval_nothing_special()
{
    int r;

    r = chili_suite_eval(_handle, "something_else");

    return r == 0;
}

/* Verifies that error is returned when adding
 * more than max tests.
 */
int test_suite_eval_too_much()
{
    char buf[100];
    int r;

    for (int i = 0; i < _max_count; i++){
        sprintf(buf, "test_%d", i);
        r = chili_suite_eval(_handle, buf);
        if (r <= 0){
            printf("Couldn't add expected num tests\n");
            return 0;
        }
    }

    r = chili_suite_eval(_handle, "test_shouldnt_fit");
    return r < 0;
}

