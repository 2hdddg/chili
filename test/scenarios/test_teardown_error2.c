#include <stdlib.h>
#include <sys/mman.h>

static int *_num_each_after;

int once_before()
{
    _num_each_after = mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE,
                           MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    if (_num_each_after == ((void*)-1)){
        return -1;
    }
    *_num_each_after = 0;
    return 1;
}

int once_after()
{
    return munmap(_num_each_after, sizeof(int));
}

int each_after()
{
    /* Fail second time */
    *_num_each_after = *_num_each_after + 1;
    return *_num_each_after > 1 ? -1 : 1;
}

int test_success_might_run()
{
    return 1;
}

int test_second_success_that_might_run()
{
    return 1;
}

int test_third_success_that_might_run()
{
    return 1;
}
