#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include "debugger.h"

/* Debugging */
#define DEBUG_PRINTS 0
#include "debug.h"


struct instance {
    char *chili_path;
};

 /* No need to do anything but a handler is
  * needed.
  * Continue signal is sent by debugger when
  * it has been setup. Debugged process is
  * stopped before right before the test is
  * to be executed.
  */
static void _sigcont_handler(int sig)
{
}

int chili_dbg_create(const char *chili_path,
                     chili_handle *handle)
{
    struct instance *instance = NULL;

    debug_print("Creating debugger instance\n");

    if (chili_path == NULL) {
        return -1;
    }

    instance = malloc(sizeof(*instance));
    if (instance == NULL) {
        goto on_failure;
    }

    instance->chili_path = strdup(chili_path);
    if (instance->chili_path == NULL) {
        goto on_failure;
    }

    *handle = instance;
    return 1;

on_failure:
    if (instance != NULL){
        if (instance->chili_path != NULL){
            free(instance->chili_path);
        }
        free(instance);
    }

    *handle = NULL;
    return -1;
}

int chili_dbg_attach(chili_handle handle,
                     pid_t pid,
                     const char *func_name)
{
    struct instance *instance = handle;
    char            processid[25];
    const char      *breakpoint_template = "-ex=""break %s""";
    char            *breakpoint;
    int             breakpoint_len;
    int             ret;

    /* gdb commandline parameters */
    char *argvs[] = {
        "",
        instance->chili_path,
        NULL, /* process id */
        NULL, /* breakpoint */
        "-ex=""signal SIGCONT""",
        NULL
    };

    /* Fill in process id */
    snprintf(processid, sizeof(processid), "%d", pid);
    argvs[2] = processid;

    /* Fill in function to break in */
    breakpoint_len = strlen(breakpoint_template) +
                     strlen(func_name) + 1;
    breakpoint = malloc(breakpoint_len);
    if (breakpoint == NULL) {
        return -1;
    }
    snprintf(breakpoint, breakpoint_len,
             breakpoint_template, func_name);
    argvs[3] = breakpoint;

    /* Exec gdb, no need to free malloc:ed stuff
     * if exec succeeds. */
    execvp("gdb", argvs);

    /* We only get here on error */
    ret = errno;
    printf("Failed to invoke gdb: %s\n", strerror(ret));
    free(breakpoint);
    return -ret;
}

void chili_dbg_target_prepare(chili_handle handle)
{
    /* Need handler for SIGCONT to be able to wake up
     * after being paused. */
    signal(SIGCONT, _sigcont_handler);
}

void chili_dbg_target_hold(chili_handle handle)
{
    /* Pause debugged process until debugger
     * sends a continue signal. */
    pause();
}

int chili_dbg_destroy(chili_handle handle)
{
    struct instance *instance = handle;

    debug_print("Destroying debugger instance\n");

    free(instance->chili_path);
    free(instance);

    return 1;
}

