#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "redirect.h"

/* Globals */
static int _stdout_copy;
static int _stdout_temp;
static const char *_stdout_name;


/* Exports */
int chili_redirect_begin()
{
    return 1;
}

void chili_redirect_start(const char *name)
{
    _stdout_temp = creat(name, S_IRUSR|S_IWUSR);
    if (_stdout_temp == -1){
        printf("Failed to create file %s due to %s\n",
            name, strerror(errno));
        return;
    }
    _stdout_name = name;

    fflush(stdout);
    _stdout_copy = dup(1);
    dup2(_stdout_temp, 1);
    close(_stdout_temp);
    _stdout_temp = 0;
}

void chili_redirect_stop()
{
    if (!_stdout_name){
        return;
    }

    fflush(stdout);
    dup2(_stdout_copy, 1);
    close(_stdout_copy);
    _stdout_copy = 0;
}

void chili_redirect_print(const char *name,
    const char* before, const char* after)
{
    int fd;
    const int max = 1024;
    char buf[max];
    int size;

    write(1, before, strlen(before));

    fd = open(name, O_RDONLY);
    while ((size = read(fd, buf, max)) > 0){
        write(1, buf, size);
    }
    close(fd);

    write(1, after, strlen(after));
}

void chili_redirect_end()
{
}
