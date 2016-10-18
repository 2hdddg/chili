#include <stdio.h>
#include "command.h"
#include "stub_command.h"


int chili_command_test(const char *suite_path,
                       const struct chili_test_options *options)
{
    return stub_command_test(suite_path, options);
}

