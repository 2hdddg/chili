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
         "Usage: chili COMMAND [OPTIONS]...\n"
         "COMMAND is command to invoke, typically test.\n"
         "\n"
         "Valid commands are:\n"
         "  all     Runs all tests in specified shared libraries\n"
         "  list    Lists tests in specified shared libraries\n"
         "  help    Shows help about a specified command\n");
}

static void _display_all_usage()
{
    puts("chili - test runner\n"
         "All command, runs tests\n"
         "Usage: chili all [OPTIONS]... LIBRARY [..]\n"
         "LIBRARY is a shared library containing tests\n"
         "to be invoked. More than one library can be\n"
         "specified."
         "\n"
         "Options are:\n"
         "  -c --color         Use colored output.\n"
         "  -m --cursor        Minimize output on console,\n"
         "                     moves cursor.\n"
         "  -i --interactive   Short for -c -m.\n");
}

static void _display_list_usage()
{
    puts("chili - test runner\n"
         "List command, lists tests.\n"
         "Usage: chili list LIBRARY [..]\n"
         "LIBRARY is a shared library containing tests\n"
         "to be invoked. More than one library can be\n"
         "specifed\n");
}
static int _handle_all_command(int argc, char *argv[])
{
    int c;
    const char **suite_paths;
    int num_suite_paths = 0;
    const char *short_options = "icmnh:";
    const struct option long_options[] = {
        /*   Turns on color, cursor movements. */
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

    return chili_command_all(suite_paths, num_suite_paths, &options);
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

static int _handle_help_command(int argc, char *argv[])
{
    const char *command = argv[1];

    if (strcmp(command, "all") == 0){
        _display_all_usage();
        return 1;
    }

    if (strcmp(command, "list") == 0){
        _display_list_usage();
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
