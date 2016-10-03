#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/mman.h>

#include "run.h"


struct chili_bind_fixture _fixture;
bool _before_failed;
bool _after_failed;
int _called_failing_fixture;
int _called_succeeding_fixture;
struct chili_result _result;
struct chili_aggregated _aggregated;
struct chili_bind_test _test;
struct timespec *_progress_time;
struct timespec *_fixture_time;
struct timespec *_test_time;


static struct timespec* _mmap_timespec()
{
    struct timespec *mapped;

    mapped = mmap(NULL, sizeof(struct timespec),
                  PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,
                  -1, 0);
    if (mapped == MAP_FAILED){
        mapped = NULL;
    }
    return mapped;
}

static void _munmap_timespec(struct timespec *t)
{
    munmap(t, sizeof(struct timespec));
}

int once_before()
{
    _progress_time = _mmap_timespec();
    _fixture_time = _mmap_timespec();
    _test_time = _mmap_timespec();

    return _progress_time != NULL
        && _fixture_time != NULL
        && _test_time != NULL ? 1 : -1;
}

int once_after()
{
    _munmap_timespec(_progress_time);
    _munmap_timespec(_fixture_time);
    _munmap_timespec(_test_time);

    return 1;
}

int before_each()
{
    memset(&_fixture, 0, sizeof(_fixture));
    memset(&_result, 0, sizeof(_result));
    memset(&_aggregated, 0, sizeof(_aggregated));
    memset(&_test, 0, sizeof(_test));
    memset(_progress_time, 0, sizeof(struct timespec));
    memset(_fixture_time, 0, sizeof(struct timespec));
    memset(_test_time, 0, sizeof(struct timespec));
    _before_failed = false;
    _after_failed = false;
    _called_failing_fixture = 0;
    return 1;
}

int _failing_fixture()
{
    _called_failing_fixture++;
    return -666;
}

int _succeeding_fixture()
{
    _called_succeeding_fixture++;
    return 1;
}

int _timestamped_fixture()
{
    clock_gettime(CLOCK_MONOTONIC_RAW, _fixture_time);
    return 1;
}

int _timestamped_test()
{
    clock_gettime(CLOCK_MONOTONIC_RAW, _test_time);
    return 1;
}

int _succeeding_test()
{
    return 1;
}

void _timestamped_progress(const char* name)
{
    clock_gettime(CLOCK_MONOTONIC_RAW, _progress_time);
}

bool _less_than(struct timespec *a, struct timespec *b)
{
    if (a->tv_sec < b->tv_sec){
        return true;
    }

    if (a->tv_sec == b->tv_sec){
        return a->tv_nsec < b->tv_nsec;
    }

    return false;
}


/* Verifies begin without any fixture.
 * Function should return zero or positive to
 * indicate success and flag should be false.
 */
int test_run_begin_empty_fixture()
{
    int ret = chili_run_begin(&_fixture, &_before_failed);
    chili_run_end(&_after_failed);

    return ret >= 0 && !_before_failed;
}

/* Verifies begin with a succeeding once_before
 * fixture.
 * Function should return zero or positive to
 * indicate success and flag should be false.
 */
int test_run_begin_fixture_once_begin_succeeds()
{
    _fixture.once_before = _succeeding_fixture;
    int ret = chili_run_begin(&_fixture, &_before_failed);
    chili_run_end(&_after_failed);

    return ret >= 0 && _called_succeeding_fixture == 1 && !_before_failed;
}

/* Verifies begin with a failing once_before
 * fixture.
 * Function should return negative to indicate
 * error in setup. Returned value should be
 * same as value returned from fixture and flag
 * should be set to true to indicate the failure.
 */
int test_run_begin_fixture_once_before_fails()
{
    _fixture.once_before = _failing_fixture;
    int ret = chili_run_begin(&_fixture, &_before_failed);

    return ret == -666 && _called_failing_fixture == 1 && _before_failed;
}

/* Verifies end without any fixture.
 * Function should return zero or positive to
 * indicate success and flag should be false.
 */
int test_run_end_empty_fixture()
{
    chili_run_begin(&_fixture, &_before_failed);
    int ret = chili_run_end(&_after_failed);

    return ret >= 0 && !_after_failed;
}

/* Verifies end with a succeeding once_after
 * fixture.
 * Function should return zero or positive to
 * indicate success and flag should be false.
 */
int test_run_end_fixture_once_after_succeeds()
{
    _fixture.once_after = _succeeding_fixture;
    chili_run_begin(&_fixture, &_before_failed);
    int ret = chili_run_end(&_after_failed);

    return ret >= 0 && _called_succeeding_fixture == 1 && !_after_failed;
}

/* Verifies end with a failing once_after
 * fixture.
 * Function should return negative to indicate
 * error in setup. Returned value should be
 * same as value returned from fixture and flag
 * should be set to true to indicate the failure.
 */
int test_run_end_fixture_once_after_fails()
{
    _fixture.once_after = _failing_fixture;
    chili_run_begin(&_fixture, &_before_failed);
    int ret = chili_run_end(&_after_failed);

    return ret == -666 && _called_failing_fixture == 1 && _after_failed;
}

/* Verifies that next calls start hook before
 * test fixture or test function
 */
int test_run_next_calls_progress_hook()
{
    chili_run_begin(&_fixture, &_before_failed);
    _fixture.each_before = _timestamped_fixture;
    _test.func = _timestamped_test;

    chili_run_next(&_result, &_aggregated, &_test, _timestamped_progress);

    chili_run_end(&_after_failed);
    return _less_than(_progress_time, _fixture_time) &&
           _less_than(_fixture_time, _test_time);
}

/* Verifies that before result is set to error
 * when each_before returns error
 */
int test_run_next_result_before_fails()
{
    chili_run_begin(&_fixture, &_before_failed);
    _fixture.each_before = _failing_fixture;
    _test.func = _succeeding_test;

    chili_run_next(&_result, &_aggregated, &_test, _timestamped_progress);

    chili_run_end(&_after_failed);
    return _result.before == fixture_error &&
           _result.test == test_uncertain &&
           _result.after == fixture_uncertain;
}

/* Verifies that aggregated num_errors is increased
 * when each_before returns error
 */
int test_run_next_aggregated_before_fails()
{
    chili_run_begin(&_fixture, &_before_failed);
    _fixture.each_before = _failing_fixture;
    _test.func = _succeeding_test;

    chili_run_next(&_result, &_aggregated, &_test, _timestamped_progress);
    chili_run_next(&_result, &_aggregated, &_test, _timestamped_progress);

    chili_run_end(&_after_failed);
    return _aggregated.num_errors == 2 &&
           _aggregated.num_succeeded == 0 &&
           _aggregated.num_failed == 0 &&
           _aggregated.num_total == 2;
}

/* Verifies that before result is set to error
 * when each_after returns error
 */
int test_run_next_result_after_fails()
{
    chili_run_begin(&_fixture, &_before_failed);
    _fixture.each_after = _failing_fixture;
    _test.func = _succeeding_test;

    chili_run_next(&_result, &_aggregated, &_test, _timestamped_progress);

    chili_run_end(&_after_failed);
    return _result.before == fixture_not_needed &&
           _result.test == test_success &&
           _result.after == fixture_error;
}

/* Verifies that aggregated num_errors is increased
 * when each_after returns error
 */
int test_run_next_aggregated_after_fails()
{
    chili_run_begin(&_fixture, &_before_failed);
    _fixture.each_after = _failing_fixture;
    _test.func = _succeeding_test;

    chili_run_next(&_result, &_aggregated, &_test, _timestamped_progress);
    chili_run_next(&_result, &_aggregated, &_test, _timestamped_progress);

    chili_run_end(&_after_failed);
    return _aggregated.num_errors == 2 &&
           _aggregated.num_succeeded == 0 &&
           _aggregated.num_failed == 0 &&
           _aggregated.num_total == 2;
}
