#include <stdlib.h>
#include <signal.h>

int test_crash_one()
{
    raise(SIGSEGV);
    return 1;
}

int test_crash_two()
{
    raise(SIGSEGV);
    return 1;
}
