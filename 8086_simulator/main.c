#include "instruction_parser/instruction_parser.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

size_t get_file_size(FILE *f)
{
    fseek(f, 0, SEEK_END);
    size_t file_size = ftell(f);
    rewind(f);
    return file_size;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <binary_file.bin>\n", argv[0]);
        printf("Error: Please provide exactly one 8086 binary file to disassemble.\n");
        return EXIT_FAILURE;
    }

    const char *file_path = argv[1];
    FILE *f               = fopen(file_path, "r");
    if (!f)
    {
        printf("Failed to open %s\n", file_path);
        return EXIT_FAILURE;
    }

    const size_t file_size = get_file_size(f);

    uint8_t *bin_buffer    = malloc(file_size);
    fread(bin_buffer, 1, file_size, f);

    InstParserErr_t err = inst_par_parse_bin(bin_buffer, file_size);
    if (err != INST_PAR_ERR_OK)
    {
        printf("Error: Failed to parse file\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
