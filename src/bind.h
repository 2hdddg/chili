#pragma once

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
 * @brief Initializes module. Binds fixture symbols.
 *
 * Reference to suite is held on to until end is called.
 * Data retrieved from the module is kept alive until
 * end is called.
 *
 * @return Negative on error, positive on success.
 */
int chili_bind_begin(const char *path, const struct chili_suite *suite);

/**
 * @brief Returns bound fixture functions.
 * @return Bound fixture functions.
 */
const struct chili_bind_fixture* chili_bind_get_fixture();

/**
 * @brief Binds next test in suite.
 *
 * @return Negative on error, positive on success, zero if
 *         no more tests exists in suite.
 */
int chili_bind_next_test(struct chili_bind_test *bind_test);

/**
 * @brief Releases all resources held by the module.
 *
 * @return Negative on error, positive on success.
 */
int chili_bind_end();

