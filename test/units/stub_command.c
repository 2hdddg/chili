#include <stdio.h>
#include "command.h"
#include "stub_command.h"


int chili_command_all(const char **library_paths,
                      int num_library_paths,
                      const struct chili_test_options *options)
{
    return stub_command_all(library_paths, num_library_paths,
                            options);
}

int chili_command_list(const char **library_paths,
                       int num_library_paths)
{
    return stub_command_list(library_paths, num_library_paths);
}
