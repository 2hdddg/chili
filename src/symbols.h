#pragma once

/* Begins parsing of symbols in shared library.
 * Library file remains open until end is called.
 * All symbol strings remains valid until end is
 * called.
 *
 * path - path to shared library
 * count - set to max number of symbols in
 *         library
 *
 * Returns neg on error, pos on success.
*/
int chili_sym_begin(const char *path, int *count);

/* Retrieves next symbol in shared library.
 * chili_sym_begin must have been called prior
 * to this function and chili_sym_end must not
 * have been called.
 *
 * name - set to name of retrieved symbol
 *
 * Returns neg on error, 0 on end of symbols
 * and pos on success.
*/
int chili_sym_next(char **name);

/* Frees allocated resources.
 * All string pointers retrieved through
 * chili_sym_begin are invalid after this
 * call.
*/
void chili_sym_end();
