#pragma once

#include <stddef.h>
#include <stdint.h>

typedef enum
{
    INST_PAR_ERR_OK,
    INST_PAR_ERR_INVALID_FILE,
    INST_PAR_ERR_FAIL
} InstParserErr_t;

InstParserErr_t inst_par_parse_bin(uint8_t *bin, size_t bin_size);
