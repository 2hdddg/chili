#include <stdio.h>

/* No guard by design */
#define debug_print(fmt, ...) \
    do { \
        if (DEBUG_PRINTS) \
            fprintf(stderr, "%s:%d:%s: " fmt, \
                    __FILE__, __LINE__, __func__, \
                    ##__VA_ARGS__); \
       } while (0)
