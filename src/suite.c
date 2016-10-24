#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "suite.h"

/* Debugging */
#define DEBUG 0
#include "debug.h"

/* Types */
struct instance {
    int max;
    struct chili_suite suite;
};

/* Constants */
const char *_once_before_name = "once_before";
const char *_once_after_name  = "once_after";
const char *_each_before_name = "each_before";
const char *_each_after_name  = "each_after";


/* List functions */
static int _add(struct instance *instance, char *symbol)
{
    struct chili_suite *suite = &instance->suite;
    char **test = suite->tests + suite->count;

    if (suite->count >= instance->max){
        printf("Tests full\n");
        return -1;
    }

    *test = symbol;
    suite->count++;

    debug_print("Found test: %s\n", symbol);

    return 1;
}


/* Eval functions */
static int _eval_fixture(char *symbol, struct chili_suite *suite)
{
    if (strcmp(symbol, _once_before_name) == 0){
        suite->once_before = _once_before_name;
        debug_print("Found fixture: once before\n");
        return 1;
    }
    if (strcmp(symbol, _once_after_name) == 0){
        suite->once_after = _once_after_name;
        debug_print("Found fixture: once after\n");
        return 1;
    }
    if (strcmp(symbol, _each_before_name) == 0){
        suite->each_before = _each_before_name;
        debug_print("Found fixture: each before\n");
        return 1;
    }
    if (strcmp(symbol, _each_after_name) == 0){
        suite->each_after = _each_after_name;
        debug_print("Found fixture: each after\n");
        return 1;
    }

    return 0;
}

static int _eval_test(const char *symbol)
{
    const char *test_ = "test_";
    const int len = 5; /* length of test_ */

    return strncmp(test_, symbol, len) == 0 ? 1 : 0;
}

/* Externals */
int chili_suite_create(int max, chili_handle *handle)
{
    int size = sizeof(char*) * max;
    struct instance *instance;

    debug_print("Creating suite with max %d entries\n", max);

    instance = malloc(sizeof(*instance));
    if (instance == NULL){
        printf("Unable to allocate instance\n");
        return -1;
    }

    instance->max = max;
    memset(&instance->suite, 0, sizeof(struct chili_suite));
    instance->suite.tests = malloc(size);

    if (instance->suite.tests == NULL){
        printf("Unable to allocate: %s\n", strerror(errno));
        free(instance);
        return -1;
    }

    *handle = instance;
    return 1;
}

int chili_suite_eval(chili_handle handle, char *symbol)
{
    struct instance *instance = (struct instance*)handle;
    int found;

    debug_print("Evaluating symbol %s\n", symbol);

    found = _eval_fixture(symbol, &instance->suite);
    if (found){
        return 1;
    }

    found = _eval_test(symbol);
    if (found){
        return _add(instance, symbol);
    }

    return 0;
}

int chili_suite_get(chili_handle handle,
                    const struct chili_suite **suite)
{
    struct instance *instance = (struct instance*)handle;

    *suite = &instance->suite;
    return 1;
}

void chili_suite_destroy(chili_handle handle)
{
    struct instance *instance = (struct instance*)handle;

    debug_print("Destroying suite\n");

    free(instance->suite.tests);
    free(instance);
}
