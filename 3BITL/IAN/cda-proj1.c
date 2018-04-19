// Binary Code Analysis
// Project 1: Simple nm-like tool
// Author: David Bolvansky (xbolva00)

#include <fcntl.h>
#include <gelf.h>
#include <libelf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "cda-proj1.h"
#include "section.h"

int parse_args(int argc, char **argv, int *show_dynamic_symbols,
               const char **filename) {
    // parse arguments
    if (argc == 1) {
        // no file name entered, try "a.out"
        *filename = DEFAULT_BINARY_FILE;
    } else if (argc == 2) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            // print help
            puts(USAGE);
            exit(no_err);
        } else if (strcmp(argv[1], "-D") == 0 ||
                   strcmp(argv[1], "--dynamic") == 0) {
            // option detected, but no file name entered, try "a.out"
            *filename = DEFAULT_BINARY_FILE;
            *show_dynamic_symbols = 1;
        } else {
            // file name entered, no options detected
            *filename = argv[1];
        }
    } else if (argc > 2) {
        if (strcmp(argv[2], "-D") == 0 || strcmp(argv[2], "--dynamic") == 0) {
            // option is used after file name
            *filename = argv[1];
            *show_dynamic_symbols = 1;
        } else if (strcmp(argv[1], "-D") == 0 ||
                   strcmp(argv[1], "--dynamic") == 0) {
            // option is used before file name
            *filename = argv[2];
            *show_dynamic_symbols = 1;
        } else {
            return args_err;
        }
    }

    return no_err;
}

int main(int argc, char **argv) {
    Elf *elf = NULL;
    int err = no_err, fd = -1;
    int show_dynamic_symbols = 0;
    const char *filename;

    // parse arguments
    err = parse_args(argc, argv, &show_dynamic_symbols, &filename);
    if (err) {
        fprintf(stderr, "Unknown option. Only -D / --dynamic is supported.\n%s",
                USAGE);
        goto end;
    }

    // open specified file
    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        err = open_err;
        fprintf(stderr, "Cannot open file \'%s\'\n", filename);
        goto end;
    }

    // open elf
    elf_version(EV_CURRENT);
    elf = elf_begin(fd, ELF_C_READ, NULL);
    if (!elf) {
        err = elf_err;
        fprintf(stderr, "Cannot open ELF file \'%s\'\n", filename);
        goto end;
    }

    // get elf class, check for errors
    if (gelf_getclass(elf) == ELFCLASSNONE) {
        fprintf(stderr, "Invalid ELF file \'%s\'\n", filename);
        goto end;
    }

    // find section
    section_t obj_section;
    err = get_section_by_id(elf, &obj_section,
                            show_dynamic_symbols ? SHT_DYNSYM : SHT_SYMTAB);
    if (err) {
        err = section_err;
        fprintf(stderr, "Cannot find %s section.\n",
                show_dynamic_symbols ? "DYNSYM" : "SYMTAB");
        goto end;
    }

    // print header
    puts(OUTPUT_HEADER);

    // print symbols in the section
    print_section(elf, &obj_section);

end:
    elf_end(elf);
    close(fd);
    return err;
}
