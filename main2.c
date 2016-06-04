#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <gelf.h>
#include <libelf.h>

int main()
{
    const char *library = "/usr/lib/libtidy.so";
    int fd;

    fd = open(library, O_RDONLY);
    if (fd <= 0){
        printf("Failed to open: %s\n", strerror(errno));
        return 1;
    }

    elf_version(EV_CURRENT);

    Elf *elf;
    Elf_Kind kind;
    elf = elf_begin(fd, ELF_C_READ, NULL);
    kind = elf_kind(elf);

    if (kind != ELF_K_ELF){
        printf("Not elf\n");
    }

    size_t string_hdr_index;
    elf_getshdrstrndx(elf, &string_hdr_index);

    Elf_Scn *section = elf_nextscn(elf, NULL);
    while (section){
        section = elf_nextscn(elf, section);
        GElf_Shdr section_hdr;
        gelf_getshdr(section, &section_hdr);

        if (section_hdr.sh_type == SHT_DYNSYM){
            char *name = elf_strptr(elf, string_hdr_index, section_hdr.sh_name);
            printf("Got section at offset:%d, size: %d named: %s\n", (int)section_hdr.sh_offset, (int)section_hdr.sh_size, name);
        }
        //gelf_getsym
        //get_getsyminfo
    }

    elf_end(elf);
    close(fd);
}
