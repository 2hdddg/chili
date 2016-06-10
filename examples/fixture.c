#include <stdio.h>

static int _once_before = 0;

int once_before()
{
    _once_before++;
    return 1;
}

int once_after()
{
    return 1;
}

int each_before()
{
    return 1;
}

int each_after()
{
    return 1;
}

int test_succesful()
{
    return _once_before == 1;
}
