#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>

#include "symbols.h"


/* Types */
enum endianness {
    big_endian,
    little_endian
};

struct header {
    int b;
    enum endianness e;
    long sections_offset;
    int section_size;
    int section_count;
};

struct section {
    int name;
    int type;
    int offset;
    int size;
    int entsize;
    int link;
};

struct symbol {
    long name;
};


/* Globals */
static int _fd;
static long _fdsize;
static char *_map;
static struct header _header;
static struct section _dynsym;
static struct section _dynstr;
static int _count;
static int _next;


/* Read functions */
static int _get_16(enum endianness e, char *buf)
{
    unsigned char c1 = *buf++;
    unsigned char c2 = *buf;

    return e == little_endian ?
        (c2 << 8) | c1 :
        -1;
}

static int _get_32(enum endianness e, char *buf)
{
    unsigned char c1 = *buf++;
    unsigned char c2 = *buf++;
    unsigned char c3 = *buf++;
    unsigned char c4 = *buf;

    return e == little_endian ?
        (c4 << 24) | c3 << 16 | c2 << 8 | c1 :
        -1;
}

static int _get_64(enum endianness e, char *buf)
{
    return -1;
}


/* Parse functions */
static int _is_elf(char *map)
{
    return
        map[0] == 0x7f &&
        map[1] == 'E' &&
        map[2] == 'L' &&
        map[3] == 'F';
}

static int _get_bitness(char *map)
{
    char c = map[4];

    switch (c){
    case 1:
        return 32;
    case 2:
        return 64;
    default:
        return -1;
    }
}

static enum endianness _get_endianness(char *map)
{
    char c = map[5];

    switch (c){
    case 1:
        return little_endian;
    case 2:
        return big_endian;
    default:
        return -1;
    }
}

static long _get_sections_offset(char *map, enum endianness e, int b)
{
    return b == 32 ?
        _get_32(e, map + 0x20) :
        _get_64(e, map + 0x28);
}

static int _get_section_count(char *map, enum endianness e, int b)
{
    return b == 32 ?
        _get_16(e, map + 0x30) :
        _get_32(e, map + 0x3c);
}

static int _get_section_size(char *map, enum endianness e, int b)
{
    return b == 32 ?
        _get_16(e, map + 0x2e) :
        _get_32(e, map + 0x3a);
}

static int _get_header(char *map, struct header *header)
{
    int b = _get_bitness(map);
    if (b != 32 && b != 64){
        printf("Unsupported bitness: %d\n", b);
        return -1;
    }

    enum endianness e = _get_endianness(map);
    if (e < 0){
        printf("Unknown endianness: %d\n", e);
        return -1;
    }

    header->sections_offset =
        _get_sections_offset(map, e, b);
    header->section_count =
        _get_section_count(map, e, b);
    header->section_size =
        _get_section_size(map, e, b);
    header->b = b;
    header->e = e;

    return 1;
}

static int _get_section(char *map, struct header *header,
                        int index, struct section *section)
{
    map += header->sections_offset +
        (header->section_size * index);

    section->name    = _get_32(header->e, map);
    section->type    = _get_32(header->e, map + 0x04);
    section->offset  = _get_32(header->e, map + 0x10);
    section->size    = _get_32(header->e, map + 0x14);
    section->link    = _get_32(header->e, map + 0x18);
    section->entsize = _get_32(header->e, map + 0x24);

    return 1;
}

static int _get_symbol(char *map, struct header *header,
                       struct section *table, int index,
                       struct symbol *symbol)
{
    map += table->offset +
        (index * table->entsize);

    symbol->name = header->b == 32 ?
        (long)_get_32(header->e, map) :
        _get_64(header->e, map);
    return 1;
}

static char* _get_string(char *map, struct section *table, long offset)
{
    return map + table->offset + offset;
}

/* External functions */
int chili_sym_begin(const char *path, int *count)
{
    int fd = 0;
    long fdsize;
    char *map = NULL;
    int i;
    int found_dynsym = 0;

    /* Open shared library file */
    fd = open(path, O_RDONLY);
    if (fd <= 0){
        printf("Failed to open path %s: %s\n",
            path, strerror(errno));
        goto onerror;
    }

    /* Determine size */
    fdsize = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    /* Map file to memory */
    map = mmap(NULL, fdsize, PROT_READ, MAP_SHARED, fd, 0);
    if (!map){
        printf("Failed to map file to memory: %s\n",
            strerror(errno));
        goto onerror;
    }

    /* Parse header */
    if (!_is_elf(map)){
        printf("File %s is not ELF\n", path);
        goto onerror;
    }
    if (_get_header(map, &_header) < 0){
        goto onerror;
    }

    /* Try to find dynsym section */
    for (i = 0; i < _header.section_count; i++){
        if (_get_section(map, &_header, i, &_dynsym) < 0){
            printf("Failed to read section %d\n", i);
            goto onerror;
        }

        if (_dynsym.type == 11 /*SHT_DYNSYM*/){
            found_dynsym = 1;
            break;
        }
    }
    if (!found_dynsym){
        printf("Unable to find .dynsym section\n");
        goto onerror;
    }

    /* Try to find dynstr section */
    if (_get_section(map, &_header, _dynsym.link, &_dynstr) < 0){
        printf("Unable to find .dynstr section\n");
        goto onerror;
    }

    _count = _dynsym.size / _dynsym.entsize;
    _fd = fd;
    _fdsize = fdsize;
    _map = map;
    _next = 0;

    if (count){
        *count = _count;
    }

    return 1;

onerror:
    if (fd > 0){
        close(fd);
    }
    if (map){
        munmap(map, fdsize);
    }
    return -1;
}

int chili_sym_next(int index, char **name)
{
    struct symbol symbol;

    if (index < 0 || index >= _count){
        printf("Illegal symbol index: %d\n", index);
        return -1;
    }

    if (_get_symbol(_map, &_header, &_dynsym, index, &symbol) <= 0){
        return -1;
    }

    *name = _get_string(_map, &_dynstr, symbol.name);

    return 1;
}

void chili_sym_end()
{
    close(_fd);
    munmap(_map, _fdsize);
}
