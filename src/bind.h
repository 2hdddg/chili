#pragma once

#include "handle.h"
#include "suite.h"

typedef int (*chili_func)(void);

struct chili_bind_fixture
{
    chili_func once_before;
    chili_func once_after;
    chili_func each_before;
    chili_func each_after;
};

struct chili_bind_test
{
    chili_func func;
    const char *name;
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
 * @brief Returns fixture
 *
 * @return void
 */
int chili_bind_fixture(chili_handle handle,
                       struct chili_bind_fixture *fixture);

/**
 * @brief Binds next test in suite.
 *
 * @return Negative on error, positive on success, zero if
 *         no more tests exists in suite.
 */
int chili_bind_test(chili_handle handle,
                    int index,
                    struct chili_bind_test *bind_test);

/**
 * @brief Releases all resources held by the module.
 *
 * @return Negative on error, positive on success.
 */
void chili_bind_destroy(chili_handle handle);

