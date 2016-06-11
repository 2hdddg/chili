#include <stdio.h>
#include <dlfcn.h>
#include <errno.h>
#include <string.h>

#include "run.h"


/* Types */
typedef int (*func)(void);


/* Globals */
static void *_libhandle;
static struct chili_suite *_suite;
func _each_before = NULL;
func _each_after = NULL;
int _next = 0;
chili_test_begin _test_begin;


/* Invocation */
static func _get_func(const char *name)
{
    func f;
    *(void **)(&f) = dlsym(_libhandle, name);
    if (f == NULL){
        printf("Unable to dlsym %s\n", name);
    }
    return f;
}

static int _invoke_func(const char *name)
{
    func f = _get_func(name);

    if (f == NULL){
        return -1;
    }

    return f();
}

/* Exports */
int chili_run_begin(const char *path, chili_test_begin test_begin, struct chili_suite *suite)
{
    int r = 0;

    _libhandle = dlopen(path, RTLD_LAZY);
    if (_libhandle == NULL){
        printf("Failed to load library %s due to %s\n",
            path, dlerror());
        r = -1;
        goto onerror;
    }

    if (suite->once_before){
        r = _invoke_func(suite->once_before);
        if (r < 0){
            goto onerror;
        }
    }

    if (suite->each_before){
        _each_before = _get_func(suite->each_before);
        if (_each_before == NULL){
            goto onerror;
        }
    }
    else {
        _each_before = NULL;
    }

    if (suite->each_after){
        _each_after = _get_func(suite->each_after);
        if (_each_after == NULL){
            goto onerror;
        }
    }
    else {
        _each_after = NULL;
    }

    _test_begin = test_begin;
    _suite = suite;
    _next = 0;

    return 1;

onerror:
    if (_libhandle){
        dlclose(_libhandle);
        _libhandle = NULL;
    }
    return r;
}

int chili_run_next(struct chili_result *result)
{
    const char* name;
    func test;
    int executed = 0;

    /* End of tests */
    if (_next >= _suite->count){
        return 0;
    }

    name = _suite->tests[_next];

    /* Call hook that test begins even before fixtures
     * to give early feedback */
    if (_test_begin){
        _test_begin(name);
    }

    /* Prepare for next so we don't miss it */
    _next++;

    result->name = name;
    result->before = result->test = result->after = 0;

    /* Do not run test if before fixture fails */
    if (_each_before){
        result->before = _each_before();
        if (result->before < 0){
            goto exit;
        }
    }

    test = _get_func(name);
    if (test == NULL){
        goto exit;
    }
    result->test = test();
    executed = 1;

exit:
    /* Regardless of output of test, execute after fixture */
    if (_each_after){
        result->after = _each_after();
    }

    return executed ? 1 : -1;
}

int chili_run_end()
{
    int r = 0;

    if (_suite->once_after){
        r = _invoke_func(_suite->once_after);
    }

    if (_libhandle){
        dlclose(_libhandle);
        _libhandle = NULL;
    }

    return r;
}
