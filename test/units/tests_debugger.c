#include <stdio.h>

#include "assert.h"
#include "debugger.h"

static int _ret;
static chili_handle _handle;
static char *_chili_path;

int each_before()
{
    _ret = 0;
    _chili_path = "../chili";
    _handle = NULL;

    return 1;
}

int each_after()
{
    if (_handle != NULL){
        chili_dbg_destroy(_handle);
    }

    return 1;
}

/* Verifies that simple creation works.
 */
int test_create_succeeds()
{
    printf("in test\n");
    _ret = chili_dbg_create(_chili_path,
                            &_handle);

    return assert_ret_success(_ret) &&
           assert_ptr_not_null(_handle);
}

/* Verifies that destroy doesn't crash.
 */
int test_destroy_succeeds()
{
    chili_dbg_create(_chili_path,
                     &_handle);
    chili_dbg_destroy(_handle);
    _handle = NULL;

    return 1;
}

