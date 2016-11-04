#include <stdio.h>
#include <string.h>

#include "command.h"
#include "stub_command.h"


/* Linking to real main. No header file for that. */
extern int main(int argc, char *argv[]);

char _library_path[100];
char _library_path2[100];
struct chili_test_options _options;

static int _copy_test_params(const char **library_paths,
                             int num_library_paths,
                             const struct chili_test_options *options)
{
    if (num_library_paths > 0){
        strncpy(_library_path, library_paths[0], sizeof(_library_path));
    }
    if (num_library_paths > 1){
        strncpy(_library_path2, library_paths[1], sizeof(_library_path2));
    }
    if (num_library_paths > 2){
        printf("Too many paths\n");
        return -1;
    }
    _options = *options;
    return 0;
}

static int _copy_list_params(const char **library_paths,
                             int num_library_paths)
{
    if (num_library_paths > 0){
        strncpy(_library_path, library_paths[0], sizeof(_library_path));
    }
    if (num_library_paths > 1){
        strncpy(_library_path2, library_paths[1], sizeof(_library_path2));
    }
    if (num_library_paths > 2){
        printf("Too many paths\n");
        return -1;
    }
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
    memset(_library_path, 0, sizeof(_library_path));
    memset(&_options, 0, sizeof(_options));
    stub_command_test = _copy_test_params;
    stub_command_list = _copy_list_params;

    return 1;
}

/* Verifies that suite path is parsed correcly when no
 * other parameters specified.
 */
int test_test_only_suite()
{
    char *argv[] = {"executable", "test", "a.so" };
    int argc = sizeof(argv) / sizeof(char*);

    main(argc, argv);

    if (strncmp(_library_path, argv[2], sizeof(_library_path)) != 0){
        printf("Path to suite is wrong.\n");
        return 0;
    }

    return 1;
}

/* Verifies that more than one suite can be specified
 * to the test command
 */
int test_test_several_suites()
{
    char *argv[] = {"executable", "test", "a.so", "b.so" };
    int argc = sizeof(argv) / sizeof(char*);

    main(argc, argv);

    if (strncmp(_library_path, argv[2], sizeof(_library_path)) != 0){
        printf("Path to first suite is wrong.\n");
        return 0;
    }
    if (strncmp(_library_path2, argv[3], sizeof(_library_path2)) != 0){
        printf("Path to second suite is wrong.\n");
        return 0;
    }

    return 1;
}

/* Verifies option defaults when using minimal parameters to
 * test command.
 */
int test_test_options_defaults()
{
    char *argv[] = {"executable", "test", "a.so" };
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
int test_test_options_interactive()
{
    char *argv[] = {"executable", "test", "-i", "a.so" };
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

/* Verifies that suite path is parsed correcly when no
 * other parameters specified.
 */
int test_list_only_suite()
{
    char *argv[] = {"executable", "list", "a.so" };
    int argc = sizeof(argv) / sizeof(char*);

    main(argc, argv);

    if (strncmp(_library_path, argv[2], sizeof(_library_path)) != 0){
        printf("Path to suite is wrong.\n");
        return 0;
    }

    return 1;
}

/* Verifies that more than one suite can be specified
 * to the list command
 */
int test_list_several_suites()
{
    char *argv[] = {"executable", "list", "a.so", "b.so" };
    int argc = sizeof(argv) / sizeof(char*);

    main(argc, argv);

    if (strncmp(_library_path, argv[2], sizeof(_library_path)) != 0){
        printf("Path to first suite is wrong.\n");
        return 0;
    }
    if (strncmp(_library_path2, argv[3], sizeof(_library_path2)) != 0){
        printf("Path to second suite is wrong.\n");
        return 0;
    }

    return 1;
}
