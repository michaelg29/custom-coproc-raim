
// get bits of the instruction [a:n_bits(mask)]
#define GET_INSTR_BITS(instr, a, mask) ((instr >> a) & mask)
// get opcode of the instruction (bits [26:31])
#define GET_INSTR_OPCODE(instr) GET_INSTR_BITS(instr, 26, 0b111111)
// get five-bit identifier for a register starting from bit a (bits [a:a+4])
#define GET_INSTR_REG(instr, a) GET_INSTR_BITS(instr, a, 0b11111)
// get the operation of a special instruction
#define GET_INSTR_SPECIAL_OP(instr) GET_INSTR_BITS(instr, 0, 0b111111)

// Instruction opcodes.
enum opcode_e {
    OPCODE_SPECIAL = 0b000000,
    OPCODE_ADDI    = 0b001000,
    OPCODE_ADDIU   = 0b001001,
    OPCODE_ANDI    = 0b001100,
    OPCODE_BEQ     = 0b000100,
    OPCODE_BEQL    = 0b010100,
    OPCODE_REGIMM  = 0b000001,
    OPCODE_BGTZ    = 0b000111,
    OPCODE_BGTZL   = 0b010111,
    OPCODE_BLEZ    = 0b000110,
    OPCODE_BLEZL   = 0b010110,
    OPCODE_BNE     = 0b000101,
    OPCODE_BNEL    = 0b010101,
    OPCODE_COP0    = 0b010000,
    OPCODE_COP1    = 0b010001,
    OPCODE_COP2    = 0b010010,
    OPCODE_COP3    = 0b010011,
    OPCODE_DADDI   = 0b011000, // not supported
    OPCODE_DADDIU  = 0b011001, // not supported
    OPCODE_J       = 0b000010,
};

// Operations of the SPECIAL opcode.
enum special_op_e {
    SPECIAL_ADD    = 0b100000,
    SPECIAL_ADDU   = 0b100001,
    SPECIAL_AND    = 0b100100,
    SPECIAL_BREAK  = 0b001101,
    SPECIAL_DADD   = 0b101100, // not supported
    SPECIAL_DADDU  = 0b101101, // not supported
    SPECIAL_DDIV   = 0b011110, // not supported
    SPECIAL_DDIVU  = 0b011111, // not supported
    SPECIAL_DIV    = 0b011010,
    SPECIAL_DIV    = 0b011011,
    SPECIAL_DMULT  = 0b011100, // not supported
    SPECIAL_DMULTU = 0b011101, // not supported
    SPECIAL_DSLL   = 0b111000, // not supported
    SPECIAL_DSLL32 = 0b111100, // not supported
    SPECIAL_DSLLV  = 0b010100, // not supported
    SPECIAL_DSRA   = 0b111011, // not supported
    SPECIAL_DSRA32 = 0b111111, // not supported
    SPECIAL_DSRAV  = 0b010111, // not supported
    SPECIAL_DSRL   = 0b111010, // not supported
    SPECIAL_DSRL32 = 0b111110, // not supported
    SPECIAL_DSRLV  = 0b010110, // not supported
    SPECIAL_DSUB   = 0b101110, // not supported
    SPECIAL_DSUBU  = 0b101111, // not supported
    SPECIAL_JALR   = 0b001001,
    SPECIAL_JR     = 0b001000,
};

// Operations of the REGIMM opcode.
enum regimm_op_e {
    REGIMM_BGEZ    = 0b00001,
    REGIMM_BGEZAL  = 0b10001,
    REGIMM_BGEZALL = 0b10011,
    REGIMM_BGEZL   = 0b00011,
    REGIMM_BLTZ    = 0b00000,
    REGIMM_BLTZAL  = 0b10000,
    REGIMM_BLTZALL = 0b10010,
    REGIMM_BLTZL   = 0b00010,
};
