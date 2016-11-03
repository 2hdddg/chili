#pragma once

#include "handle.h"
#include "suite.h"

typedef int (*chili_func)(void);

struct chili_bind_fixture
{
    /* Function to call once, before
     * any test */
    chili_func once_before;
    /* Function to call once, after
     * all tests been executed. */
    chili_func once_after;
    /* Function to call before a test
     * function is executed */
    chili_func each_before;
    /* Function to call after a test
     * has executed. */
    chili_func each_after;
};

struct chili_bind_test
{
    /* Test function */
    chili_func func;
    /* Name of test */
    const char *name;
    /* Name and path to library
     * containing test */
    const char *library;
};


/**
 * @brief Creates symbol binder for a library.
 *
 * Reference to suite is held on to until destroy is called.
 *
 * @return Negative on error, positive on success.
 */
int chili_bind_create(const char *path,
                      const struct chili_suite *suite,
                      chili_handle *handle);

/**
 * @brief Returns bound fixture.
 *
 * @return Negative on error, positive on success.
 */
int chili_bind_fixture(chili_handle handle,
                       struct chili_bind_fixture *fixture);

/**
 * @brief Binds next test in suite.
 *
 * @return Negative on error, positive on success.
 */
int chili_bind_test(chili_handle handle,
                    int index,
                    struct chili_bind_test *bind_test);

/**
 * @brief Releases all resources held by the module.
 *
 */
void chili_bind_destroy(chili_handle handle);

