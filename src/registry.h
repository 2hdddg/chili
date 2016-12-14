#pragma once

#include "handle.h"

/**
 * @brief Creates a registry.
 *
 * A registry is a set of handles that can be retrieved by key.
 *
 * @param capacity Initial size of registry. Registry increases
 *                 size on demand when more is added.
 * @param handle   Instance handle set on success.
 *
 * @return Negative on error.
 *         Positive on success.
*/
int chili_reg_create(int initial_size, chili_handle *handle);

/**
 * @brief Adds a handle to the registry.
 *
 * @param handle        Registry handle.
 * @param key           Key used to access this handle.
 * @param client_handle Handle to put in registry, associated with key.
 *
 * @return Negative on error.
 *         Positive on success.
 */
int chili_reg_add(chili_handle handle,
                  const char *name,
                  chili_handle client_handle);

/**
 * @brief Iterates over all handles in registry.
 *
 * @param handle Registry handle.
 * @param token  Token used to keep position in iteration.
 *               Should be set to 0 on first call and ignored
 *               by caller after that.
 *
 * @return Handle that has been put in registry or NULL when no more
 *         handles in the registry.
 */
chili_handle chili_reg_next(chili_handle handle, int *token);

/**
 * @brief Looks up and returns a handle by the associated key.
 *
 * @param handle Registry handle.
 * @param key    Key associated with the requested handle.
 *
 * @return Handle or NULL if not found.
 */
chili_handle chili_reg_find(chili_handle handle,
                            const char *key);

/**
 * @brief Frees all resources for this registry.
 *
 * @param handle Registry handle.
 */
void chili_reg_destroy(chili_handle handle);

