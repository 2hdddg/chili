#pragma once


/**
 * @brief Parses a line specifying a test in a suite library.
 *
 * Parses line into two parts, library path and name of test.
 * A line looks like:
 * path_to_library:name_of_test
 *
 * @param line         Line that specifies a test in a library.
 * @param library_path Contains the parsed library path on
 *                     success.
 * @param test_name    Contains the parsed test name on success.
 *
 * @return Negative on error, positive on success.
 */
int chili_named_parse(char *line,
                      char **library_path,
                      char **test_name);
