#include <stdio.h>
#include <dlfcn.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>

#include "redirect.h"
#include "run.h"
#include "debugger.h"

/* Debugging */
#define DEBUG_PRINTS 0
#include "debug.h"

/* Types */
struct child_result {
    enum fixture_result  before;
    enum test_result     test;
    enum fixture_result  after;
};

/* Globals */
static int _next_identity = 0;

/* Locals */
static void _sigchld_handler(int sig, siginfo_t *info, void* context)
{
    debug_print("Received signal %d in process %d "
                "from process %d\n", sig, getpid(), info->si_pid);
}

static int _signal_setup()
{
    struct sigaction action = {
            .sa_sigaction = _sigchld_handler,
            .sa_flags = SA_SIGINFO
        };

    /* Set blocked signals while handling */
    sigemptyset(&action.sa_mask);
    sigaction(SIGCHLD, &action, NULL);

    return 0;
}

static enum fixture_result evaluate_fixture(chili_func fixture)
{
    if (fixture){
        return fixture() < 0 ?
            fixture_error : fixture_success;
    }
    else{
        return fixture_not_needed;
    }
}

static enum test_result evaluate_test(chili_func test)
{
    int returned = test();

    if (returned < 0){
        return test_error;
    }
    return returned == 0 ?
        test_failure : test_success;
}

static void _aggregate(const struct chili_result *result,
                       struct chili_aggregated *aggregated)
{
    bool executed = result->execution != execution_not_started;
    bool error = result->before == fixture_error ||
                 result->after == fixture_error ||
                 result->test == test_error ||
                 result->execution == execution_unknown_error ||
                 result->execution == execution_crashed ||
                 result->execution == execution_timed_out;
    bool failed = executed && !error &&
                  result->test == test_failure;
    bool succeeded = executed && !error && !failed &&
                     result->test == test_success;

    aggregated->num_total += executed ? 1 : 0;
    aggregated->num_errors += error ? 1 : 0;
    aggregated->num_failed += failed ? 1 : 0;
    aggregated->num_succeeded += succeeded ? 1 : 0;
}

static int _child_write_result(chili_func each_before,
                               chili_func test,
                               chili_func each_after,
                               const char *name,
                               const char *redirect_name,
                               int result_pipe)
{
    int written;
    struct child_result result = {
        .before = fixture_uncertain,
        .test   = test_uncertain,
        .after  = fixture_uncertain,
    };

    debug_print("In child preparing to execute test\n");

    /* Everything written to stdout in tests might be
     * redirected somewhere else */
    chili_redirect_start(redirect_name);

    result.before = evaluate_fixture(each_before);
    if (result.before != fixture_error){
        result.test = evaluate_test(test);
        result.after = evaluate_fixture(each_after);
    }

    chili_redirect_stop();

    written = write(result_pipe, &result, sizeof(result));
    if (written != sizeof(result)){
        printf("Wrong number of bytes written\n");
        return -1;
    }
    close(result_pipe);

    return 1;
}

static void _me_read_result(struct chili_result *result,
                            const struct chili_times *times,
                            int result_pipe)
{
    fd_set readset;
    sigset_t emptyset;
    int selected;
    struct child_result from_child;
    int received;

    FD_ZERO(&readset);
    FD_SET(result_pipe, &readset);
    sigemptyset(&emptyset);

    selected = pselect(result_pipe + 1, &readset, NULL, NULL,
                       &times->timeout, &emptyset);
    if (selected > 0){
        received = read(result_pipe, &from_child, sizeof(from_child));
        if (received != sizeof(from_child)){
            printf("Read wrong number of bytes from child\n");
            result->execution = execution_unknown_error;
        }
        else{
            /* This is the "normal" scenario */
            debug_print("Received result from child process\n");
            result->execution = execution_done;
            result->before = from_child.before;
            result->test = from_child.test;
            result->after = from_child.after;
        }
    }
    else if (selected == 0){
        /* Timeout */
        result->execution = execution_timed_out;
        debug_print("Timeout while waiting for child process\n");
        /* Child is still running at this point. */
    }
    else{
        if (errno == EINTR){
            /* Got signal while waiting, until we expect
             * anything but SIGCHILD we can assume that
             * the child crashed before it wrote anything */
            debug_print("Child process crashed during "
                        "test in process %d\n", getpid());
            result->execution = execution_crashed;
        }
        else{
            /* Got an error */
            debug_print("Error while waiting for test to complete\n");
            result->execution = execution_unknown_error;
            /* Child is still running at this point */
        }
    }
}

static int _fork_and_run(chili_func each_before,
                         chili_func test,
                         chili_func each_after,
                         struct chili_result *result,
                         const struct chili_times *times)
{
    pid_t child;
    int pipes[2];
    sigset_t blocked_signals;
    int status;
    char identity[25];

    if (pipe(pipes) < 0){
        printf("Failed to create pipe: %s\n", strerror(errno));
        return -1;
    }

    /* Block SIGCHLD before we start the child process, otherwise
     * we can have a race where the process crashes and we don't
     * notice it */
    sigemptyset(&blocked_signals);
    sigaddset(&blocked_signals, SIGCHLD);
    sigprocmask(SIG_BLOCK, &blocked_signals, NULL);

    child = fork();
    if (child < 0){
        printf("Failed to fork: %s\n", strerror(errno));
        return -1;
    }

    snprintf(identity, 25, "%d", result->identity);
    if (child == 0){
        _child_write_result(each_before, test, each_after,
                            result->name, identity, pipes[1]);
        /* Exit child here ! */
        debug_print("Exiting process %d\n", getpid());
        _exit(0);
    }


    /* Continue in parent process */
    debug_print("Waiting for child process %d to execute test\n", child);
    _me_read_result(result, times, pipes[0]);
    close(pipes[0]);
    close(pipes[1]);

    if (result->execution == execution_timed_out ||
        result->execution == execution_unknown_error){
        /* Child is still running, shoot it down */
        if (kill(child, SIGKILL) < 0){
            printf("Failed to kill timed out child process\n");
            /* Restore signals and bail out, not safe to wait 
               or continue testing
             */
            sigprocmask(SIG_UNBLOCK, &blocked_signals, NULL);
            return -1;
        }
    }

    /* Child either exited normally or killed by code above */
    wait(&status);

    /* Restore signals, there is probably a SIGCHLD
     * pending at this moment from this test and we
     * don't want that signal next round ! */
    sigprocmask(SIG_UNBLOCK, &blocked_signals, NULL);

    return 1;
}

static int _fork_and_debug(chili_handle debugger,
                           chili_func each_before,
                           chili_func test,
                           chili_func each_after,
                           const char *name)
{
    pid_t child = fork();
    enum fixture_result result_before;

    if (child < 0){
        printf("Failed to fork: %s\n", strerror(errno));
        return -1;
    }

    if (child == 0){
        debug_print("In child preparing to debug test\n");
        chili_dbg_target_prepare(debugger);

        /* Run setup fixture */
        result_before = evaluate_fixture(each_before);
        if (result_before != fixture_error){
            /* Hold until debugger attached */
            chili_dbg_target_hold(debugger);
            /* Run the actual test, debugger has
             * been setup to break in test func. */
            test();
            /* Run teardown fixture */
            evaluate_fixture(each_after);
        }
        /* Exit child here ! */
        debug_print("Exiting process %d\n", getpid());
        _exit(0);
    }

    /* Continue in parent process, use parent to
     * exec into debugger. */
    return chili_dbg_attach(debugger, child, name);
}

/* Exports */
int chili_run_before(const struct chili_bind_fixture *fixture)
{
    int r;

    if (fixture->once_before){
        r = fixture->once_before();
        if (r < 0){
            return r;
        }
    }

    _signal_setup();

    return 1;
}

int chili_run_test(struct chili_result *result,
                   struct chili_aggregated *aggregated,
                   const struct chili_bind_test *test,
                   const struct chili_bind_fixture *fixture,
                   const struct chili_times *times,
                   chili_progress test_progress)
{
    result->execution = execution_not_started;
    result->before    = result->after = fixture_uncertain;
    result->test      = test_uncertain;
    result->name      = test->name;
    result->library   = test->library;
    result->identity  = _next_identity++;

    debug_print("Preparing to run %s\n", result->name);

    /* Call hook that test begins even before fixtures
     * to give early feedback */
    if (test_progress){
        test_progress(NULL, result->name);
    }

    if (_fork_and_run(fixture->each_before, test->func,
                      fixture->each_after, result, times) < 0){
        return -1;
    };

    _aggregate(result, aggregated);

    return 1;
}

int chili_run_debug(chili_handle debugger,
                    const struct chili_bind_test *test,
                    const struct chili_bind_fixture *fixture)
{
    return _fork_and_debug(debugger,
                           fixture->each_before, test->func,
                           fixture->each_after, test->name);
}

int chili_run_after(const struct chili_bind_fixture *fixture)
{
    int r = 1;

    if (fixture->once_after){
        r = fixture->once_after();
    }

    return r;
}
