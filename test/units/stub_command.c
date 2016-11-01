#include <stdio.h>
#include "command.h"
#include "stub_command.h"


int chili_command_test(const char **suite_path,
                       int num_suite_paths,
                       const struct chili_test_options *options)
{
    return stub_command_test(suite_path, num_suite_paths,
                             options);
}

int chili_command_list(const char **suite_paths,
                       int num_suite_paths)
{
    return stub_command_list(suite_paths, num_suite_paths);
}
