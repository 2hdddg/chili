#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <elf.h>

enum endianness {
    big_endian,
    little_endian
};

struct section_header {
    int type;
    int offset;
};

static int _read(int fd, int offset, char* buffer, int size)
{
    int result;

    result = lseek(fd, offset, SEEK_SET);
    if (result < 0){
        printf("Error seeking to %d\n", 0);
        return -1;
    }

    result = read(fd, buffer, size);
    if (result < size && result >= 0){
        printf("Failed to read %d bytes, got only %d\n", size, result);
        return -1;
    }
    else if (result < 0){
        printf("Error during read: %s\n", strerror(errno));
        return -1;
    }

    return 1;
}


static int _is_elf(int fd)
{
    int result;
    char signature[4];

    result = _read(fd, 0, signature, sizeof(signature));
    if (result < 0){
        return result;
    }

    return
        signature[0] == 0x7f &&
        signature[1] == 'E' &&
        signature[2] == 'L' &&
        signature[3] == 'F';
}

static int _get_bitness(int fd)
{
    int result;
    char c;

    result = _read(fd, 4, &c, 1);
    if (result < 0){
        return result;
    }

    switch (c){
    case 1:
        return 32;
    case 2:
        return 64;
    default:
        printf("Unknown bitness: %d\n", c);
        return -1;
    }
}

static enum endianness _get_endianness(int fd)
{
    int result;
    char c;

    result = _read(fd, 5, &c, 1);
    if (result < 0){
        return result;
    }

    switch (c){
    case 1:
        return little_endian;
    case 2:
        return big_endian;
    default:
        printf("Unknown endianness: %d\n", c);
        return -1;
    }
}

static int get_16(enum endianness e, char *buf)
{
    if (e == little_endian){
        return (buf[1] << 8) | buf[0];
    }

    printf("Get int16 on unsupported combo\n");
    return -1;
}

static int get_32(enum endianness e, char *buf)
{
    if (e == little_endian){
        return (buf[3] << 24) | buf[2] << 16 | buf[1] << 8 | buf[0];
    }

    printf("Get int32 on unsupported combo\n");
    return -1;
}
static long get_offset(enum endianness e, int bitness, char *buf)
{
    if (bitness == 32){
        return get_32(e, buf);
    }

    printf("Get offset on unsupported combo\n");
    return -1;
}


static int get_size(enum endianness e, int bitness, char *buf)
{
    if (bitness == 32){
        return get_16(e, buf);
    }
    return -1;
}

static long _get_section_header_offset(int fd, enum endianness e, int bitness)
{
    char buff[8];
    int result;

// 161e0 5324c
    result = bitness == 32 ?
        _read(fd, 0x20, buff, 4) :
        _read(fd, 0x28, buff, 8);
    if (result < 0){
        return result;
    }

    return get_offset(e, bitness, buff);
}

static long _get_program_header_offset(int fd, enum endianness e, int bitness)
{
    char buff[8];
    int result;

    result = bitness == 32 ?
        _read(fd, 0x1c, buff, 4) :
        _read(fd, 0x20, buff, 8);
    if (result < 0){
        return result;
    }

    return get_offset(e, bitness, buff);
}

static int _get_num_program_headers(int fd, enum endianness e, int bitness)
{
    char buf[2];
    int result;

    result = bitness == 32 ?
        _read(fd, 0x2c, buf, 2) :
        _read(fd, 0x38, buf, 2);
    if (result < 0){
        return result;
    }
    return get_size(e, bitness, buf);
}

static int _get_num_section_headers(int fd, enum endianness e, int bitness)
{
    char buf[2];
    int result;

    result = bitness == 32 ?
        _read(fd, 0x30, buf, 2) :
        _read(fd, 0x3c, buf, 2);
    if (result < 0){
        return result;
    }
    return get_size(e, bitness, buf);
}

static int _get_section_header(int fd, enum endianness e, int b, int offset, int index, struct section_header *header)
{
    char buf[4];
    int result;
    int size = b == 32 ?
        sizeof(Elf32_Shdr) : sizeof(Elf64_Shdr);
    offset += size * index;
    result = _read(fd, offset + 4, buf, 4);
    if (result < 0){
        return result;
    }

    header->type = get_32(e, buf);
}

int main()
{
    const char *library = "/usr/lib/libtidy.so";
    int fd;

    fd = open(library, O_RDONLY);
    if (fd <= 0){
        printf("Failed to open: %s\n", strerror(errno));
        return 1;
    }
    printf("Opened %s\n", library);

    if (!_is_elf(fd)){
        printf("Not ELF!\n");
        return 1;
    }

    int bitness = _get_bitness(fd);
    if (bitness < 0){
        return 1;
    }
    printf("ELF of %d bits\n", bitness);

    enum endianness endianness = _get_endianness(fd);
    if (endianness < 0){
        return 1;
    }
    printf("Endianness is %s\n", endianness == big_endian ? "big" : "little");

    long program_header_offset = _get_program_header_offset(fd, endianness, bitness);
    printf("Program header starts at %ld\n", program_header_offset);

    int num_program_headers = _get_num_program_headers(fd, endianness, bitness);
    printf("Number of program headers: %d\n", num_program_headers);

    long section_header_offset = _get_section_header_offset(fd, endianness, bitness);
    printf("Section table starts at %ld\n", section_header_offset);

    int num_section_headers = _get_num_section_headers(fd, endianness, bitness);
    printf("Number of section headers: %d\n", num_section_headers);

    int i;
    struct section_header section_hdr;
    int found = 0;

    for (i = 0; i < num_section_headers; i++){
        _get_section_header(fd, endianness, bitness, section_header_offset, i, &section_hdr);
        if (section_hdr.type == SHT_DYNSYM){
            printf("Found dyn sym section");
            found = 1;
            break;
        }
    }

    if (!found){
        printf("Unable to find dyn sym section\n");
        exit(1);
    }


    close(fd);

    return 0;
}
