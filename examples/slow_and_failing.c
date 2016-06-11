#include <unistd.h>

int test_sleep_1()
{
    sleep(1);
    return 1;
}

int test_sleep_2()
{
    sleep(2);
    return 1;
}

int test_sleep_1_and_fail()
{
    sleep(1);
    return 0;
}
