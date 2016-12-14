#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "report.h"
#include "library.h"
#include "command.h"
#include "registry.h"
#include "named.h"

/* Debugging */
#define DEBUG 0
#include "debug.h"


static bool _continue_testing(struct chili_result *result,
                              struct chili_aggregated *aggregated)
{
    bool error_occured = result->before == fixture_error ||
                         result->after == fixture_error ||
                         result->test == test_error ||
                         result->execution == execution_unknown_error;

    return error_occured ? false : true;
}

static int _run_suite(chili_handle lib_handle,
                      const struct chili_test_options *options,
                      struct chili_aggregated *aggregated)
{
    int r;
    struct chili_result result;
    struct chili_times times;
    int index = 0;

    times.timeout.tv_nsec = 0;
    times.timeout.tv_sec = 10;

    r = chili_lib_before_fixture(lib_handle);
    if (r < 0){
        /* Tests arent safe to run when
         * initialization failed */
        chili_report_suite_begin_fail(r);
        chili_report_end(aggregated);
        chili_redirect_end();
        return r;
    }

    do {
        /*   0 if there were no more tests,
         * > 0 if setup, test and teardown ran without error,
         * < 0 if any of above encountered an error */
        r = chili_lib_next_test(lib_handle,
                                &index,
                                &times,
                                &result,
                                aggregated);
        if (r == 0){
            break;
        }
        chili_report_test(&result, aggregated);

        if (!_continue_testing(&result, aggregated)){
            break;
        }
    } while (true); /* Stop executing on error */

    /* Preserve error from above */
    if (r < 0){
        chili_lib_after_fixture(lib_handle);
    }
    else {
        r = chili_lib_after_fixture(lib_handle);
        if (r < 0){
            chili_report_suite_end_fail(r);
        }
    }

    return r;
}

static const char *_bool_str(bool b)
{
    return b ? "true" : "false";
}

static void _option_print(const char *intro,
                          const struct chili_test_options *options)
{
    debug_print("%s\n"
                "\tuse_color: %s\n"
                "\tuse_cursor: %s\n"
                "\tuse_redirect: %s\n"
                "\tnice_stats: %s\n"
                "\tredirect_path: %s\n",
                intro,
                _bool_str(options->use_color),
                _bool_str(options->use_cursor),
                _bool_str(options->use_redirect),
                _bool_str(options->nice_stats),
                options->redirect_path);
}

static void _aggregated_print(const char *intro,
                              const struct chili_aggregated *agg)
{
    debug_print("%s\n"
                "\tnum_succeeded: %d\n"
                "\tnum_failed: %d\n"
                "\tnum_errors: %d\n",
                intro,
                agg->num_succeeded,
                agg->num_failed,
                agg->num_errors);
}

static int _ensure_library(chili_handle registry,
                           const char *library_path,
                           chili_handle *lib_handle)
{
    int r;
    chili_handle lib = chili_reg_find(registry, library_path);

    if (lib == NULL){
        debug_print("Library %s not loaded, loading it\n",
                    library_path);
        r = chili_lib_create(library_path,
                             chili_report_test_begin,
                             &lib);
        if (r < 0){
            printf("Failed to load library: %s\n",
                   library_path);
            return r;
        }
        debug_print("Succesfully loaded library %s\n",
                    library_path);

        r = chili_lib_before_fixture(lib);
        if (r < 0){
            printf("Fixture failed for library: %s\n",
                   library_path);
            chili_lib_destroy(lib);
            return r;
        }

        r = chili_reg_add(registry, library_path, lib);
        if (r < 0){
            printf("Failed to register library: %s\n",
                   library_path);
            chili_lib_after_fixture(lib);
            chili_lib_destroy(lib);
            return r;
        }
    }

    *lib_handle = lib;
    return 1;
}

static int _invoke_named_test(struct chili_aggregated *aggregated,
                              chili_handle registry,
                              const char *library_path,
                              const char *test_name,
                              struct chili_times *times)
{
    int r;
    chili_handle lib_handle;
    struct chili_result result;

    debug_print("Running named test: %s:%s\n",
                library_path, test_name);

    r = _ensure_library(registry, library_path,
                        &lib_handle);
    if (r < 0){
        aggregated->num_errors++;
        return r;
    }

    r = chili_lib_named_test(lib_handle,
                             test_name,
                             times,
                             &result,
                             aggregated);
    debug_print("Executed named test: %s:%s returned: %d\n",
                library_path, test_name, r);
    if (r < 0){
        printf("Fatal error while running test %s:%s.\n",
               library_path, test_name);
        return r;
    }

    chili_report_test(&result, aggregated);

    return r;
}

int _close_libraries(chili_handle registry)
{
    int token = 0;
    int r = 0;
    chili_handle lib_handle;

    lib_handle = chili_reg_next(registry, &token);
    while (lib_handle != NULL) {
        /* Preserve error */
        if (r < 0){
            chili_lib_after_fixture(lib_handle);
        }
        else {
            r = chili_lib_after_fixture(lib_handle);
            if (r < 0){
                chili_report_suite_end_fail(r);
            }
        }

        lib_handle = chili_reg_next(registry, &token);
    }
    return r;
}

int chili_command_all(const char **library_paths,
                      int num_libraries,
                      const struct chili_test_options *options)
{
    int r;
    struct chili_report report;
    struct chili_aggregated aggregated = { 0 };
    chili_handle lib_handle;

    _option_print("Running 'all' command with options:", options);

    report.use_color = options->use_color;
    report.use_cursor = options->use_cursor;
    report.nice_stats = options->nice_stats;

    r = chili_redirect_begin(options->use_redirect,
                             options->redirect_path);
    if (r < 0){
        return r;
    }

    r = chili_report_begin(&report);
    if (r < 0){
        chili_redirect_end();
        return r;
    }

    for (int i = 0; i < num_libraries; i++){
        r = chili_lib_create(library_paths[i],
                             chili_report_test_begin,
                             &lib_handle);
        if (r < 0){
            goto on_exit;
        }

        r = _run_suite(lib_handle, options, &aggregated);
        chili_lib_destroy(lib_handle);

        /* Errors triumphs */
        if (r < 0){
            goto on_exit;
        }
    }

    _aggregated_print("'All' command ended:\n", &aggregated);

    r = aggregated.num_failed > 0 ? 0 : 1;

on_exit:
    chili_report_end(&aggregated);
    chili_redirect_end();

    return r;
}

int chili_command_named(const char *names_path,
                        const struct chili_test_options *options)
{
    int r;
    struct chili_report report;
    struct chili_aggregated aggregated = { 0 };
    char buffer[1024];
    char *line;
    FILE *f;
    char *library_path;
    char *test_name;
    chili_handle registry;
    struct chili_times times;

    times.timeout.tv_nsec = 0;
    times.timeout.tv_sec = 10;

    /* When no input file specified, use stdin */
    f = names_path == NULL ?
        stdin :
        fopen(names_path, "r");
    if (f == NULL){
        printf("Failed to open file %s: %s\n",
               names_path, strerror(errno));
        return -1;
    }

    r = chili_reg_create(10, &registry);
    if (r < 0){
        return r;
    }

    _option_print("Running 'named' command with options:", options);

    report.use_color = options->use_color;
    report.use_cursor = options->use_cursor;
    report.nice_stats = options->nice_stats;

    r = chili_redirect_begin(options->use_redirect,
                             options->redirect_path);
    if (r < 0){
        return r;
    }

    r = chili_report_begin(&report);
    if (r < 0){
        chili_redirect_end();
        return r;
    }

    while (true) {
        line = fgets(buffer, 1024, f);
        if (line != NULL){
            r = chili_named_parse(line, &library_path, &test_name);
            if (r > 0){
                r = _invoke_named_test(&aggregated, registry,
                                       library_path, test_name,
                                       &times);
                if (r < 0){
                    break;
                }
            }
            else{
                debug_print("Failed to parse: %s\n", line);
            }
        }
        else {
            debug_print("End of file\n");
            break;
        }
    }
    _aggregated_print("'Named' command ended:\n", &aggregated);

    /* Preserve error on failure */
    if (r < 0){
        _close_libraries(registry);
    }
    else {
        r = _close_libraries(registry);
    }

    /* Preserve error on failure */
    if (r >= 0) {
        r = aggregated.num_failed > 0 ? 0 : 1;
    }

    chili_report_end(&aggregated);
    chili_redirect_end();
    chili_reg_destroy(registry);

    return r;
}

int chili_command_list(const char **library_paths,
                       int num_libraries)
{
    int r = 0;
    chili_handle lib_handle;

    for (int i = 0; i < num_libraries; i++){
        r = chili_lib_create(library_paths[i], NULL, &lib_handle);
        if (r < 0){
            return r;
        }
        r = chili_lib_print_tests(lib_handle);
        chili_lib_destroy(lib_handle);
    }

    return r;
}
