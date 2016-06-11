#include <stdio.h>

static int _once = 0;
static int _each = 0;

int once_before()
{
    _once++;
    return 1;
}

int once_after()
{
    _once--;
    return 1;
}

int each_before()
{
    _each++;
    return 1;
}

int each_after()
{
    _each--;
    return 1;
}

int test_once_before()
{
    return _once == 1;
}

int test_each_before()
{
    return _each == 1;
}
