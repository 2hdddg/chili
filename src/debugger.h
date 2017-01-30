#pragma once

#include <sys/types.h>
#include "handle.h"

/**
 * @brief Creates debugger module instance.
 *
 * The debugger module instance contains the
 * configuration and behaviour of the debugger
 * used on the system.
 *
 * @param chili_path Path to chili executable
 *
 * @return Negative on error.
 *         Positive on success.
 */
int chili_dbg_create(const char *chili_path,
                     chili_handle *handle);

/**
 * @brief Attaches debugger
 *
 * The debugger is attached to the specified
 * process and a breakpoint is set in the
 * specified function.
 *
 * @param handle    Debugger instance.
 * @param processid Process id of the process
 *                  containing test to debug.
 * @param func_name Name of function in process
 *                  that should be debugged.
 *
 * @return Negative on error.
 *         Does not return on success.
 */
int chili_dbg_attach(chili_handle handle,
                     pid_t processid,
                     const char *func_name);

/**
 * @brief Prepares debug target for debugging.
 *
 * Should be called from process that executes
 * code to be debugged.
 *
 */
void chili_dbg_target_prepare(chili_handle handle);

/**
 * @brief Pauses debug target until debugger
 *        is attached.
 *
 * Should be called from process that executes
 * code to be debugged.
 */
void chili_dbg_target_hold(chili_handle handle);

/**
 * @brief Frees allocated resources.
 *
 * @param handle Valid module handle.
*/
int chili_dbg_destroy(chili_handle handle);

