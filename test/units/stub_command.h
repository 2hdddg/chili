#pragma once

/* Callbacks used by stub implementation, set these in test
 * code to check parameters or check reaction.
 */
int (*stub_command_all)(const char **library_path,
                        int num_library_paths,
                        const struct chili_test_options *options);
int (*stub_command_list)(const char **library_paths,
                         int num_library_paths);

