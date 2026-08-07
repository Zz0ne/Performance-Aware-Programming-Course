#include "instruction_parser.h"

#include <stdint.h>
#include <stdio.h>

#define INST_MAX_SIZE_      30
#define MNEMONIC_MAX_SIZE_  8
#define MAX_8088_FILE_SIZE_ 1000000
#define INST_TABLE_SIZE_    63
#define INST_CODE_MOV_      0x22

#define MNEMONIC_MASK       0xFC
#define D_MASK              0x02
#define W_MASK              0x01
#define MOD_MASK            0xC0
#define REG_MASK            0x38
#define R_M_MASK            0x07

typedef char instruction_t[MNEMONIC_MAX_SIZE_];

static instruction_t instruction_table[63] = {
    [INST_CODE_MOV_] = "mov",
};

static const char *register_table[2][8] = {
    // W = 0 (8-bit registers)
    [0] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"},
    // W = 1 (16-bit registers)
    [1] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"},
};

static void parse_mov_instruction_(uint16_t inst, char *inst_buffer, size_t buffer_size)
{
    uint8_t msb         = (inst >> 8);
    uint8_t lsb         = (inst & 0x00FF);

    uint8_t mnemonic    = (msb & MNEMONIC_MASK) >> 2;
    uint8_t d           = (msb & D_MASK) >> 1;
    uint8_t w           = msb & W_MASK;

    uint8_t mod         = (lsb & MOD_MASK) >> 6;
    uint8_t reg         = (lsb & REG_MASK) >> 3;
    uint8_t r_m         = lsb & R_M_MASK;

    char *mnemonic_str  = instruction_table[mnemonic];
    const char *reg_str = NULL;
    const char *r_m_str = NULL;

    if (mnemonic_str[0] == '\0')
    {
        snprintf(inst_buffer, buffer_size, "instruction not supported");
        return;
    }

    switch (mod)
    {
    case 0:
    case 1:
    case 2:
        break;
    case 3:
        reg_str = register_table[w][reg];
        r_m_str = register_table[w][r_m];
        break;
    default:
        printf("Error: invalid mod value\n");
        break;
    }

    const char *mem_str_1;
    const char *mem_str_2;
    if (d == 1)
    {
        mem_str_1 = reg_str;
        mem_str_2 = r_m_str;
    }
    else
    {
        mem_str_1 = r_m_str;
        mem_str_2 = reg_str;
    }

    if (reg_str != NULL && r_m_str != NULL)
        snprintf(inst_buffer, buffer_size, "%s %s, %s", mnemonic_str, mem_str_1, mem_str_2);
    else
        snprintf(inst_buffer, buffer_size, "%s (memory operands not yet implemented)",
                 mnemonic_str);
}

InstParserErr_t inst_par_parse_bin(uint8_t *bin, size_t bin_size)
{
    if (bin_size > MAX_8088_FILE_SIZE_)
        return INST_PAR_ERR_INVALID_FILE;

    char inst_buffer[INST_MAX_SIZE_];

    for (size_t i = 0; i < bin_size; i += 2)
    {
        // little endian
        uint16_t inst = (bin[i] << 8) | bin[i + 1];
        parse_mov_instruction_(inst, inst_buffer, INST_MAX_SIZE_);

        printf("%s\n", inst_buffer);
    }
    return INST_PAR_ERR_OK;
}
