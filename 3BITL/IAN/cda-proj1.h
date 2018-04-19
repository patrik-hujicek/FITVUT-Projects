#ifndef CDA_PROJ1_H
#define CDA_PROJ1_H

#include <gelf.h>
#include <libelf.h>

const char *USAGE =
    "USAGE: cda-proj1 [file] [-D|--dynamic]\n\nfile: object file "
    "or executable name, default is \'a.out\'\n-D | --dynamic: display the "
    "dynamic "
    "symbols";                                           ///< Program's usage
const char *OUTPUT_HEADER = "value bind type size name"; ///< Output header
const char *DEFAULT_BINARY_FILE =
    "a.out"; ///< Defualt binary file when no arguments

/**
 * Errors
 */
enum { no_err, args_err, open_err, elf_err, section_err };

/**
 * \brief Parse arguments
 *
 * @param argc number of arguments
 * @param argv array of arguments
 * @param show_dynamic_symbols show dynamic symbols from DYNSYM section
 * @param filename object or executable file name
 * @return no_err on parsing success, args_err when failure happens
 */
int parse_args(int argc, char **argv, int *show_dynamic_symbols,
               const char **filename);

#endif // CDA_PROJ1
