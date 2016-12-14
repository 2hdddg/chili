#pragma once

#include "handle.h"


/**
 * @brief Creates a symbol parser for specified
 *        shared library.
 *
 * All symbol strings remains valid until parser
 * is destroyed.
 *
 * @param path     Path to shared library
 * @param count    Set to max number of symbols in
 *                 library
 * @param handle   Instance handle set on success.
 *
 * @return Negative on error.
 *         Positive on success.
 */
int chili_sym_create(const char *path, int *count, chili_handle *handle);

/**
 * @brief Retrieves next symbol in shared library.
 *
 * @param handle Valid module handle.
 * @param name   Set to name of retrieved symbol
 *
 * @return Negative on error.
 *         Zero on end of symbols
 *         Positive on success.
 */
int chili_sym_next(chili_handle handle, char **name);

/**
 * @brief Frees allocated resources.
 *
 * All symbol strings received on this
 * instance are invalid after this call.
 *
 * @param handle Valid module handle.
 */
void chili_sym_destroy(chili_handle handle);
