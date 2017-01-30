#include "assert.h"
#include "registry.h"

static int _ret;
static int _token;
static chili_handle _handle;
static chili_handle _one = (chili_handle)1;
static chili_handle _two = (chili_handle)2;

int each_before()
{
    _ret = 0;
    _token = 0;
    _handle = NULL;

    return 1;
}

int each_after()
{
    if (_handle != NULL){
        chili_reg_destroy(_handle);
    }

    return 1;
}

/* Verifies that simplest possible creation
 * works.
 */
int test_create_succeeds()
{
    _ret = chili_reg_create(1, &_handle);

    return assert_ret_success(_ret) &&
           assert_ptr_not_null(_handle);
}

/* Verifies that destroy not crashes.
 */
int test_destroy_succeeds()
{
    chili_reg_create(1, &_handle);
    chili_reg_destroy(_handle);
    _handle = NULL;

    return 1;
}

/* Verifies that an item can be added when there is capacity.
 */
int test_can_add()
{
    int retrieved = 3;
    chili_reg_create(1, &_handle);

    _ret = chili_reg_add(_handle, "name", &retrieved);

    retrieved = *(int*)chili_reg_next(_handle, &_token);
    return assert_ret_success(_ret) &&
           assert_int(3, retrieved);
}

/* Verifies that an item can be added when capacity is reached.
 */
int test_can_add_over_initial_size()
{
    int *next;
    chili_reg_create(1, &_handle);

    /* Add two items when initial capacity is 1 */
    chili_reg_add(_handle, "one", &_one);
    _ret = chili_reg_add(_handle, "two", &_two);

    next = chili_reg_next(_handle, &_token);
    next = chili_reg_next(_handle, &_token);

    return assert_ret_success(_ret) &&
           assert_ptr_not_null(next) &&
           assert_int(2, *next);
}

/* Verifies that iteration using next function
 * iterates over the expected number of items.
 */
int test_next_returns_sequence()
{
    int num = -1;
    void *next = NULL;
    chili_reg_create(2, &_handle);
    chili_reg_add(_handle, "one", &_one);
    chili_reg_add(_handle, "two", &_two);

    do {
        next = chili_reg_next(_handle, &_token);
        num++;
    } while (next != NULL);

    return assert_int(2, num);
}

/* Verifies that data can be retrieved by name
 */
int test_can_find()
{
    int *found_1;
    int *found_2;
    chili_reg_create(2, &_handle);

    chili_reg_add(_handle, "one", &_one);
    chili_reg_add(_handle, "two", &_two);

    found_2 = (int*)chili_reg_find(_handle, "two");
    found_1 = (int*)chili_reg_find(_handle, "one");

    return assert_ptr_not_null(found_1) &&
           assert_ptr_not_null(found_2) &&
           assert_int(1, *found_1) &&
           assert_int(2, *found_2);
}

/* Verifies that NULL is returned when not found
 */
int test_find_when_not_existing()
{
    chili_handle found;
    chili_reg_create(2, &_handle);

    chili_reg_add(_handle, "one", &_one);

    found = chili_reg_find(_handle, "nothing");

    return assert_ptr_null(found);
}

