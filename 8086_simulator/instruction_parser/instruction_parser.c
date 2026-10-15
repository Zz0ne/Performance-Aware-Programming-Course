#include "instruction_parser.h"

#include <stdint.h>
#include <stdio.h>

#define INST_MAX_SIZE_      30
#define MNEMONIC_MAX_SIZE_  8
#define MAX_8088_FILE_SIZE_ 1000000
#define INST_TABLE_SIZE_    63
#define INST_CODE_MOV_      0x22

// AI generated secction - begin
typedef enum
{
    FORMAT_UNKNOWN = 0,       // Default for uninitialized slots
    FORMAT_REG_MEM_TO_REG,    // Handles 0x88-0x8B (MOV), 0x00-0x03 (ADD), etc.
    FORMAT_IMM_TO_REG,        // Handles 0xB0-0xBF (MOV), etc.
    FORMAT_IMM_TO_REG_MEM,    // Handles 0xC6-0xC7 (MOV), 0x80-0x83 (Math), etc.
    FORMAT_MEM_TO_ACCUM,      // Handles 0xA0-0xA3 (MOV), 0x04-0x05 (ADD), etc.
    FORMAT_SEG_REG_TO_REG_MEM // Handles 0x8C, 0x8E (MOV segment regs)
} inst_format_e;

typedef struct
{
    const char *mnemonic;
    inst_format_e format;
} fast_lookup_t;

static const fast_lookup_t opcode_table[256] = {
    [0x88] = {"mov", FORMAT_REG_MEM_TO_REG}, // r/m8, r8
    [0x89] = {"mov", FORMAT_REG_MEM_TO_REG}, // r/m16, r16
    [0x8A] = {"mov", FORMAT_REG_MEM_TO_REG}, // r8, r/m8
    [0x8B] = {"mov", FORMAT_REG_MEM_TO_REG}, // r16, r/m16

    // 8-bit registers (AL, CL, DL, BL, AH, CH, DH, BH)
    [0xB0] = {"mov", FORMAT_IMM_TO_REG},
    [0xB1] = {"mov", FORMAT_IMM_TO_REG},
    [0xB2] = {"mov", FORMAT_IMM_TO_REG},
    [0xB3] = {"mov", FORMAT_IMM_TO_REG},
    [0xB4] = {"mov", FORMAT_IMM_TO_REG},
    [0xB5] = {"mov", FORMAT_IMM_TO_REG},
    [0xB6] = {"mov", FORMAT_IMM_TO_REG},
    [0xB7] = {"mov", FORMAT_IMM_TO_REG},
    // 16-bit registers (AX, CX, DX, BX, SP, BP, SI, DI)
    [0xB8] = {"mov", FORMAT_IMM_TO_REG},
    [0xB9] = {"mov", FORMAT_IMM_TO_REG},
    [0xBA] = {"mov", FORMAT_IMM_TO_REG},
    [0xBB] = {"mov", FORMAT_IMM_TO_REG},
    [0xBC] = {"mov", FORMAT_IMM_TO_REG},
    [0xBD] = {"mov", FORMAT_IMM_TO_REG},
    [0xBE] = {"mov", FORMAT_IMM_TO_REG},
    [0xBF] = {"mov", FORMAT_IMM_TO_REG},

    [0xC6] = {"mov", FORMAT_IMM_TO_REG_MEM}, // r/m8, imm8
    [0xC7] = {"mov", FORMAT_IMM_TO_REG_MEM}, // r/m16, imm16

    [0xA0] = {"mov", FORMAT_MEM_TO_ACCUM}, // AL, moffs8
    [0xA1] = {"mov", FORMAT_MEM_TO_ACCUM}, // AX, moffs16
    [0xA2] = {"mov", FORMAT_MEM_TO_ACCUM}, // moffs8, AL
    [0xA3] = {"mov", FORMAT_MEM_TO_ACCUM}, // moffs16, AX

    [0x8C] = {"mov", FORMAT_SEG_REG_TO_REG_MEM}, // r/m16, Sreg
    [0x8E] = {"mov", FORMAT_SEG_REG_TO_REG_MEM}, // Sreg, r/m16
};

static const char *register_table[2][8] = {
    // W = 0 (8-bit registers)
    [0] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"},
    // W = 1 (16-bit registers)
    [1] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"},
};

static const char *effective_address_table[8] = {
    "bx + si", // 000
    "bx + di", // 001
    "bp + si", // 010
    "bp + di", // 011
    "si",      // 100
    "di",      // 101
    "bp",      // 110 (Special case for direct address if MOD == 0)
    "bx"       // 111
};

// AI generated secction - end

static size_t parse_reg_mem_to_reg_(uint8_t *stream, const char *mnemonic_str, char *inst_buffer,
                                    size_t buffer_size)
{
    size_t inst_size    = 0;
    uint8_t msb         = stream[0];
    uint8_t lsb         = stream[1];

    uint8_t d           = (msb & 0b00000010) >> 1;
    uint8_t w           = msb & 1;

    uint8_t mod         = (lsb & 0b11000000) >> 6;
    uint8_t reg         = (lsb & 0b00111000) >> 3;
    uint8_t r_m         = lsb & 0b00000111;

    const char *reg_str = NULL;
    const char *r_m_str = NULL;

    switch (mod)
    {
    case 0:
        reg_str   = register_table[w][reg];
        r_m_str   = effective_address_table[r_m];
        inst_size = 2;
        break;
    case 1:
        inst_size = 3;
        break;
    case 2:
        inst_size = 4;
        break;
    case 3:
        reg_str   = register_table[w][reg];
        r_m_str   = register_table[w][r_m];
        inst_size = 2;
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
        if (mod == 3)
        {
            snprintf(inst_buffer, buffer_size, "%s %s, %s", mnemonic_str, mem_str_1, mem_str_2);
        }
        else
        {
            snprintf(inst_buffer, buffer_size, "%s %s, [%s]", mnemonic_str, mem_str_1, mem_str_2);
        }
    else
        snprintf(inst_buffer, buffer_size, "%s (memory operands not yet implemented)",
                 mnemonic_str);
    return inst_size;
}

static size_t parse_imm_to_reg(uint8_t *stream, const char *mnemonic_str, char *inst_buffer,
                               size_t buffer_size)
{
    size_t inst_size    = 0;
    uint8_t msb         = stream[0];

    uint8_t w           = (msb & 0b00001000) >> 3;
    uint8_t reg         = msb & 0b00000111;
    const char *reg_str = register_table[w][reg];

    int16_t data        = 0;
    if (w == 1)
    {
        data      = (stream[2] << 8) | stream[1];
        inst_size = 3;
    }
    else
    {
        data      = (int8_t)stream[1];
        inst_size = 2;
    }

    snprintf(inst_buffer, buffer_size, "%s %s, %d", mnemonic_str, reg_str, data);

    return inst_size;
}

static size_t decode_(uint8_t *stream, char *inst_buffer, size_t buffer_size)
{
    uint8_t msb              = stream[0];

    const char *mnemonic_str = opcode_table[msb].mnemonic;
    inst_format_e format     = opcode_table[msb].format;

    switch (format)
    {
    case FORMAT_UNKNOWN:
        printf("Error: Unknown or unsupported instruction format.\n");
        return 0;

    case FORMAT_REG_MEM_TO_REG:
        return parse_reg_mem_to_reg_(stream, mnemonic_str, inst_buffer, buffer_size);

    case FORMAT_IMM_TO_REG:
        return parse_imm_to_reg(stream, mnemonic_str, inst_buffer, buffer_size);

    case FORMAT_IMM_TO_REG_MEM:
        // 0xC6 - 0xC7: Immediate value to Register or Memory
        break;

    case FORMAT_MEM_TO_ACCUM:
        // 0xA0 - 0xA3: Direct memory offset to/from Accumulator (AL/AX)
        break;

    case FORMAT_SEG_REG_TO_REG_MEM:
        // 0x8C, 0x8E: Segment Register to/from Register or Memory
        break;

    default:
        printf("Error: Invalid instruction format enum.\n");
        return 0;
    }

    return 0;
}

InstParserErr_t inst_par_parse_bin(uint8_t *bin, size_t bin_size)
{
    if (bin_size > MAX_8088_FILE_SIZE_)
        return INST_PAR_ERR_INVALID_FILE;

    char inst_buffer[INST_MAX_SIZE_];

    size_t bin_index = 0;
    while (bin_size > bin_index)
    {
        bin_index += decode_(&bin[bin_index], inst_buffer, INST_MAX_SIZE_);
        printf("%s\n", inst_buffer);
    }

    return INST_PAR_ERR_OK;
}
