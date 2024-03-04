
#ifndef ISA_H
#define ISA_H

// get bits of the instruction [a:n_bits(mask)]
#define GET_INSTR_BITS(instr, a, mask) ((instr >> a) & mask)
// get opcode of the instruction (bits [26:31])
#define GET_INSTR_OPCODE(instr) GET_INSTR_BITS(instr, 26, 0b111111)
// get five-bit identifier for a register starting from bit a (bits [a:a+4])
#define GET_INSTR_REG(instr, a) GET_INSTR_BITS(instr, a, 0b11111)
// get the operation of a special instruction
#define GET_INSTR_SPECIAL_OP(instr) GET_INSTR_BITS(instr, 0, 0b111111)

// Possible exceptions.
enum exception_e {
    EX_NONE         = 0x00000000,
    EX_INVALID      = 0x00000001,
    EX_NOIMP        = 0x00000002,
    EX_TRAP         = 0x00000004,
    EX_ADDRESS      = 0x00000008,
};

// Instruction opcodes.
enum opcode_e {
    OPCODE_ADDI     = 0b001000,
    OPCODE_SPECIAL  = 0b000000,
    OPCODE_ADDIU    = 0b001001,
    OPCODE_ANDI     = 0b001100,
    OPCODE_BEQ      = 0b000100,
    OPCODE_BEQL     = 0b010100,
    OPCODE_REGIMM   = 0b000001,
    OPCODE_BGTZ     = 0b000111,
    OPCODE_BGTZL    = 0b010111,
    OPCODE_BLEZ     = 0b000110,
    OPCODE_BLEZL    = 0b010110,
    OPCODE_BNE      = 0b000101,
    OPCODE_BNEL     = 0b010101,
    OPCODE_COP0     = 0b010000,
    OPCODE_COP1     = 0b010001,
    OPCODE_COP2     = 0b010010,
    OPCODE_COP3     = 0b010011,
    OPCODE_DADDI    = 0b011000, // not supported
    OPCODE_DADDIU   = 0b011001, // not supported
    OPCODE_J        = 0b000010,
    OPCODE_JAL      = 0b000011,
    OPCODE_LB       = 0b100000,
    OPCODE_LBU      = 0b100100,
    OPCODE_LD       = 0b110111, // not supported
    OPCODE_LDC1     = 0b110101, // not supported
    OPCODE_LDC2     = 0b110110, // not supported
    OPCODE_LDL      = 0b011010, // not supported
    OPCODE_LDR      = 0b011011, // not supported
    OPCODE_LH       = 0b100001,
    OPCODE_LHU      = 0b100101,
    OPCODE_LL       = 0b110000, // not supported
    OPCODE_LLD      = 0b110100, // not supported
    OPCODE_LUI      = 0b001111,
    OPCODE_LW       = 0b100011,
    OPCODE_LWC1     = 0b110001,
    OPCODE_LWC2     = 0b110010,
    OPCODE_LWC3     = 0b110011,
    OPCODE_LWL      = 0b100010, // not supported
    OPCODE_LWR      = 0b100110, // not supported
    OPCODE_LWU      = 0b100111,
    OPCODE_ORI      = 0b001101,
    OPCODE_PREF     = 0b110011, // not supported
    OPCODE_SB       = 0b101000,
    OPCODE_SC       = 0b111000, // not supported
    OPCODE_SCD      = 0b111100, // not supported
    OPCODE_SD       = 0b111111, // not supported
    OPCODE_SDC1     = 0b111101, // not supported
    OPCODE_SDC2     = 0b111110, // not supported
    OPCODE_SDL      = 0b101100, // not supported
    OPCODE_SDR      = 0b101101, // not supported
    OPCODE_SH       = 0b101001,
    OPCODE_SLTI     = 0b001010,
    OPCODE_SLTIU    = 0b001011,
    OPCODE_SW       = 0b101011,
    OPCODE_SWC1     = 0b111001,
    OPCODE_SWC2     = 0b111010,
    OPCODE_SWC3     = 0b111011,
    OPCODE_SWL      = 0b101010, // not supported
    OPCODE_SWR      = 0b101110, // not supported
    OPCODE_XORI     = 0b001110,
};

// Operations of the SPECIAL opcode.
enum special_op_e {
    SPECIAL_ADD     = 0b100000,
    SPECIAL_ADDU    = 0b100001,
    SPECIAL_AND     = 0b100100,
    SPECIAL_BREAK   = 0b001101,
    SPECIAL_DADD    = 0b101100, // not supported
    SPECIAL_DADDU   = 0b101101, // not supported
    SPECIAL_DDIV    = 0b011110, // not supported
    SPECIAL_DDIVU   = 0b011111, // not supported
    SPECIAL_DIV     = 0b011010,
    SPECIAL_DIVU    = 0b011011,
    SPECIAL_DMULT   = 0b011100, // not supported
    SPECIAL_DMULTU  = 0b011101, // not supported
    SPECIAL_DSLL    = 0b111000, // not supported
    SPECIAL_DSLL32  = 0b111100, // not supported
    SPECIAL_DSLLV   = 0b010100, // not supported
    SPECIAL_DSRA    = 0b111011, // not supported
    SPECIAL_DSRA32  = 0b111111, // not supported
    SPECIAL_DSRAV   = 0b010111, // not supported
    SPECIAL_DSRL    = 0b111010, // not supported
    SPECIAL_DSRL32  = 0b111110, // not supported
    SPECIAL_DSRLV   = 0b010110, // not supported
    SPECIAL_DSUB    = 0b101110, // not supported
    SPECIAL_DSUBU   = 0b101111, // not supported
    SPECIAL_JALR    = 0b001001,
    SPECIAL_JR      = 0b001000,
    SPECIAL_MFHI    = 0b010000,
    SPECIAL_MFLO    = 0b010010,
    SPECIAL_MOVN    = 0b001011,
    SPECIAL_MOVZ    = 0b001010,
    SPECIAL_MTHI    = 0b010001,
    SPECIAL_MTLO    = 0b010011,
    SPECIAL_MULT    = 0b011000,
    SPECIAL_MULTU   = 0b011001,
    SPECIAL_NOR     = 0b100111,
    SPECIAL_OR      = 0b100101,
    SPECIAL_SLL     = 0b000000,
    SPECIAL_SLLV    = 0b000100,
    SPECIAL_SLT     = 0b101010,
    SPECIAL_SLTU    = 0b101011,
    SPECIAL_SRA     = 0b000011,
    SPECIAL_SRAV    = 0b000111,
    SPECIAL_SRL     = 0b000010,
    SPECIAL_SRLV    = 0b000110,
    SPECIAL_SUB     = 0b100010,
    SPECIAL_SUBU    = 0b100011,
    SPECIAL_SYNC    = 0b001111, // not supported
    SPECIAL_SYSCALL = 0b001100,
    SPECIAL_TEQ     = 0b110100,
    SPECIAL_TGE     = 0b110000,
    SPECIAL_TGEU    = 0b110001,
    SPECIAL_TLT     = 0b110010,
    SPECIAL_TLTU    = 0b110011,
    SPECIAL_TNE     = 0b110110,
    SPECIAL_XOR     = 0b100110,
};

// Operations of the REGIMM opcode.
enum regimm_op_e {
    REGIMM_BGEZ     = 0b00001,
    REGIMM_BGEZAL   = 0b10001,
    REGIMM_BGEZALL  = 0b10011,
    REGIMM_BGEZL    = 0b00011,
    REGIMM_BLTZ     = 0b00000,
    REGIMM_BLTZAL   = 0b10000,
    REGIMM_BLTZALL  = 0b10010,
    REGIMM_BLTZL    = 0b00010,
    REGIMM_TEQI     = 0b01100,
    REGIMM_TGEI     = 0b01000,
    REGIMM_TGEIU    = 0b01001,
    REGIMM_TLTI     = 0b01010,
    REGIMM_TLTIU    = 0b01011,
    REGIMM_TNEI     = 0b01110,
};

#endif // ISA_H
