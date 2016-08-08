#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdint.h>

#include "symbols.h"

/* Debugging */
#define DEBUG 0
#include "debug.h"

/* Types */
enum endianness {
    big_endian,
    little_endian
};

struct header {
    char            b;
    enum endianness e;
    uint64_t        sections_offset;
    uint16_t        section_size;
    uint16_t        section_count;
};

struct section {
    uint64_t name;
    uint64_t type;
    uint64_t offset;
    uint64_t size;
    uint64_t entsize;
    uint64_t link;
};

struct symbol {
    uint64_t name;
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
static uint16_t _get_16(enum endianness e, char *buf)
{
    unsigned char c1 = *buf++;
    unsigned char c2 = *buf;

    return e == little_endian ?
        c2 << 8 | c1 :
        -1;
}

static uint32_t _get_32(enum endianness e, char *buf)
{
    uint32_t w1 = _get_16(e, buf);
    uint32_t w2 = _get_16(e, buf + 2);

    return e == little_endian ?
        w2 << 16 | w1 :
        -1;
}

static uint64_t _get_64(enum endianness e, char *buf)
{
    uint64_t i1 = _get_32(e, buf);
    uint64_t i2 = _get_32(e, buf + 4);

    return e == little_endian ?
        i2 << 32 | i1 :
        -1;
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
    switch (map[4]){
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
    switch (map[5]){
    case 1:
        return little_endian;
    case 2:
        return big_endian;
    default:
        return -1;
    }
}

static uint64_t _get_sections_offset(char *map, char b, enum endianness e)
{
    return b == 32 ?
        _get_32(e, map + 0x20) :
        _get_64(e, map + 0x28);
}

static uint16_t _get_section_count(char *map, char b, enum endianness e)
{
    return b == 32 ?
        _get_16(e, map + 0x30) :
        _get_16(e, map + 0x3c);
}

static uint16_t _get_section_size(char *map, char b, enum endianness e)
{
    return b == 32 ?
        _get_16(e, map + 0x2e) :
        _get_16(e, map + 0x3a);
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

    header->e= e;
    header->b= b;
    header->sections_offset =
        _get_sections_offset(map, b, e);
    header->section_count =
        _get_section_count(map, b, e);
    header->section_size =
        _get_section_size(map, b, e);

    return 1;
}

static int _get_section(char *map, struct header *header,
                        int index, struct section *section)
{
    char b = header->b;
    enum endianness e = header->e;

    map += header->sections_offset +
        (header->section_size * index);

    if (b == 32){
        section->name    = _get_32(e, map);
        section->type    = _get_32(e, map + 0x04);
        section->offset  = _get_32(e, map + 0x10);
        section->size    = _get_32(e, map + 0x14);
        section->link    = _get_32(e, map + 0x18);
        section->entsize = _get_32(e, map + 0x24);
    }
    else{
        section->name    = _get_32(e, map);
        section->type    = _get_32(e, map + 0x04);
        /* Flags         = _get_64(e, map + 0x08)*/
        /* Address       = _get_64(e, map + 0x10)*/
        section->offset  = _get_64(e, map + 0x18);
        section->size    = _get_64(e, map + 0x20);
        section->link    = _get_32(e, map + 0x28);
        /*sh_info        = _get_32(30) */
        /*sh_addrallign  = _get_64(34) */
        section->entsize = _get_64(e, map + 0x38);
    }

    return 1;
}

static uint64_t _get_symbol(char *map, struct header *header,
                       struct section *table, int index,
                       struct symbol *symbol)
{
    map += table->offset +
        (index * table->entsize);

    symbol->name = header->b == 32 ?
        _get_32(header->e, map) :
        _get_32(header->e, map);
    return 1;
}

static char* _get_string(char *map, struct section *table,
                         uint64_t offset)
{
    return map + table->offset + offset;
}

static void debug_print_header(struct header *h)
{
    debug_print("Parsed header:\n"
                "\tsections_offset: %lu\n"
                "\tsection_size: %u\n"
                "\tsection_count: %u\n",
                h->sections_offset,
                h->section_size,
                h->section_count);
}

static void debug_print_section(struct section *s, const char *n)
{
    debug_print("Parsed section %s:\n"
                "\tname: %lu\n"
                "\ttype: %lu\n"
                "\toffset: %lu\n"
                "\tsize: %lu\n"
                "\tentsize: %lu\n"
                "\tlink: %lu\n",
                n,
                s->name,
                s->type,
                s->offset,
                s->size,
                s->entsize,
                s->link);
}

static void debug_print_symbol(struct symbol *s, int i)
{
    debug_print("Parsed symbol %d:\n"
                "\tname: %lu\n", i, s->name);
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
    debug_print_header(&_header);

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
    debug_print_section(&_dynsym, "dynsym");

    /* Try to find dynstr section */
    if (_get_section(map, &_header, _dynsym.link,
                     &_dynstr) < 0){
        printf("Unable to find .dynstr section\n");
        goto onerror;
    }
    debug_print_section(&_dynstr, "dynstr");

    _count = _dynsym.size / _dynsym.entsize;
    _fd = fd;
    _fdsize = fdsize;
    _map = map;
    _next = 0;

    if (count){
        *count = _count;
        debug_print("ELF contains %d symbols\n", _count);
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

int chili_sym_next(char **name)
{
    struct symbol symbol;

    /* End of symbols */
    if (_next >= _count){
        return 0;
    }

    if (_get_symbol(_map, &_header, &_dynsym, _next,
                    &symbol) <= 0){
        printf("Failed to retrieve symbol\n");
        return -1;
    }
    debug_print_symbol(&symbol, _next);

    *name = _get_string(_map, &_dynstr, symbol.name);

    _next++;

    return 1;
}

void chili_sym_end()
{
    close(_fd);
    munmap(_map, _fdsize);
}
