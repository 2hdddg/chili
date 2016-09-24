#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

#include "bind.h"


/* Globals */
static void *_libhandle = NULL;
static struct chili_bind_fixture _fixture;
static int _next = 0;
static const struct chili_suite *_suite;


static chili_func _get_func(const char *name)
{
    chili_func f;

    *(void **)(&f) = dlsym(_libhandle, name);
    if (f == NULL){
        printf("Unable to dlsym %s\n", name);
    }
    return f;
}

static int _bind_func(const char *name, chili_func *func)
{
    if (name){
        *func = _get_func(name);
        return *func == NULL ? -1 : 1;
    }
    else{
        *func = NULL;
        return 1;
    }
}

int chili_bind_begin(const char *path,
                     const struct chili_suite *suite)
{
    int r;

    _libhandle = dlopen(path, RTLD_LAZY);
    if (_libhandle == NULL){
        printf("Failed to load library %s due to %s\n",
            path, dlerror());
        return -1;
    }

    r = _bind_func(suite->once_before, &_fixture.once_before);
    if (r < 0){
        goto onerror;
    }
    r = _bind_func(suite->once_after, &_fixture.once_after);
    if (r < 0){
        goto onerror;
    }
    r = _bind_func(suite->each_before, &_fixture.each_before);
    if (r < 0){
        goto onerror;
    }
    r = _bind_func(suite->each_after, &_fixture.each_after);
    if (r < 0){
        goto onerror;
    }

    _suite = suite;

    return 1;

onerror:
    dlclose(_libhandle);
    _libhandle = NULL;
    return r;
}

const struct chili_bind_fixture* chili_bind_get_fixture()
{
    return &_fixture;
}

int chili_bind_next_test(struct chili_bind_test *bind_test)
{
    const char *name;
    int r;

    if (_next >= _suite->count){
        /* No more tests */
        return 0;
    }

    name = _suite->tests[_next];
    if (name == NULL){
        return -1;
    }
    r = _bind_func(name, &bind_test->func);
    if (r < 0){
        return -1;
    }
    bind_test->name = name;

    _next++;

    return 1;
}

int chili_bind_end()
{
    if (_libhandle){
        dlclose(_libhandle);
        _libhandle = NULL;
    }
    memset(&_suite, 0, sizeof(_suite));
    return 1;
}

