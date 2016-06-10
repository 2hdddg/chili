#include <stdio.h>

#include "run.h"
#include "report.h"

/* Exports */
int chili_report_begin()
{
    return 1;
}

void chili_report_test(struct chili_result *result)
{
    if (result->test > 0){
        printf("%s succeeded.\n", result->name);
    }
    else{
        printf("%s failed!\n", result->name);
    }
}

void chili_report_end()
{
}
