#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "symbols.h"
#include "suite.h"
#include "run.h"
#include "bind.h"
#include "library.h"

struct instance {
    const char *path;
    chili_handle sym_handle;
    chili_handle suite_handle;
    chili_handle bind_handle;
    const struct chili_suite *suite;
    struct chili_bind_fixture fixture;
};

static int _build_suite(chili_handle sym_handle,
                        chili_handle suite)
{
    int r;
    char *name;

    /* Evaluate symbols to find tests and setup */
    do {
        r = chili_sym_next(sym_handle, &name);
        if (r < 0){
            return r;
        }
        else if (r > 0){
            r = chili_suite_eval(suite, name);
            if (r < 0){
                return r;
            }
        }
        else{
            break;
        }
    } while (true);

    return 1;
}

int chili_lib_create(const char *path,
                     chili_handle *handle)
{
    int r;
    int symbol_count;
    struct instance *instance;

    instance = malloc(sizeof(*instance));
    if (instance == NULL){
        printf("Unable to allocate lib instance\n");
        return -1;
    }

    instance->path = path;

    /* Create symbol parser */
    r = chili_sym_create(path, &symbol_count,
                         &instance->sym_handle);
    if (r < 0){
        goto on_sym_error;
    }

    /* Create suite */
    r = chili_suite_create(symbol_count, &instance->suite_handle);
    if (r < 0){
        goto on_suite_error;
    }

    /* Build the suite */
    r = _build_suite(instance->sym_handle, instance->suite_handle);
    if (r < 0){
        goto on_build_error;
    }

    /* Create binder used to bind functions in suite */
    r = chili_suite_get(instance->suite_handle, &instance->suite);
    if (r < 0){
        goto on_build_error;
    }
    r = chili_bind_create(path, instance->suite,
                          &instance->bind_handle);
    if (r < 0){
        goto on_bind_error;
    }

    chili_bind_fixture(instance->bind_handle,
                       &instance->fixture);

    *handle = instance;
    return 1;

on_bind_error:
on_build_error:
    chili_suite_destroy(instance->suite_handle);
on_suite_error:
    chili_sym_destroy(instance->sym_handle);
on_sym_error:
    free(instance);
    return r;
}

int chili_lib_before_fixture(chili_handle handle)
{
    struct instance *instance = (struct instance*)handle;

    return chili_run_before(&instance->fixture);
}

int chili_lib_next_test(chili_handle handle,
                        int *pindex,
                        struct chili_times *times,
                        struct chili_result *result,
                        struct chili_aggregated *aggregated)
{
    struct instance *instance = (struct instance*)handle;
    struct chili_bind_test test;
    int r;
    int index = *pindex;

    if (index >= instance->suite->count){
        /* No more tests */
        return 0;
    }

    r = chili_bind_test(instance->bind_handle, index, &test);
    if (r <= 0){
        /* Fatal error, can not continue or last test
         * has already executed, no more (0)  */
        return r;
    }

    r = chili_run_test(result, aggregated, &test,
                       &instance->fixture,
                       times, NULL);
    if (r < 0){
        /* Fatal error, can not continue */
        return r;
    }

    *pindex = index + 1;

    return 1;
}

int chili_lib_named_test(chili_handle handle, const char *name)
{
    return 0;
}

int chili_lib_print_tests(chili_handle handle)
{
    struct instance *instance = (struct instance*)handle;

    for (int i = 0; i < instance->suite->count; i++){
        printf("%s:%s\n", instance->path, instance->suite->tests[i]);
    }
    return 1;
}

int chili_lib_after_fixture(chili_handle handle)
{
    struct instance *instance = (struct instance*)handle;
    return chili_run_after(&instance->fixture);
}

void chili_lib_destroy(chili_handle handle)
{
    struct instance *instance = (struct instance*)handle;

    chili_bind_destroy(instance->bind_handle);
    chili_suite_destroy(instance->suite_handle);
    chili_sym_destroy(instance->sym_handle);
}
