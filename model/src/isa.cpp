
#include "isa.h"

/** Generate I-type (immediate) instruction. */
int gen_immd_instr(int opcode, int rs, int rt, int offset) {
    return (((int)opcode & 0b111111) << 26) |
        ((rs & 0b11111) << 21) |
        ((rt & 0b11111) << 16) |
        ((offset & 0xffff) << 0);
}

/** Generate J-type (jump) instruction. */
int gen_jump_instr(int opcode, int instr_idx) {
    return (((int)opcode & 0b111111) << 26) |
        ((instr_idx & 0x3ffffff) << 0);
}

/** Generate R-type (register) instruction. */
int gen_reg_instr(int opcode, int rs, int rt, int rd, int sa, int function) {
    return (((int)opcode & 0b111111) << 26) |
        ((rs & 0b11111) << 21) |
        ((rt & 0b11111) << 16) |
        ((rd & 0b11111) << 11) |
        ((sa & 0b11111) << 6) |
        ((function & 0b111111) << 0);
}
