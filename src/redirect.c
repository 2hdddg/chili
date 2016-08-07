#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "redirect.h"


/* Globals */
static int _enabled;
static int _stdout_copy;
static int _stdout_temp;
static int _name_offset;
/* Pre allocate buffers for path + name */
static char _stdout_name[
    CHILI_REDIRECT_MAX_PATH + CHILI_REDIRECT_MAX_NAME + 1];
static char _print_name[
    CHILI_REDIRECT_MAX_PATH + CHILI_REDIRECT_MAX_NAME + 1];


static int _ensure_directory(const char *path)
{
    struct stat st;
    int is_dir;
    int can_write;
    int created;

    if (stat(path, &st) < 0){
        created = mkdir(path, 0777);
        if (created < 0){
            printf("Failed to create directory %s error %s\n",
                path, strerror(errno));
            return -1;
        }
    }
    else{
        /* Path exists */
        is_dir = st.st_mode & S_IFDIR;
        if (!is_dir){
            printf("%s is not a directory.\n", path);
            return -1;
        }

        can_write = st.st_mode & S_IWUSR
            || st.st_mode & S_IWGRP
            || st.st_mode & S_IWOTH;
        if (!can_write){
            printf("Cannot write to %s.\n", path);
            return -1;
        }
    }
    return 1;
}

static int _build_path(char *buffer, const char *name)
{
    if (strlen(name) >= CHILI_REDIRECT_MAX_NAME){
        printf("Too long name to build path\n");
        return -1;
    }
    strcpy(buffer + _name_offset, name);
    return 1;
}

/* Exports */
int chili_redirect_begin(int enable, const char *path)
{
    int valid_path;
    int path_length;

    _enabled = enable;
    if (_enabled){
        path_length = strlen(path);
        if (path_length >= CHILI_REDIRECT_MAX_PATH){
            return -1;
        }

        /* Check if directory exists and is accessible.
         * If not, try create directory. */
        valid_path = _ensure_directory(path);
        if (valid_path < 0){
            return -1;
        }

        /* Copy path into pre-allocated buffers and 
         * make sure that it ends with a slash */
        strncpy(_stdout_name, path, path_length);
        if (_stdout_name[path_length - 1] != '/'){
            _stdout_name[path_length] = '/';
            path_length++;
        }
        strncpy(_print_name, _stdout_name, path_length);
        _name_offset = path_length;
    }

    return 1;
}

void chili_redirect_start(const char *name)
{
    if (!_enabled){
        return;
    }
    if (_build_path(_stdout_name, name) < 0){
        return;
    }

    _stdout_temp = creat(_stdout_name, S_IRUSR|S_IWUSR);
    if (_stdout_temp == -1){
        printf("Failed to create file %s due to %s\n",
            _stdout_name, strerror(errno));
        return;
    }

    fflush(stdout);
    _stdout_copy = dup(1);
    dup2(_stdout_temp, 1);
    close(_stdout_temp);
    _stdout_temp = 0;
}

void chili_redirect_stop()
{
    if (!_enabled){
        return;
    }

    fflush(stdout);
    dup2(_stdout_copy, 1);
    close(_stdout_copy);
    _stdout_copy = 0;
}

void chili_redirect_print(const char *name, const char* before, 
                          const char* after)
{
    int fd;
    const int max = 1024;
    char buf[max];
    int size;

    if (!_enabled){
        return;
    }
    if (_build_path(_print_name, name) < 0){
        return;
    }

    if (before){
        write(1, before, strlen(before));
    }

    fd = open(_print_name, O_RDONLY);
    while ((size = read(fd, buf, max)) > 0){
        write(1, buf, size);
    }
    close(fd);

    if (after){
        write(1, after, strlen(after));
    }
}

void chili_redirect_end()
{
}
