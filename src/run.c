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
                    struct chili_suite *suite,
                    bool *before_failed)
{
    int r = 0;
    *before_failed = false;

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
            *before_failed = true;
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

int chili_run_next(struct chili_result *result,
                   struct chili_aggregated *aggregated,
                   chili_test_begin test_begin)
{
    func test;

    /* End of tests */
    if (_next >= _suite->count){
        return 0;
    }

    result->before = result->after = result->test = 0;
    result->name = _suite->tests[_next];

    debug_print("Preparing to run %s\n", result->name);

    /* Call hook that test begins even before fixtures
     * to give early feedback */
    if (test_begin){
        test_begin(result->name);
    }

    /* Prepare for next so we don't miss it */
    _next++;

    result->before = result->test = result->after = 0;

    /* Any prints below this point might be redirected */
    chili_redirect_start(result->name);

    /* Do not run test if before fixture fails */
    if (_each_before){
        result->before = _each_before();
        if (result->before < 0){
            goto exit;
        }
    }

    test = _get_func(result->name);
    if (test == NULL){
        goto exit;
    }
    result->test = test();

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
                result->name, result->before, result->test, result->after);

    if (result->before < 0 ||
        result->test < 0 ||
        result->after < 0){
        aggregated->num_errors++;
        aggregated->num_total++;
        return -1;
    }
    else if (result->test > 0){
        aggregated->num_succeeded++;
        aggregated->num_total++;
        return 1;
    }
    else{
        aggregated->num_failed++;
        aggregated->num_total++;
        return 1;
    }
}

int chili_run_end(bool *after_failed)
{
    int r = 1;
    *after_failed = false;

    if (_suite->once_after){
        r = _invoke_func(_suite->once_after);
        *after_failed = r < 0;
    }

    if (_libhandle){
        dlclose(_libhandle);
        _libhandle = NULL;
    }

    return r;
}
