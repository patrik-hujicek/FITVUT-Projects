#include "section.h"

int get_section_by_id(Elf *elf, section_t *section, unsigned id) {
    if (elf == NULL || section == NULL) {
        return -1;
    }

    Elf_Scn *scn = NULL;

    while ((scn = elf_nextscn(elf, scn)) != NULL) {
        gelf_getshdr(scn, &section->header);
        if (section->header.sh_type == id) {
            // section found
            section->data = elf_getdata(scn, NULL);
            section->symbols_count =
                section->header.sh_size / section->header.sh_entsize;
            return 0;
        }
    }

    return -1;
}

void print_section(Elf *elf, const section_t *section) {
    if (elf == NULL || section == NULL) {
        return;
    }

    GElf_Sym symbol;
    const char *symbol_name;

    const char *elf_fmt = (gelf_getclass(elf) == ELFCLASS32)
                              ? "%08lx %u %u %lu %s\n"
                              : "%016lx %u %u %lu %s\n";

    for (unsigned i = 0; i < section->symbols_count; ++i) {
        gelf_getsym(section->data, i, &symbol);
        if (symbol.st_name == 0) {
            // symbol table entry has no name
            continue;
        }

        // get symbol name using offset to strings section
        symbol_name = elf_strptr(elf, section->header.sh_link, symbol.st_name);

        if (!symbol_name) {
            // cannot retrieve name of symbol
            continue;
        }

        printf(elf_fmt, symbol.st_value, ELF32_ST_BIND(symbol.st_info),
               ELF32_ST_TYPE(symbol.st_info), symbol.st_size, symbol_name);
    }
}