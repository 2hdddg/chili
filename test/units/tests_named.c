#include <stdio.h>
#include <string.h>

#include "named.h"
#include "assert.h"


char *_lib_path;
char *_tst_name;

int each_before()
{
    _lib_path = NULL;
    _tst_name = NULL;

    return 0;
}

/* Verifies simplest possible correct line,
 * no spaces, only chars a-z and separator.
 */
int test_named_parse_good_line()
{
    char line[] = "library:test";

    int ret = chili_named_parse(line, &_lib_path, &_tst_name);

    return assert_ret_success(ret) &&
           assert_str("library", _lib_path) &&
           assert_str("test", _tst_name);
}

/* Verifies that whitespace in beginning of line is ignored.
 */
int test_named_parse_space_before_lib()
{
    char line[] = " \tlibrary:test";

    int ret = chili_named_parse(line, &_lib_path, &_tst_name);

    return assert_ret_success(ret) &&
           assert_str("library", _lib_path) &&
           assert_str("test", _tst_name);
}

/* Verifies that whitespace after library path is ignored.
 */
int test_named_parse_space_after_lib()
{
    char line[] = "library\t :test";

    int ret = chili_named_parse(line, &_lib_path, &_tst_name);

    return assert_ret_success(ret) &&
           assert_str("library", _lib_path) &&
           assert_str("test", _tst_name);
}

/* Verifies that whitespace after separator but before name
 * of test is ignored.
 */
int test_named_parse_space_before_test()
{
    char line[] = "library: \ttest";

    int ret = chili_named_parse(line, &_lib_path, &_tst_name);

    return assert_ret_success(ret) &&
           assert_str("library", _lib_path) &&
           assert_str("test", _tst_name);
}

/* Verifies that whitespace after name of test (end of line)
 * is ignored.
 */
int test_named_parse_space_after_test()
{
    char line[] = "library:test\t ";

    int ret = chili_named_parse(line, &_lib_path, &_tst_name);

    return assert_ret_success(ret) &&
           assert_str("library", _lib_path) &&
           assert_str("test", _tst_name);
}

/* Verifies return value when line has no separator.
 */
int test_named_parse_no_separator()
{
    char line[] = "no separator";

    int ret = chili_named_parse(line, &_lib_path, &_tst_name);

    return assert_ret_fail(ret);
}

/* Verifies return value when line starts with separator.
 * No library path specified.
 */
int test_named_parse_starts_with_separator()
{
    char line[] = ":test";

    int ret = chili_named_parse(line, &_lib_path, &_tst_name);

    return assert_ret_fail(ret);
}

/* Verifies return value when line ends with separator.
 * No test name specified.
 */
int test_named_parse_ends_with_separator()
{
    char line[] = "lib:";

    int ret = chili_named_parse(line, &_lib_path, &_tst_name);

    return assert_ret_fail(ret);
}

