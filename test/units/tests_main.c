#include <stdio.h>
#include <string.h>

#include "command.h"
#include "stub_command.h"
#include "assert.h"


/* Linking to real main. No header file for that. */
extern int main(int argc, char *argv[]);

char *_latest_command;
char _path[100];
char _path2[100];
struct chili_test_options _options;

static int _stub_command_all(const char **library_paths,
                             int num_library_paths,
                             const struct chili_test_options *options)
{
    if (num_library_paths > 0){
        strncpy(_path, library_paths[0], sizeof(_path));
    }
    if (num_library_paths > 1){
        strncpy(_path2, library_paths[1], sizeof(_path2));
    }
    if (num_library_paths > 2){
        printf("Too many paths\n");
        return -1;
    }
    _options = *options;
    _latest_command = "all";
    return 0;
}

static int _stub_command_named(const char *names_path,
                               const struct chili_test_options *options)
{
    _latest_command = "named";

    return 0;
}

static int _stub_command_list(const char **library_paths,
                             int num_library_paths)
{
    if (num_library_paths > 0){
        strncpy(_path, library_paths[0], sizeof(_path));
    }
    if (num_library_paths > 1){
        strncpy(_path2, library_paths[1], sizeof(_path2));
    }
    if (num_library_paths > 2){
        printf("Too many paths\n");
        return -1;
    }
    _latest_command = "list";

    return 0;
}

static const char* _str_bool(bool b)
{
    return b ? "true" : "false";
}

static bool _check_options(const struct chili_test_options *a,
                           const struct chili_test_options *b)
{
    bool ok = true;
    if (a->use_color != b->use_color){
        printf("use_color should be %s\n", _str_bool(a->use_color));
        ok = false;
    }
    if (a->use_cursor != b->use_cursor){
        printf("use_cursor should be %s\n", _str_bool(a->use_cursor));
        ok = false;
    }
    if (a->use_redirect != b->use_redirect){
        printf("use_redirect should be %s\n", _str_bool(a->use_redirect));
        ok = false;
    }
    if (a->nice_stats != b->nice_stats){
        printf("nice_stats should be %s\n", _str_bool(a->nice_stats));
        ok = false;
    }
    return ok;
}

int each_before()
{
    memset(_path, 0, sizeof(_path));
    memset(&_options, 0, sizeof(_options));
    _latest_command = NULL;
    stub_command_all = _stub_command_all;
    stub_command_list = _stub_command_list;
    stub_command_named = _stub_command_named;

    return 1;
}

/* Verifies that 'all' command is invoked.
 */
int test_all_command()
{
    char *argv[] = {"executable", "all", "a.so" };
    int argc = sizeof(argv) / sizeof(char*);

    main(argc, argv);

    return assert_str("all", _latest_command);
}

/* Verifies that suite path is parsed correcly when no
 * other parameters specified.
 */
int test_all_only_suite()
{
    char *argv[] = {"executable", "all", "a.so" };
    int argc = sizeof(argv) / sizeof(char*);

    main(argc, argv);

    return assert_str_e(argv[2], _path, "Path to suite is wrong.\n");
}

/* Verifies that more than one suite can be specified
 * to the test command
 */
int test_all_several_suites()
{
    char *argv[] = {"executable", "all", "a.so", "b.so" };
    int argc = sizeof(argv) / sizeof(char*);

    main(argc, argv);

    return assert_str_e(argv[2], _path,
                        "Path to first suite is wrong.\n") &&
           assert_str_e(argv[3], _path2,
                        "Path to second suite is wrong.\n");
}

/* Verifies option defaults when using minimal parameters to
 * test command.
 */
int test_all_options_defaults()
{
    char *argv[] = {"executable", "all", "a.so" };
    int argc = sizeof(argv) / sizeof(char*);
    /* Option defaults is for non-interactive consumption */
    struct chili_test_options options = {
        .use_color = false,
        .use_cursor = false,
        .use_redirect = true,
        .nice_stats = false
    };

    main(argc, argv);

    return _check_options(&options, &_options) ? 1 : 0;
}

/* Verifies options when requesting interactive mode.
 */
int test_all_options_interactive()
{
    char *argv[] = {"executable", "all", "-i", "a.so" };
    int argc = sizeof(argv) / sizeof(char*);
    /* Option defaults is for interactive consumption */
    struct chili_test_options options = {
        .use_color = true,
        .use_cursor = true,
        .use_redirect = true,
        .nice_stats = true
    };

    main(argc, argv);

    return _check_options(&options, &_options) ? 1 : 0;
}

/* Verifies that 'list' command is invoked.
 */
int test_list_command()
{
    char *argv[] = {"executable", "list", "a.so" };
    int argc = sizeof(argv) / sizeof(char*);

    main(argc, argv);

    return assert_str("list", _latest_command);
}

/* Verifies that suite path is parsed correcly when no
 * other parameters specified.
 */
int test_list_only_suite()
{
    char *argv[] = {"executable", "list", "a.so" };
    int argc = sizeof(argv) / sizeof(char*);

    main(argc, argv);

    return assert_str_e(argv[2], _path, "Path to suite is wrong.\n");
}

/* Verifies that more than one suite can be specified
 * to the list command
 */
int test_list_several_suites()
{
    char *argv[] = {"executable", "list", "a.so", "b.so" };
    int argc = sizeof(argv) / sizeof(char*);

    main(argc, argv);

    return assert_str_e(argv[2], _path,
                        "Path to first suite is wrong.\n") &&
           assert_str_e(argv[3], _path2,
                        "Path to second suite is wrong.\n");
}

/* Verifies that 'named' command is invoked.
 */
int test_named_command()
{
    char *argv[] = {"executable", "named", "a.so" };
    int argc = sizeof(argv) / sizeof(char*);

    main(argc, argv);

    return assert_str("named", _latest_command);
}
