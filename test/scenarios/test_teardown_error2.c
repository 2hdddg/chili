int _num_each_after = 0;

int each_before()
{
    return 1;
}

int each_after()
{
    /* Fail second time */
    _num_each_after++;
    return _num_each_after > 1 ? -1 : 1;
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
