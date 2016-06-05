#include <stdio.h>

#include "symbols.h"

int main()
{
    int symbol_count;
    int i;
    int r;
    char *name;

    /* Initialize modules */
    r = chili_sym_begin("/usr/lib/libtidy.so", &symbol_count);
    if (r < 0){
        goto cleanup;
    }
    r = chili_run_begin(symbol_count);
    if (r < 0){
        goto cleanup;
    }

    /* Evaluate symbols to find tests and setup */
    for (i = 0; i < symbol_count; i++){
        r = chili_sym_next(i, &name);
        if (r < 0){
            goto cleanup;
        }
        r = chili_run_eval(name);
        if (r < 0){
            goto cleanup;
        }
    }

cleanup:
    chili_run_end();
    chili_sym_end();

    return r < 0 ? 1 : 0;
}
