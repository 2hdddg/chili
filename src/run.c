#include <stdio.h>
#include <dlfcn.h>
#include <errno.h>
#include <string.h>

#include "redirect.h"
#include "run.h"

/* Debugging */
#define DEBUG 0
#include "debug.h"

/* Types */
typedef int (*func)(void);


/* Globals */
static void *_libhandle;
static struct chili_suite *_suite;
static func _each_before = NULL;
static func _each_after = NULL;
static int _next = 0;
static chili_test_begin _test_begin;


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
int chili_run_begin(const char *path,
                    chili_test_begin test_begin,
                    struct chili_suite *suite)
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

    /* End of tests */
    if (_next >= _suite->count){
        return 0;
    }

    result->executed = false;
    result->before = 0;
    result->after = 0;

    name = _suite->tests[_next];

    debug_print("Preparing to run %s\n", name);

    /* Call hook that test begins even before fixtures
     * to give early feedback */
    if (_test_begin){
        _test_begin(name);
    }

    /* Prepare for next so we don't miss it */
    _next++;

    result->name = name;
    result->before = result->test = result->after = 0;

    /* Any prints below this point might be redirected */
    chili_redirect_start(name);

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
    result->executed = true;

exit:
    /* Regardless of output of test, execute after fixture */
    if (result->before >= 0 && _each_after){
        result->after = _each_after();
    }

    chili_redirect_stop();

    debug_print("After running %s\n"
                "\tbefore: %d\n"
                "\ttest: %d\n"
                "\tafter: %d\n",
                name, result->before, result->test, result->after);

    return result->before < 0 ||
           result->test < 0 ||
           result->after < 0 ? -1 : 1;
}

int chili_run_end()
{
    int r = 1;

    if (_suite->once_after){
        r = _invoke_func(_suite->once_after);
    }

    if (_libhandle){
        dlclose(_libhandle);
        _libhandle = NULL;
    }

    return r;
}
