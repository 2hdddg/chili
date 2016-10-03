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

/* Debugging */
#define DEBUG 0
#include "debug.h"

/* Types */
struct child_result {
    enum fixture_result  before;
    enum test_result     test;
    enum fixture_result  after;
};


/* Globals */
static const struct chili_bind_fixture *_fixture;

/* Locals */
static void _empty_handler(int sig)
{
}

static int _signal_setup()
{
    struct sigaction action = {
            .sa_handler = _empty_handler,
            .sa_flags = 0
        };


    /* Add empty signal handler to make sure we get
     * the signal when we want to */
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
                               const char *name, int result_pipe)
{
    int written;
    struct child_result result = {
        .before = fixture_uncertain,
        .test   = test_uncertain,
        .after  = fixture_uncertain,
    };

    /* Everything written to stdout in tests might be
     * redirected somewhere else */
    chili_redirect_start(name);

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
                            int result_pipe)
{
    fd_set readset;
    struct timespec timeout;
    sigset_t emptyset;
    int selected;
    int status;
    struct child_result from_child;
    int received;

    FD_ZERO(&readset);
    FD_SET(result_pipe, &readset);
    timeout.tv_sec = 10;
    timeout.tv_nsec = 0;
    sigemptyset(&emptyset);

    selected = pselect(result_pipe + 1, &readset, NULL, NULL,
                       &timeout, &emptyset);
    if (selected > 0){
        received = read(result_pipe, &from_child, sizeof(from_child));
        /* Should be safe to wait here since the child exits
         * right after writing */
        wait(&status);
        if (received != sizeof(from_child)){
            printf("Read wrong number of bytes from child\n");
            result->execution = execution_unknown_error;
        }
        else{
            /* This is the "normal" scenario */
            result->execution = execution_done;
            result->before = from_child.before;
            result->test = from_child.test;
            result->after = from_child.after;
        }
    }
    else if (selected == 0){
        /* Timeout */
        result->execution = execution_timed_out;
        /* Child is still running at this point,
         * kill it ? */
    }
    else{
        if (errno == EINTR){
            /* Got signal while waiting, until we expect
             * anything but SIGCHILD we can assume that
             * the child crashed before it wrote anything */
            result->execution = execution_crashed;
            wait(&status);
        }
        else{
            /* Got an error */
            result->execution = execution_unknown_error;
            /* Child is still running at this point,
             * kill it ? */
        }
    }
}

static int _fork_and_run(chili_func each_before,
                         chili_func test,
                         chili_func each_after,
                         struct chili_result *result)
{
    pid_t child;
    int pipes[2];
    sigset_t blocked_signals;
    sigset_t original_signals;

    if (pipe(pipes) < 0){
        printf("Failed to create pipe: %s\n", strerror(errno));
        return -1;
    }

    /* Block SIGCHLD before we start the child process, otherwise
     * we can have a race where the process crashes and we don't
     * notice it */
    sigemptyset(&blocked_signals);
    sigaddset(&blocked_signals, SIGCHLD);
    sigprocmask(SIG_BLOCK, &blocked_signals, &original_signals);

    child = fork();
    if (child < 0){
        printf("Failed to fork: %s\n", strerror(errno));
        return -1;
    }

    if (child == 0){
        _child_write_result(each_before, test, each_after, result->name, pipes[1]);
        /* Exit child here ! */
        _exit(0);
    }

    /* Continue in parent process */
    _me_read_result(result, pipes[0]);
    close(pipes[0]);
    close(pipes[1]);

    /* Restore signals, there is probably a SIGCHLD
     * pending at this moment from this test and we
     * don't want that signal next round ! */
    sigprocmask(SIG_SETMASK, &original_signals, NULL);

    return 1;
}

/* Exports */
int chili_run_begin(const struct chili_bind_fixture *fixture,
                    bool *before_failed)
{
    int r;
    *before_failed = false;

    if (fixture->once_before){
        r = fixture->once_before();
        if (r < 0){
            *before_failed = true;
            return r;
        }
    }

    _fixture = fixture;

    _signal_setup();

    return 1;
}

int chili_run_next(struct chili_result *result,
                   struct chili_aggregated *aggregated,
                   struct chili_bind_test *test,
                   chili_test_begin test_begin)
{
    result->execution = execution_not_started;
    result->before = result->after = fixture_uncertain;
    result->test = test_uncertain;
    result->name = test->name;

    debug_print("Preparing to run %s\n", result->name);

    /* Call hook that test begins even before fixtures
     * to give early feedback */
    if (test_begin){
        test_begin(result->name);
    }

    if (_fork_and_run(_fixture->each_before, test->func,
                      _fixture->each_after, result) < 0){
        return -1;
    };

    _aggregate(result, aggregated);

    return 1;
}

int chili_run_end(bool *after_failed)
{
    int r = 1;
    *after_failed = false;

    if (_fixture->once_after){
        r = _fixture->once_after();
        *after_failed = r < 0;
    }

    return r;
}
