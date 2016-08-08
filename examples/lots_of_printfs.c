#include <stdio.h>

int test_print_a_lot()
{
    int i;

    for (i = 0; i < 25; i++){
        printf("Printing %d\n", i);
    }
    return 1;
}

int test_print_a_lot_and_fail()
{
    int i;

    for (i = 0; i < 25; i++){
        printf("Printing and failing %d\n", i);
    }
    return 0;
}
