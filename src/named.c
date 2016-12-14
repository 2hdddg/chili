#include <ctype.h>
#include <string.h>

#include "named.h"


int chili_named_parse(char *line,
                      char **library_path,
                      char **test_name)
{
    char *lib;
    char *sep = strchr(line, ':');
    char *tst;
    int  len;

    /* Separator not found */
    if (sep == NULL){
        return -1;
    }
    *sep = '\0';
    tst = sep + 1;

    /* Ignore whitespace in the beginning of library name */
    lib = line;
    while (lib < sep){
        if (!isspace(*lib)){
            break;
        }
        lib++;
    }
    /* Ignore whitespace in the end of library name */
    sep--;
    while (sep > lib){
        if (!isspace(*sep)){
            break;
        }
        *sep = '\0';
        sep--;
    }
    /* Ignore whitespace in the beginning of test name */
    len = strlen(tst);
    while (len){
        if (!isspace(*tst)){
            break;
        }
        tst++;
        len--;
    }
    /* Ignore whitespace in the end of test name. */
    while (len){
        if (!isspace(tst[len - 1])){
            break;
        }
        tst[len - 1] = '\0';
        len--;
    }

    if (strlen(lib) == 0 ||
        strlen(tst) == 0) {
        return -1;
    }

    *library_path = lib;
    *test_name = tst;

    return 1;
}

