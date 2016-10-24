#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "bind.h"

struct instance {
    void *lib_handle;
    const struct chili_suite *suite;
};

static chili_func _get_func(void *lib_handle, const char *name)
{
    chili_func f;

    *(void **)(&f) = dlsym(lib_handle, name);
    if (f == NULL){
        printf("Unable to dlsym %s\n", name);
    }
    return f;
}

static int _bind_func(void *lib_handle, const char *name,
                      chili_func *func)
{
    if (name){
        *func = _get_func(lib_handle, name);
        return *func == NULL ? -1 : 1;
    }
    else{
        *func = NULL;
        return 1;
    }
}

int chili_bind_create(const char *path,
                      const struct chili_suite *suite,
                      chili_handle *handle)
{
    int r;
    void *lib_handle = NULL;
    struct instance *instance = NULL;

    lib_handle = dlopen(path, RTLD_LAZY);
    if (lib_handle == NULL){
        printf("Failed to load library %s due to %s\n",
               path, dlerror());
        return -1;
    }

    instance = malloc(sizeof(*instance));
    if (instance == NULL){
        r = -1;
        printf("Failed to allocate instance\n");
        goto onerror;
    }

    instance->lib_handle = lib_handle;


    instance->suite = suite;
    *handle = instance;

    return 1;

onerror:
    dlclose(lib_handle);
    if (instance != NULL){
        free(instance);
    }
    return r;
}

int chili_bind_fixture(chili_handle handle,
                       struct chili_bind_fixture *fixture)
{
    struct instance *instance = (struct instance*)handle;
    void* lib_handle = instance->lib_handle;
    const struct chili_suite *suite = instance->suite;
    int r;

    r = _bind_func(lib_handle, suite->once_before,
                   &fixture->once_before);
    if (r < 0){
        return r;
    }
    r = _bind_func(lib_handle, suite->once_after,
                   &fixture->once_after);
    if (r < 0){
        return r;
    }
    r = _bind_func(lib_handle, suite->each_before,
                   &fixture->each_before);
    if (r < 0){
        return r;
    }
    r = _bind_func(lib_handle, suite->each_after,
                   &fixture->each_after);
    if (r < 0){
        return r;
    }

    return r;
}

int chili_bind_test(chili_handle handle,
                    int index,
                    struct chili_bind_test *bind_test)
{
    struct instance *instance = (struct instance*)handle;
    const char *name;
    int r;
    const struct chili_suite *suite = instance->suite;

    if (index >= suite->count || index < 0){
        /* Invalid index */
        return -1;
    }

    name = suite->tests[index];
    if (name == NULL){
        return -1;
    }
    r = _bind_func(instance->lib_handle, name, &bind_test->func);
    if (r < 0){
        return -1;
    }
    bind_test->name = name;

    return 1;
}

void chili_bind_destroy(chili_handle handle)
{
    struct instance *instance = (struct instance*)handle;

    dlclose(instance->lib_handle);
    free(instance);
}

