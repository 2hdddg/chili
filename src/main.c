#include <string.h>
#include <stdio.h>
#include <getopt.h>
#include <stdbool.h>
#include <signal.h>

#include "command.h"

/* Debugging */
#define DEBUG_PRINTS 0
#include "debug.h"


static const char *_option_path =
      "  <path>...\n"
      "    Path to shared libraries that contains tests. More than\n"
      "    one shared library can be specified.\n";

static const char *_option_named_path =
      "  <path>...\n"
      "    Path to file containing names of tests to run.\n";

static const char *_option_color =
      "  -c, --color\n"
      "    Enable colored output.\n";

static const char *_option_cursor =
      "  -m, --cursor\n"
      "    Enable cursor movements. This will minimize console\n"
      "    output.\n";

static const char *_option_nice =
      "  -n, --nice\n"
      "    Nice and friendly output, but harder to parse.\n";

static const char *_option_interactive =
      "  -i, --interactive\n"
      "    Indicates human interactive use. Will enable all\n"
      "    features for improved user interaction.\n"
      "    Turns on cursor movements, colored output and\n"
      "    nice output.\n";

static void _display_usage()
{
    printf(
      "Usage: chili <command> <options> <args>...\n"
      "\n"
      "These are the valid commands:\n"
      "\n"
      "Running tests\n"
      "  all     Runs all tests in specified shared libraries\n"
      "  named   Runs all named tests\n"
      "\n"
      "Other\n"
      "  list    Lists all tests in specified shared libraries\n"
      "  help    Shows help about a specified command\n");
}

static void _display_all_usage()
{
    printf(
      "chili all [--color | -c] [--cursor | -m] [--interactive | -i]\n"
      "          <path>...\n"
      "\n"
      "DESCRIPTION\n"
      "  Runs all tests that can be found in the specified shared\n"
      "  libraries.\n"
      "\n"
      "OPTIONS\n"
      "%s\n" /* Path        */
      "%s\n" /* Color       */
      "%s\n" /* Cursor      */
      "%s\n" /* Nice        */
      "%s",  /* Interactive */
      _option_path, _option_color, _option_cursor, _option_nice,
      _option_interactive);
}

static void _display_named_usage()
{
    printf(
      "chili named [--color | -c] [--cursor | -m] [--interactive | -i]\n"
      "            [<path>]\n"
      "\n"
      "DESCRIPTION\n"
      "  Runs all named tests in the specified order.\n"
      "  Tests are named on the form:\n"
      "    <path to shared library>:<name of test>\n"
      "  List of named tests can be read from a specified\n"
      "  file or read from stdin if no file is specified.\n"
      "\n"
      "OPTIONS\n"
      "%s\n" /* Path        */
      "%s\n" /* Color       */
      "%s\n" /* Cursor      */
      "%s\n" /* Nice        */
      "%s",  /* Interactive */
      _option_named_path, _option_color, _option_cursor, _option_nice,
      _option_interactive);
}

static void _display_list_usage()
{
    printf(
      "chili list <path>...\n"
      "\n"
      "DESCRIPTION\n"
      "  Prints all tests that can be found in the specified shared\n"
      "  libraries.\n"
      "\n"
      "OPTIONS\n"
      "%s",  /* Path */
      _option_path);
}

static int _handle_all_command(int argc, char *argv[])
{
    int c;
    const char **paths;
    int num_paths = 0;
    const char *short_options = "icmnh:";
    const struct option long_options[] = {
        { "interactive", no_argument,       0, 'i' },
        { "color",       no_argument,       0, 'c' },
        { "cursor",      no_argument,       0, 'm' },
        { "nice",        no_argument,       0, 'n' },
        { "help",        no_argument,       0, 'h' },
    };
    int index;
    struct chili_test_options options;

    memset(&options, 0, sizeof(options));

    /* Default to redirect test output to local directory */
    strcpy(options.redirect_path, "./chili_log");
    options.use_redirect = true;

    do {
        c = getopt_long(argc, argv, short_options,
                        long_options, &index);
        switch (c){
            case 'i':
                options.use_color = true;
                options.use_cursor = true;
                options.nice_stats = true;
                break;
            case 'c':
                options.use_color = true;
                break;
            case 'm':
                options.use_cursor = true;
                break;
            case 'n':
                options.nice_stats = true;
                break;
            case 'h':
                _display_all_usage();
                return -1;
        }
    } while (c != -1);

    if (optind < argc){
        paths = (const char**)&argv[optind];
        num_paths = argc - optind;
    }
    else{
        printf("Specify path to shared library "
               "containing tests\n");
        return -1;
    }

    /* Need to reset to be able to parse again */
    optind = 0;

    return chili_command_all(paths, num_paths, &options);
}

static int _handle_named_command(int argc, char *argv[])
{
    int c;
    const char *path;
    const char *short_options = "icmnh:";
    const struct option long_options[] = {
        { "interactive", no_argument,       0, 'i' },
        { "color",       no_argument,       0, 'c' },
        { "cursor",      no_argument,       0, 'm' },
        { "nice",        no_argument,       0, 'n' },
        { "help",        no_argument,       0, 'h' },
    };
    int index;
    struct chili_test_options options;

    memset(&options, 0, sizeof(options));

    /* Default to redirect test output to local directory */
    strcpy(options.redirect_path, "./chili_log");
    options.use_redirect = true;

    do {
        c = getopt_long(argc, argv, short_options,
                        long_options, &index);
        switch (c){
            case 'i':
                options.use_color = true;
                options.use_cursor = true;
                options.nice_stats = true;
                break;
            case 'c':
                options.use_color = true;
                break;
            case 'm':
                options.use_cursor = true;
                break;
            case 'n':
                options.nice_stats = true;
                break;
            case 'h':
                _display_all_usage();
                return -1;
        }
    } while (c != -1);

    if (optind < argc){
        path = argv[optind];
    }
    else {
        path = NULL;
    }

    /* Need to reset to be able to parse again */
    optind = 0;

    return chili_command_named(path, &options);
}

static int _handle_list_command(int argc, char *argv[])
{
    int c;
    const char **paths;
    int num_paths = 0;
    const char *short_options = "h:";
    const struct option long_options[] = {
        { "help", no_argument, 0, 'h' },
    };
    int index;

    do {
        c = getopt_long(argc, argv, short_options,
                        long_options, &index);
        switch (c){
            case 'h':
                _display_list_usage();
                return -1;
        }
    } while (c != -1);

    if (optind < argc){
        paths = (const char**)&argv[optind];
        num_paths = argc - optind;
    }
    else{
        printf("Specify path to shared library "
               "containing tests\n");
        return -1;
    }

    /* Need to reset to be able to parse again */
    optind = 0;

    return chili_command_list(paths, num_paths);
}

static int _handle_help_command(int argc, char *argv[])
{
    const char *command;

    if (argc < 2){
        _display_usage();
        return 1;
    }

    command = argv[1];

    if (strcmp(command, "all") == 0){
        _display_all_usage();
        return 1;
    }

    if (strcmp(command, "list") == 0){
        _display_list_usage();
        return 1;
    }

    if (strcmp(command, "named") == 0){
        _display_named_usage();
        return 1;
    }

    printf("Unknown help topic: %s\n", command);
    return -1;
}

int main(int argc, char *argv[])
{
    const char *command;

    if (argc == 1){
        printf("Specify command\n");
        goto on_error;
    }

    command = argv[1];
    /* Adjust for using getopt in command
     * parsers */
    argc = argc - 1;
    argv = argv + 1;

    if (strcmp(command, "all") == 0){
        return _handle_all_command(argc, argv) > 0 ?
            0 : 1;
    }
    else if (strcmp(command, "named") == 0){
        return _handle_named_command(argc, argv) > 0 ?
            0 : 1;
    }
    else if (strcmp(command, "list") == 0){
        return _handle_list_command(argc, argv) >= 0 ?
            0 : 1;
    }
    else if (strcmp(command, "help") == 0){
        return _handle_help_command(argc, argv) >= 0 ?
            0 : 1;
    }

    printf("Unknown command: %s\n", command);

on_error:
    _display_usage();
    return 1;
}
