#ifndef SECTION_H
#define SECTION_H

#include <gelf.h>
#include <libelf.h>
#include <stdio.h>

/**
 * Section structure containing its data, header, symbols
 */
typedef struct {
    Elf_Data *data;       ///< Section's data
    GElf_Shdr header;     ///< Section's header
    size_t symbols_count; ///< Number of symbols in the section
} section_t;

/**
 * \brief Get the specific section
 *
 * @param elf Elf structure
 * @param section Section structure
 * @param id Section id
 * @return 0 on success, -1 when not found
 */
int get_section_by_id(Elf *elf, section_t *section, unsigned id);

/**
 * \brief Print symbols in the section
 *
 * @param elf Elf structure
 * @param section Section structure
 * @return 0 on success, -1 when not found
 */
void print_section(Elf *elf, const section_t *section);

#endif // SECTION_H