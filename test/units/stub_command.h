#pragma once

/* Callbacks used by stub implementation, set these in test
 * code to check parameters or check reaction.
 */
int (*stub_command_test)(const char **suite_path,
                         int num_suite_paths,
                         const struct chili_test_options *options);
int (*stub_command_list)(const char **suite_paths,
                         int num_suite_paths);

