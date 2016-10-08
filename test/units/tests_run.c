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

static int _failing_fixture()
{
    _called_failing_fixture++;
    return -666;
}

static int _succeeding_fixture()
{
    _called_succeeding_fixture++;
    return 1;
}

static int _timestamped_fixture()
{
    clock_gettime(CLOCK_MONOTONIC_RAW, _fixture_time);
    return 1;
}

static int _timestamped_test()
{
    clock_gettime(CLOCK_MONOTONIC_RAW, _test_time);
    return 1;
}

static int _succeeding_test()
{
    return 1;
}

static int _failing_test()
{
    return 0;
}

static int _errounous_test()
{
    return -1;
}

static void _timestamped_progress(const char *name)
{
    clock_gettime(CLOCK_MONOTONIC_RAW, _progress_time);
}

static void _progress(const char *name)
{
}

static bool _less_than(struct timespec *a, struct timespec *b)
{
    if (a->tv_sec < b->tv_sec){
        return true;
    }

    if (a->tv_sec == b->tv_sec){
        return a->tv_nsec < b->tv_nsec;
    }

    return false;
}

static const char* _execution_str(enum execution_result e)
{
    switch (e){
    case execution_not_started:
        return "execution_not_started";
    case execution_unknown_error:
        return "execution_unknown_error";
    case execution_crashed:
        return "execution_crashed";
    case execution_timed_out:
        return "execution_timed_out";
    case execution_done:
        return "execution_done";
    default:
        return "unknown";
    }
}

static const char* _test_str(enum test_result r)
{
    switch (r){
    case test_uncertain:
        return "test_uncertain";
    case test_error:
        return "test_error";
    case test_failure:
        return "test_failure";
    case test_success:
        return "test_success";
    default:
        return "unknown";
    }
}

static const char* _fixture_str(enum fixture_result f)
{
    switch (f){
    case fixture_uncertain:
        return "fixture_uncertain";
    case fixture_not_needed:
        return "fixture_not_needed";
    case fixture_error:
        return "fixture_error";
    case fixture_success:
        return "fixture_success";
    default:
        return "unknown";
    }
}

static void _print_result(const struct chili_result *r)
{
    printf("chili_result:\n"
           "\texecution: %s\n"
           "\tbefore: %s\n"
           "\ttest: %s\n"
           "\tafter: %s\n",
           _execution_str(r->execution), _fixture_str(r->before),
           _test_str(r->test), _fixture_str(r->after));
}

static void _print_aggregated(const struct chili_aggregated *a)
{
    printf("chili_aggregated:\n"
           "\tnum_succeeded: %d\n"
           "\tnum_failed: %d\n"
           "\tnum_errors: %d\n"
           "\tnum_total: %d\n",
           a->num_succeeded, a->num_failed, a->num_errors, a->num_total);
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

/* Verifies result when before fixture
 * returns error.
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
           _result.after == fixture_uncertain &&
           _result.execution == execution_done;
}

/* Verifies aggregated when before
 * fixture returns error.
 */
int test_run_next_aggregated_before_fails()
{
    chili_run_begin(&_fixture, &_before_failed);
    _fixture.each_before = _failing_fixture;
    _test.func = _succeeding_test;

    chili_run_next(&_result, &_aggregated, &_test, _timestamped_progress);
    chili_run_next(&_result, &_aggregated, &_test, _timestamped_progress);

    chili_run_end(&_after_failed);
    _print_aggregated(&_aggregated);
    return _aggregated.num_errors == 2 &&
           _aggregated.num_succeeded == 0 &&
           _aggregated.num_failed == 0 &&
           _aggregated.num_total == 2;
}

/* Verifies result when after fixture
 * returns error
 */
int test_run_next_result_after_fails()
{
    chili_run_begin(&_fixture, &_before_failed);
    _fixture.each_after = _failing_fixture;
    _test.func = _succeeding_test;

    chili_run_next(&_result, &_aggregated, &_test, _timestamped_progress);

    chili_run_end(&_after_failed);
    _print_aggregated(&_aggregated);
    return _result.before == fixture_not_needed &&
           _result.test == test_success &&
           _result.after == fixture_error &&
           _result.execution == execution_done;
}

/* Verifies aggregated when after
 * fixture returns error
 */
int test_run_next_aggregated_after_fails()
{
    chili_run_begin(&_fixture, &_before_failed);
    _fixture.each_after = _failing_fixture;
    _test.func = _succeeding_test;

    chili_run_next(&_result, &_aggregated, &_test, _timestamped_progress);
    _print_result(&_result);
    chili_run_next(&_result, &_aggregated, &_test, _timestamped_progress);
    _print_result(&_result);

    chili_run_end(&_after_failed);
    _print_aggregated(&_aggregated);
    return _aggregated.num_errors == 2 &&
           _aggregated.num_succeeded == 0 &&
           _aggregated.num_failed == 0 &&
           _aggregated.num_total == 2;
}

/* Verifies result when there is a before and
 * after fixture and everything runs fine.
 */
int test_run_next_result_after_all_successes()
{
    chili_run_begin(&_fixture, &_before_failed);
    _fixture.each_before = _succeeding_fixture;
    _fixture.each_after = _succeeding_fixture;
    _test.func = _succeeding_test;

    chili_run_next(&_result, &_aggregated, &_test, _timestamped_progress);
    _print_result(&_result);

    chili_run_end(&_after_failed);
    return _result.before == fixture_success &&
           _result.test == test_success &&
           _result.after == fixture_success &&
           _result.execution == execution_done;
}

/* Verifies result when no fixture
 * but a successful test.
 */
int test_run_next_result_success()
{
    chili_run_begin(&_fixture, &_before_failed);
    _test.func = _succeeding_test;

    chili_run_next(&_result, &_aggregated, &_test, _progress);
    _print_result(&_result);

    chili_run_end(&_after_failed);
    _print_result(&_result);
    return _result.before == fixture_not_needed &&
           _result.test == test_success &&
           _result.after == fixture_not_needed &&
           _result.execution == execution_done;
}

/* Verifies result when no fixture
 * but a failing test.
 */
int test_run_next_result_failure()
{
    chili_run_begin(&_fixture, &_before_failed);
    _test.func = _failing_test;

    chili_run_next(&_result, &_aggregated, &_test, _progress);

    chili_run_end(&_after_failed);
    _print_result(&_result);
    return _result.before == fixture_not_needed &&
           _result.test == test_failure &&
           _result.after == fixture_not_needed &&
           _result.execution == execution_done;
}

/* Verifies aggregated when a bunch of
 * succesful, a bunch of failing and a
 * bunch of tests with errors are executed.
 */
int test_run_next_aggregated_misc()
{
    chili_run_begin(&_fixture, &_before_failed);

    _test.func = _succeeding_test;
    chili_run_next(&_result, &_aggregated, &_test, _progress);
    chili_run_next(&_result, &_aggregated, &_test, _progress);
    _test.func = _failing_test;
    chili_run_next(&_result, &_aggregated, &_test, _progress);
    chili_run_next(&_result, &_aggregated, &_test, _progress);
    chili_run_next(&_result, &_aggregated, &_test, _progress);
    _print_result(&_result);
    _test.func = _errounous_test;
    chili_run_next(&_result, &_aggregated, &_test, _progress);

    chili_run_end(&_after_failed);
    _print_aggregated(&_aggregated);
    return _aggregated.num_errors == 1 &&
           _aggregated.num_succeeded == 2 &&
           _aggregated.num_failed == 3 &&
           _aggregated.num_total == 6;
}
