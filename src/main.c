#include <string.h>
#include <stdio.h>
#include <getopt.h>
#include <stdbool.h>
#include <signal.h>

#include "command.h"

/* Debugging */
#define DEBUG 0
#include "debug.h"


static void _display_usage()
{
    puts("chili - test runner\n"
         "Usage: chili COMMAND [OPTIONS]... FILE\n"
         "COMMAND is command to invoke, typically test.\n"
         "\n"
         "Valid commands are:\n"
         "  test    Runs test in specified shared library\n"
         "  list    Lists tests in specified shared library\n"
         "  help    Shows help about a specified command\n");
}

static void _display_test_usage()
{
    puts("chili - test runner\n"
         "Test command, runs tests\n"
         "Usage: chili test [OPTIONS]... FILE\n"
         "FILE is a shared library containing tests\n"
         "to be invoked.\n"
         "\n"
         "Options are:\n"
         "  -c --color         use colored output\n"
         "  -m --cursor        minimize output on console,\n"
         "                     moves cursor\n"
         "  -r --redirect\n"
         "  -i --interactive   short for -c -m -r\n");
}

static void _display_list_usage()
{
    puts("chili - test runner\n"
         "List command, lists tests.\n"
         "Usage: chili list FILE\n"
         "FILE is a shared library containing tests\n"
         "to be invoked.\n"
         "\n");
}
static int _handle_test_command(int argc, char *argv[])
{
    int c;
    const char **suite_paths;
    int num_suite_paths = 0;
    const char *short_options = "icmnrh:";
    const struct option long_options[] = {
        /*   Turns on color, cursor movements and redirect.
         *   Default path for redirect is ./chili_log */
        { "interactive", no_argument,       0, 'i' },
        { "color",       no_argument,       0, 'c' },
        { "cursor",      no_argument,       0, 'm' },
        { "nice",        no_argument,       0, 'n' },
        { "redirect",    required_argument, 0, 'r' },
        { "help",        no_argument,       0, 'h' },
    };
    int index;
    int len;
    struct chili_test_options options;

    memset(&options, 0, sizeof(options));

    do {
        c = getopt_long(argc, argv, short_options,
                        long_options, &index);
        switch (c){
            case 'i':
                options.use_color = true;
                options.use_cursor = true;
                options.use_redirect = true;
                options.nice_stats = true;
                strcpy(options.redirect_path, "./chili_log");
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
            case 'r':
                len = strlen(optarg);
                if (len >= CHILI_REDIRECT_MAX_PATH){
                    printf("Too long redirect path\n");
                    return -1;
                }
                options.use_redirect = true;
                strcpy(options.redirect_path, optarg);
                break;
            case 'h':
                _display_test_usage();
                return -1;
        }
    } while (c != -1);

    if (optind < argc){
        suite_paths = (const char**)&argv[optind];
        num_suite_paths = argc - optind;
    }
    else{
        printf("Specify path to shared library "
               "containing tests\n");
        return -1;
    }

    /* Need to reset to be able to parse again */
    optind = 0;

    return chili_command_test(suite_paths, num_suite_paths, &options);
}

static int _handle_list_command(int argc, char *argv[])
{
    int c;
    const char **suite_paths;
    int num_suite_paths = 0;
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
        suite_paths = (const char**)&argv[optind];
        num_suite_paths = argc - optind;
    }
    else{
        printf("Specify path to shared library "
               "containing tests\n");
        return -1;
    }

    /* Need to reset to be able to parse again */
    optind = 0;

    return chili_command_list(suite_paths, num_suite_paths);
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

    if (strcmp(command, "test") == 0){
        return _handle_test_command(argc, argv) > 0 ?
            0 : 1;
    }
    else if (strcmp(command, "list") == 0){
        return _handle_list_command(argc, argv) >= 0 ?
            0 : 1;
    }

    printf("Unknown command: %s\n", command);

on_error:
    _display_usage();
    return 1;
}
