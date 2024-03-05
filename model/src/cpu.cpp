
#include "cpu.h"

#include "system.h"
#include "isa.h"

#include "systemc.h"

int32_t sign_extend_immd(uint32_t ir, uint32_t shift) {
    // get immediate and sign extend
    int32_t ret = GET_INSTR_BITS(ir, 0, 0xffff);
    if (ret & 0x7000) ret |= 0xffff0000;
    ret <<= shift;
    return ret;
}

int32_t zero_extend_immd(uint32_t ir, uint32_t shift) {
    // get immediate and sign extend
    int32_t ret = GET_INSTR_BITS(ir, 0, 0xffff);
    ret <<= shift;
    return ret;
}

cpu::cpu(sc_module_name name, uint32_t start_addr, uint32_t exit_addr, uint32_t max_instr_cnt) : sc_module(name), _start_addr(start_addr), _exit_addr(exit_addr), _max_instr_cnt(max_instr_cnt) {
    SC_THREAD(main);

    // initial register values
    _regs.s.pc = _start_addr;
}

cpu::~cpu() {}

void cpu::main() {
    uint32_t instr_cnt = 0;

    // decoded instruction values
    int32_t opcode;
    int32_t rs, rt, rd;
    int32_t immd;

    // floating point instruction values
    int32_t fs, ft, fd, fr;
    int32_t fpu_fmt;
    int32_t fp_branch_flags;

    // computation result
    int32_t next_pc;
    int32_t dst_reg_idx;
    int32_t res;
    uint32_t ures;
    int64_t res64;
    uint64_t ures64;

    while (!_max_instr_cnt || instr_cnt < _max_instr_cnt) {
        // default values
        next_pc = _regs.s.pc + 4;
        dst_reg_idx = -1;
        _regs.s.ex = EX_NONE;

        // =======================
        // == INSTRUCTION FETCH ==
        // =======================
        mem->read(_regs.s.pc, _regs.s.ir);
        LOGF("[%s]: Read instr %08x from addr %08x", this->name(), _regs.s.ir, _regs.s.pc);

        // ========================
        // == INSTRUCTION DECODE ==
        // ========================
        rs = GET_INSTR_REG(_regs.s.ir, 21);
        rt = GET_INSTR_REG(_regs.s.ir, 16);
        rd = GET_INSTR_REG(_regs.s.ir, 11);
        opcode = GET_INSTR_OPCODE(_regs.s.ir);

        switch (opcode) {
        case OPCODE_SPECIAL: {
            dst_reg_idx = rd;
            switch (GET_INSTR_SPECIAL_OP(_regs.s.ir)) {
            case SPECIAL_ADD:
            case SPECIAL_ADDU: {
                res = _regs.w[rt] + _regs.w[rs];
                break;
            }
            case SPECIAL_AND: {
                res = _regs.w[rt] & _regs.w[rs];
                break;
            }
            case SPECIAL_BREAK: {
                break;
            }
            case SPECIAL_DIV: {
                _regs.s.lo = _regs.w[rs] / _regs.w[rt];
                _regs.s.hi = _regs.w[rs] % _regs.w[rt];
                break;
            }
            case SPECIAL_DIVU: {
                _regs.s.lo = (int32_t)(_regs.uw[rs] / _regs.uw[rt]);
                _regs.s.hi = (int32_t)(_regs.uw[rs] % _regs.uw[rt]);
                break;
            }
            case SPECIAL_JALR: {
                next_pc = _regs.uw[rs];
                _regs.w[rd] = _regs.s.pc + 4; // should be +8 for a pipelined register
                break;
            }
            case SPECIAL_JR: {
                next_pc = _regs.uw[rs];
                break;
            }
            case SPECIAL_MFHI: {
                res = _regs.s.hi;
                break;
            }
            case SPECIAL_MFLO: {
                res = _regs.s.lo;
                break;
            }
            case SPECIAL_MOVN: {
                if (_regs.w[rt] != 0) {
                    res = _regs.w[rs];
                }
                else {
                    dst_reg_idx = -1;
                }
                break;
            }
            case SPECIAL_MOVZ: {
                if (_regs.w[rt] == 0) {
                    res = _regs.w[rs];
                }
                else {
                    dst_reg_idx = -1;
                }
                break;
            }
            case SPECIAL_MTHI: {
                _regs.s.hi = _regs.w[rs];
                dst_reg_idx = -1;
                break;
            }
            case SPECIAL_MTLO: {
                _regs.s.lo = _regs.w[rs];
                dst_reg_idx = -1;
                break;
            }
            case SPECIAL_MULT: {
                res64 = (int64_t)_regs.w[rs] * (int64_t)_regs.w[rt];
                _regs.s.lo = res64 & 0xffffffff;
                _regs.s.hi = (res64 >> 32) & 0xffffffff;
                dst_reg_idx = -1;
                break;
            }
            case SPECIAL_MULTU: {
                ures64 = (uint64_t)_regs.w[rs] * (uint64_t)_regs.w[rt];
                _regs.s.lo = (int32_t)(ures64 & 0xffffffff);
                _regs.s.hi = (int32_t)((ures64 >> 32) & 0xffffffff);
                dst_reg_idx = -1;
                break;
            }
            case SPECIAL_NOR: {
                res = ~(_regs.w[rs] | _regs.w[rt]);
                break;
            }
            case SPECIAL_OR: {
                res = _regs.w[rs] | _regs.w[rt];
                break;
            }
            case SPECIAL_SLL: {
                // get immediate
                immd = GET_INSTR_BITS(_regs.s.ir, 6, 0b11111);
                res = _regs.uw[rt] << immd;
                break;
            }
            case SPECIAL_SLLV: {
                res = _regs.uw[rt] << _regs.w[rs];
                break;
            }
            case SPECIAL_SLT: {
                res = _regs.w[rs] < _regs.w[rt] ? 1 : 0;
                break;
            }
            case SPECIAL_SLTU: {
                res = _regs.uw[rs] < _regs.uw[rt] ? 1 : 0;
                break;
            }
            case SPECIAL_SRA: {
                // get immediate
                immd = GET_INSTR_BITS(_regs.s.ir, 6, 0b11111);
                res = _regs.w[rt] >> immd;
                break;
            }
            case SPECIAL_SRAV: {
                res = _regs.w[rt] >> _regs.w[rs];
                break;
            }
            case SPECIAL_SRL: {
                // get immediate
                immd = GET_INSTR_BITS(_regs.s.ir, 6, 0b11111);
                res = _regs.uw[rt] >> immd;
                break;
            }
            case SPECIAL_SRLV: {
                res = _regs.uw[rt] >> _regs.w[rs];
                break;
            }
            case SPECIAL_SUB:
            case SPECIAL_SUBU: {
                res = _regs.w[rs] - _regs.w[rt];
                break;
            }
            case SPECIAL_SYSCALL: {
                // todo syscall
                break;
            }
            case SPECIAL_TEQ: {
                if (_regs.w[rs] == _regs.w[rt]) {
                    signal_ex(EX_TRAP);
                }
                break;
            }
            case SPECIAL_TGE: {
                if (_regs.w[rs] >= _regs.w[rt]) {
                    signal_ex(EX_TRAP);
                }
                break;
            }
            case SPECIAL_TGEU: {
                if (_regs.uw[rs] >= _regs.uw[rt]) {
                    signal_ex(EX_TRAP);
                }
                break;
            }
            case SPECIAL_TLT: {
                if (_regs.w[rs] < _regs.w[rt]) {
                    signal_ex(EX_TRAP);
                }
                break;
            }
            case SPECIAL_TLTU: {
                if (_regs.uw[rs] < _regs.uw[rt]) {
                    signal_ex(EX_TRAP);
                }
                break;
            }
            case SPECIAL_TNE: {
                if (_regs.uw[rs] != _regs.uw[rt]) {
                    signal_ex(EX_TRAP);
                }
                break;
            }
            case SPECIAL_XOR: {
                res = _regs.uw[rs] ^ _regs.uw[rt];
                break;
            }
            case SPECIAL_MOVF: {
                if (_fp_cc == 0) {
                    res = _regs.w[rs];
                }
                else {
                    dst_reg_idx = -1;
                }
                break;
            }
            case SPECIAL_DADD:
            case SPECIAL_DADDU:
            case SPECIAL_DDIV:
            case SPECIAL_DDIVU:
            case SPECIAL_DMULT:
            case SPECIAL_DMULTU:
            case SPECIAL_DSLL:
            case SPECIAL_DSLL32:
            case SPECIAL_DSLLV:
            case SPECIAL_DSRA:
            case SPECIAL_DSRA32:
            case SPECIAL_DSRAV:
            case SPECIAL_DSRL:
            case SPECIAL_DSRL32:
            case SPECIAL_DSRLV:
            case SPECIAL_DSUB:
            case SPECIAL_DSUBU:
            case SPECIAL_SYNC: {
                LOGF("Unimplemented SPECIAL opcode: %02x", GET_INSTR_SPECIAL_OP(_regs.s.ir));
                dst_reg_idx = -1;
                signal_ex(EX_NOIMP);
                break;
            }
            default: {
                LOGF("Invalid SPECIAL opcode: %02x", GET_INSTR_SPECIAL_OP(_regs.s.ir));
                dst_reg_idx = -1;
                signal_ex(EX_INVALID);
                break;
            }
            }
            break;
        }
        case OPCODE_ADDI:
        case OPCODE_ADDIU: {
            dst_reg_idx = rt;

            // get immediate and sign extend
            immd = sign_extend_immd(_regs.s.ir, 0);

            // perform immediate addition
            res = _regs.w[rs] + immd;
            break;
        }
        case OPCODE_ANDI: {
            dst_reg_idx = rt;

            // get immediate and zero extend
            immd = zero_extend_immd(_regs.s.ir, 0);

            // perform immediate logical AND
            res = _regs.w[rs] & immd;
            break;
        }
        case OPCODE_BEQ:
        case OPCODE_BEQL: {
            // get immediate and left shift
            immd = sign_extend_immd(_regs.s.ir, 2);

            // conditional PC-relative branch
            if (_regs.w[rs] == _regs.w[rt]) {
                next_pc = _regs.s.pc + immd;
            }
            break;
        }
        case OPCODE_REGIMM: {
            // get immediate and left shift
            immd = sign_extend_immd(_regs.s.ir, 2);

            switch (rt) {
            case REGIMM_BGEZ:
            case REGIMM_BGEZL: {
                // conditional PC-relative branch
                if (_regs.w[rs] >= 0) {
                    next_pc = _regs.s.pc + immd;
                }
                break;
            }
            case REGIMM_BGEZAL:
            case REGIMM_BGEZALL: {
                // conditional PC-relative branch to procedure call
                if (_regs.w[rs] >= 0) {
                    next_pc = _regs.s.pc + immd;
                    _regs.w[31] = _regs.s.pc + 4; // should be +8 for a pipelined register
                }
                break;
            }
            case REGIMM_BLTZ:
            case REGIMM_BLTZL: {
                // conditional PC-relative branch
                if (_regs.w[rs] < 0) {
                    next_pc = _regs.s.pc + immd;
                }
                break;
            }
            case REGIMM_BLTZAL:
            case REGIMM_BLTZALL: {
                // conditional PC-relative branch to procedure call
                if (_regs.w[rs] < 0) {
                    next_pc = _regs.s.pc + immd;
                    _regs.w[31] = _regs.s.pc + 4; // should be +8 for a pipelined register
                }
                break;
            }
            case REGIMM_TEQI: {
                // get immediate and sign extend
                immd = sign_extend_immd(_regs.s.ir, 0);
                if (_regs.w[rs] == immd) {
                    signal_ex(EX_TRAP);
                }
                dst_reg_idx = -1;
                break;
            }
            case REGIMM_TGEI: {
                // get immediate and sign extend
                immd = sign_extend_immd(_regs.s.ir, 0);
                if (_regs.w[rs] >= immd) {
                    signal_ex(EX_TRAP);
                }
                dst_reg_idx = -1;
                break;
            }
            case REGIMM_TGEIU: {
                // get immediate
                immd = zero_extend_immd(_regs.s.ir, 0);
                if (_regs.w[rs] >= immd) {
                    signal_ex(EX_TRAP);
                }
                dst_reg_idx = -1;
                break;
            }
            case REGIMM_TLTI: {
                // get immediate and sign extend
                immd = sign_extend_immd(_regs.s.ir, 0);
                if (_regs.w[rs] < immd) {
                    signal_ex(EX_TRAP);
                }
                dst_reg_idx = -1;
                break;
            }
            case REGIMM_TLTIU: {
                // get immediate
                immd = zero_extend_immd(_regs.s.ir, 0);
                if (_regs.uw[rs] < immd) {
                    signal_ex(EX_TRAP);
                }
                dst_reg_idx = -1;
                break;
            }
            case REGIMM_TNEI: {
                // get immediate and sign extend
                immd = sign_extend_immd(_regs.s.ir, 0);
                if (_regs.w[rs] != immd) {
                    signal_ex(EX_TRAP);
                }
                dst_reg_idx = -1;
                break;
            }
            /*case : {
                LOGF("Unimplemented REGIMM opcode: %02x", rt);
                dst_reg_idx = -1;
                signal_ex(EX_NOIMP);
                break;
            }*/
            default: {
                LOGF("Invalid REGIMM opcode: %02x", rt);
                dst_reg_idx = -1;
                signal_ex(EX_INVALID);
                break;
            }
            }
            break;
        }
        case OPCODE_BGTZ:
        case OPCODE_BGTZL: {
            // get immediate and left shift
            immd = sign_extend_immd(_regs.s.ir, 2);

            // conditional PC-relative branch
            if (_regs.w[rs] > 0) {
                next_pc = _regs.s.pc + immd;
            }
            break;
        }
        case OPCODE_BLEZ:
        case OPCODE_BLEZL: {
            // get immediate and left shift
            immd = sign_extend_immd(_regs.s.ir, 2);

            // conditional PC-relative branch
            if (_regs.w[rs] <= 0) {
                next_pc = _regs.s.pc + immd;
            }
            break;
        }
        case OPCODE_BNE:
        case OPCODE_BNEL: {
            // get immediate and left shift
            immd = zero_extend_immd(_regs.s.ir, 2);

            // conditional PC-relative branch
            if (_regs.w[rs] != _regs.w[rt]) {
                next_pc = _regs.s.pc + immd;
            }
            break;
        }
        case OPCODE_COP0:
        //case OPCODE_COP1: // defined below as floating point instructions
        case OPCODE_COP2: {
        //case OPCODE_COP3: { // defined below as floating point instruction
            break;
        }
        case OPCODE_J: {
            immd = GET_INSTR_BITS(_regs.s.ir, 0, 0x03ffffff) << 2;
            next_pc = _regs.s.pc + immd;
            break;
        }
        case OPCODE_JAL: {
            immd = GET_INSTR_BITS(_regs.s.ir, 0, 0x03ffffff) << 2;
            next_pc = _regs.s.pc + immd;
            _regs.w[31] = _regs.s.pc + 4; // should be +8 for a pipelined register
            break;
        }
        case OPCODE_LB: {
            // get immediate and sign extend
            immd = sign_extend_immd(_regs.s.ir, 0);

            // construct address and load 4B word from memory
            immd = immd + _regs.w[rs];
            mem->read(immd & 0xfffffffc, ures);

            // shift and mask to get byte
            res = ((int32_t)ures >> ((immd & 0b11) << 3)) & 0xff;
            dst_reg_idx = rt;

            // sign extend
            if (res & 0x70) res |= 0xffffff00;
            break;
        }
        case OPCODE_LBU: {
            // get immediate and sign extend
            immd = sign_extend_immd(_regs.s.ir, 0);

            // construct address and load 4B word from memory
            immd = immd + _regs.w[rs];
            mem->read(immd & 0xfffffffc, ures);

            // shift and mask to get byte
            res = ((int32_t)ures >> ((immd & 0b11) << 3)) & 0xff;
            dst_reg_idx = rt;
            break;
        }
        case OPCODE_LH: {
            // get immediate and sign extend
            immd = sign_extend_immd(_regs.s.ir, 0);

            // construct address
            immd = immd + _regs.w[rs];
            if (immd & 0b1) {
                signal_ex(EX_ADDRESS);
            }
            else {
                // load 4B word from memory
                mem->read(immd & 0xfffffffc, ures);

                // shift and mask to get halfword
                res = ((int32_t)ures >> ((immd & 0b10) << 3)) & 0xffff;
                dst_reg_idx = rt;

                // sign extend
                if (res & 0x7000) res |= 0xffff0000;
            }
            break;
        }
        case OPCODE_LHU: {
            // get immediate and sign extend
            immd = sign_extend_immd(_regs.s.ir, 0);

            // construct address
            immd = immd + _regs.w[rs];
            if (immd & 0b1) {
                signal_ex(EX_ADDRESS);
            }
            else {
                // load 4B word from memory
                mem->read(immd & 0xfffffffc, ures);

                // shift and mask to get halfword
                res = ((int32_t)ures >> ((immd & 0b10) << 3)) & 0xffff;
                dst_reg_idx = rt;
            }
            break;
        }
        case OPCODE_LUI: {
            // get immedate
            immd = zero_extend_immd(_regs.s.ir, 0);
            res = (immd << 16) & 0xffff0000;
            dst_reg_idx = rt;
            break;
        }
        case OPCODE_LW:
        case OPCODE_LWU: {
            // get immediate and sign extend
            immd = sign_extend_immd(_regs.s.ir, 0);

            // construct address
            immd = immd + _regs.w[rs];
            if (immd & 0b1) {
                signal_ex(EX_ADDRESS);
            }
            else {
                // load 4B word from memory
                mem->read(immd & 0xfffffffc, ures);
                res = (int32_t)ures;
                dst_reg_idx = rt;
            }
            break;
        }
        //case OPCODE_LWC1: // defined below as floating point instruction
        case OPCODE_LWC2:
        case OPCODE_LWC3: {
            // get immediate and sign extend
            immd = sign_extend_immd(_regs.s.ir, 0);

            // construct address
            immd = immd + _regs.w[rs];
            if (immd & 0b11) {
                signal_ex(EX_ADDRESS);
            }
            else {
                // load 4B word from memory
                mem->read(immd & 0xfffffffc, ures);
                res = (int32_t)ures;
                dst_reg_idx = rt;

                // todo write to coprocessor z register
                immd = opcode & 0b11;
            }
            break;
        }
        case OPCODE_ORI: {
            // get immedate
            immd = zero_extend_immd(_regs.s.ir, 0);
            res = immd | _regs.w[rs];
            dst_reg_idx = rt;
            break;
        }
        case OPCODE_SB: {
            // get immediate and sign extend
            immd = sign_extend_immd(_regs.s.ir, 0);

            // construct address and write byte to memory
            immd = immd + _regs.w[rs];
            mem->write(immd, (uint32_t)_regs.w[rt], 1);
            break;
        }
        case OPCODE_SH: {
            // get immediate and sign extend
            immd = sign_extend_immd(_regs.s.ir, 0);

            // construct address and write byte to memory
            immd = immd + _regs.w[rs];

            if (immd & 0b1) {
                signal_ex(EX_ADDRESS);
            }
            else {
                // write halfword to memory
                mem->write(immd, (uint32_t)_regs.w[rt], 2);
            }
            break;
        }
        case OPCODE_SLTI: {
            // get immediate and sign extend
            immd = sign_extend_immd(_regs.s.ir, 0);
            res = _regs.w[rs] < immd ? 1 : 0;
            dst_reg_idx = rt;
            break;
        }
        case OPCODE_SLTIU: {
            // get immediate and sign extend
            immd = sign_extend_immd(_regs.s.ir, 0);
            res = (uint32_t)_regs.w[rs] < (uint32_t)immd ? 1 : 0;
            dst_reg_idx = rt;
            break;
        }
        case OPCODE_SW: {
            // get immediate and sign extend
            immd = sign_extend_immd(_regs.s.ir, 0);

            // construct address and write byte to memory
            immd = immd + _regs.w[rs];

            if (immd & 0b11) {
                signal_ex(EX_ADDRESS);
            }
            else {
                // write word to memory
                mem->write(immd, (uint32_t)_regs.w[rt], 4);
            }
            break;
        }
        //case OPCODE_SWC1: // defined below as floating point instruction
        case OPCODE_SWC2:
        case OPCODE_SWC3: {
            // get immediate and sign extend
            immd = sign_extend_immd(_regs.s.ir, 0);

            // construct address
            immd = immd + _regs.w[rs];
            if (immd & 0b11) {
                signal_ex(EX_ADDRESS);
            }
            else {
                // todo read from coprocessor z register
                // ures = ...;

                // write to memory
                mem->write(immd, ures, 4);
            }
            break;
        }
        case OPCODE_XORI: {
            // get immedate
            immd = zero_extend_immd(_regs.s.ir, 0);
            res = immd ^ _regs.w[rs];
            dst_reg_idx = rt;
            break;
        }
        case OPCODE_COP1: {
            dst_reg_idx = -1;
            fs = GET_INSTR_REG(_regs.s.ir, 11);
            ft = GET_INSTR_REG(_regs.s.ir, 16);
            fd = GET_INSTR_REG(_regs.s.ir, 6);
            fr = GET_INSTR_REG(_regs.s.ir, 21);

            fpu_fmt = GET_INSTR_FPU_FMT(_regs.s.ir);
            fp_branch_flags = GET_INSTR_FPU_FLAGS(_regs.s.ir);

            // decode format
            if (fpu_fmt == FPU_FMT_BC) {
                // branch instruction
                switch (fp_branch_flags) {
                case 0b00:
                case 0b10: {
                    // get immediate and left shift
                    immd = sign_extend_immd(_regs.s.ir, 2);

                    // conditional PC-relative branch
                    if (_fp_cc == 0b000) {
                        next_pc = _regs.s.pc + immd;
                    }
                    break;
                }
                case 0b01:
                case 0b11: {
                    // get immediate and left shift
                    immd = sign_extend_immd(_regs.s.ir, 2);

                    // conditional PC-relative branch
                    if (_fp_cc == 0b001) {
                        next_pc = _regs.s.pc + immd;
                    }
                    break;
                }
                }
                break;
            }
            else if (GET_INSTR_BITS(_regs.s.ir, 4, 0b11) == 0b11) {
                // set the condition flag
                switch (GET_INSTR_BITS(_regs.s.ir, 0, 0xf)) {
                case FPU_F:    _fp_cc = 0; break;
                case FPU_UN:   _fp_cc = 0; break;
                case FPU_EQ:   _fp_cc = _fp_regs.s[fs] == _fp_regs.s[ft] ? 1 : 0; break;
                case FPU_UEQ:  _fp_cc = _fp_regs.s[fs] == _fp_regs.s[ft] ? 1 : 0; break;
                case FPU_OLT:  _fp_cc = _fp_regs.s[fs] < _fp_regs.s[ft] ? 1 : 0; break;
                case FPU_ULT:  _fp_cc = _fp_regs.s[fs] < _fp_regs.s[ft] ? 1 : 0; break;
                case FPU_OLE:  _fp_cc = _fp_regs.s[fs] <= _fp_regs.s[ft] ? 1 : 0; break;
                case FPU_ULE:  _fp_cc = _fp_regs.s[fs] <= _fp_regs.s[ft] ? 1 : 0; break;
                case FPU_SF:   _fp_cc = 0; break;
                case FPU_NGLE: _fp_cc = 0; break;
                case FPU_SEQ:  _fp_cc = _fp_regs.s[fs] == _fp_regs.s[ft] ? 1 : 0; break;
                case FPU_NGL:  _fp_cc = _fp_regs.s[fs] == _fp_regs.s[ft] ? 1 : 0; break;
                case FPU_LT:   _fp_cc = _fp_regs.s[fs] < _fp_regs.s[ft] ? 1 : 0; break;
                case FPU_NGE:  _fp_cc = _fp_regs.s[fs] < _fp_regs.s[ft] ? 1 : 0; break;
                case FPU_LE:   _fp_cc = _fp_regs.s[fs] <= _fp_regs.s[ft] ? 1 : 0; break;
                case FPU_NGT:  _fp_cc = _fp_regs.s[fs] <= _fp_regs.s[ft] ? 1 : 0; break;
                }
                break;
            }
            else if (fpu_fmt == FPU_FMT_CF) {
                res = *(int32_t*)&_fp_regs.s[fs];
                dst_reg_idx = rt;
                break;
            }
            else if (fpu_fmt == FPU_FMT_CT) {
                _fp_regs.s[fs] = *(float*)&_regs.w[rt];
                break;
            }
            else if (fpu_fmt == FPU_FMT_MFC) {
                res = *(int32_t*)&_fp_regs.s[fs];
                dst_reg_idx = rt;
                break;
            }
            else if (fpu_fmt == FPU_FMT_MT) {
                _fp_regs.s[fs] = *(float*)&_regs.w[rt];
                break;
            }
            /*else if (fpu_fmt != FPU_FMT_S) {
                LOGF("Unimplemented FPU format: %02x", fpu_fmt);
                dst_reg_idx = -1;
                signal_ex(EX_NOIMP);
                break;
            }*/

            // decode operation
            switch (GET_INSTR_FPU_OP(_regs.s.ir)) {
            case FPU_ABS: {
                _fp_regs.s[fd] = fabs(_fp_regs.s[fs]);
                break;
            }
            case FPU_ADD: {
                _fp_regs.s[fd] = _fp_regs.s[fs] + _fp_regs.s[ft];
                break;
            }
            case FPU_DIV: {
                _fp_regs.s[fd] = _fp_regs.s[fs] / _fp_regs.s[ft];
                break;
            }
            case FPU_MADD_S: {
                _fp_regs.s[fd] = _fp_regs.s[fs] * _fp_regs.s[ft] + _fp_regs.s[fr];
                break;
            }
            case FPU_MOV: {
                _fp_regs.s[fd] = _fp_regs.s[fs];
                break;
            }
            case FPU_MOVCF: {
                // branch flags must be equal to the condition code
                if (!(fp_branch_flags ^ _fp_cc)) {
                    _fp_regs.s[fd] = _fp_regs.s[fs];
                }
                break;
            }
            case FPU_MOVN: {
                if (_regs.w[rt] != 0) {
                    _fp_regs.s[fd] = _fp_regs.s[fs];
                }
                break;
            }
            case FPU_MOVZ: {
                if (_regs.w[rt] == 0) {
                    _fp_regs.s[fd] = _fp_regs.s[fs];
                }
                break;
            }
            case FPU_MSUB_S: {
                _fp_regs.s[fd] = _fp_regs.s[fs] * _fp_regs.s[ft] - _fp_regs.s[fr];
                break;
            }
            case FPU_MUL: {
                _fp_regs.s[fd] = _fp_regs.s[fs] * _fp_regs.s[ft];
                break;
            }
            case FPU_NEG: {
                _fp_regs.s[fd] = -_fp_regs.s[fs];
                break;
            }
            case FPU_NMADD_S: {
                _fp_regs.s[fd] = -(_fp_regs.s[fs] * _fp_regs.s[ft] + _fp_regs.s[fr]);
                break;
            }
            case FPU_NMSUB_S: {
                _fp_regs.s[fd] = -(_fp_regs.s[fs] * _fp_regs.s[ft] - _fp_regs.s[fr]);
                break;
            }
            case FPU_RECIP: {
                _fp_regs.s[fd] = 1.0f / _fp_regs.s[fs];
                break;
            }
            case FPU_RSQRT: {
                _fp_regs.s[fd] = 1.0f / sqrt(_fp_regs.s[fs]);
                break;
            }
            case FPU_SQRT: {
                _fp_regs.s[fd] = sqrt(_fp_regs.s[fs]);
                break;
            }
            case FPU_SUB: {
                _fp_regs.s[fd] = _fp_regs.s[fs] - _fp_regs.s[ft];
                break;
            }
            case FPU_CEILL:
            case FPU_CEILW:
            //case FPU_CVTD: // repeated 6LSbs as FPU_MADD_D
            case FPU_CVTL:
            //case FPU_CVTS: // repeated 6LSbs as FPU_MADD_S
            case FPU_CVTW:
            case FPU_FLOORL:
            case FPU_FLOORW:
            case FPU_MADD_D:
            case FPU_MSUB_D:
            case FPU_NMADD_D:
            case FPU_ROUNDL:
            case FPU_ROUNDW:
            case FPU_TRUNCW: {
                LOGF("Unimplemented FPU opcode: %02x", GET_INSTR_FPU_OP(_regs.s.ir));
                dst_reg_idx = -1;
                signal_ex(EX_NOIMP);
                break;
            }
            default: {
                LOGF("Invalid FPU opcode: %02x", GET_INSTR_FPU_OP(_regs.s.ir));
                dst_reg_idx = -1;
                signal_ex(EX_INVALID);
                break;
            }
            }
            break;
        }
        case OPCODE_COP1X: {
            // decode operation
            switch (GET_INSTR_FPU_OP(_regs.s.ir)) {
            case FPU_LWXC1: {
                fd = GET_INSTR_REG(_regs.s.ir, 6);

                // construct address
                immd = _regs.w[rs] + _regs.w[rt];
                if (immd & 0b11) {
                    signal_ex(EX_ADDRESS);
                }
                else {
                    // load 4B word from memory
                    mem->read(immd & 0xfffffffc, ures);
                    _fp_regs.s[fd] = *(float*)&ures;
                }
                break;
            }
            case FPU_SWXC1: {
                fs = GET_INSTR_REG(_regs.s.ir, 11);

                // construct address
                immd = _regs.w[rs] + _regs.w[rt];
                if (immd & 0b11) {
                    signal_ex(EX_ADDRESS);
                }
                else {
                    // write word to memory
                    mem->write(immd, *(uint32_t*)&_fp_regs.s[fs], 4);
                }
                break;
            }
            /*case : {
                LOGF("Unimplemented FPU opcode: %02x", GET_INSTR_FPU_OP(_regs.s.ir));
                dst_reg_idx = -1;
                signal_ex(EX_NOIMP);
                break;
            }*/
            default: {
                LOGF("Invalid COP1X opcode: %02x", GET_INSTR_FPU_OP(_regs.s.ir));
                dst_reg_idx = -1;
                signal_ex(EX_INVALID);
                break;
            }
            }
            break;
        }
        case OPCODE_LWC1: {
            // get immediate and sign extend
            immd = sign_extend_immd(_regs.s.ir, 0);
            ft = GET_INSTR_REG(_regs.s.ir, 16);

            // construct address
            immd = immd + _regs.w[rs];
            if (immd & 0b11) {
                signal_ex(EX_ADDRESS);
            }
            else {
                // load 4B word from memory
                mem->read(immd & 0xfffffffc, ures);
                _fp_regs.s[ft] = *(float*)&ures;
            }
            break;
        }
        case OPCODE_SWC1: {
            // get immediate and sign extend
            immd = sign_extend_immd(_regs.s.ir, 0);
            ft = GET_INSTR_REG(_regs.s.ir, 16);

            // construct address
            immd = immd + _regs.w[rs];
            if (immd & 0b11) {
                signal_ex(EX_ADDRESS);
            }
            else {
                // write word to memory
                mem->write(immd, *(uint32_t*)&_fp_regs.s[ft], 4);
            }
            break;
        }
        case OPCODE_DADDI:
        case OPCODE_DADDIU:
        case OPCODE_LD:
        case OPCODE_LDC1:
        case OPCODE_LDC2:
        case OPCODE_LDL:
        case OPCODE_LDR:
        case OPCODE_LL:
        case OPCODE_LLD:
        case OPCODE_LWL:
        case OPCODE_LWR:
        //case OPCODE_PREF:
        case OPCODE_SC:
        case OPCODE_SCD:
        case OPCODE_SD:
        case OPCODE_SDC1:
        case OPCODE_SDC2:
        case OPCODE_SDL:
        case OPCODE_SDR:
        case OPCODE_SWL:
        case OPCODE_SWR: {
        //case OPCODE_LDC1: // repeated
        //case OPCODE_LDXC1: // repeated
            LOGF("Unimplemented instr opcode: %02x", GET_INSTR_OPCODE(_regs.s.ir));
            signal_ex(EX_NOIMP);
            break;
        }
        default: {
            LOGF("Invalid instr opcode: %02x", GET_INSTR_OPCODE(_regs.s.ir));
            signal_ex(EX_INVALID);
            break;
        }
        }

        // ================
        // == WRITE-BACK ==
        // ================
        if (dst_reg_idx >= 0) {
            _regs.w[dst_reg_idx] = res;
            LOGF("Writing result %08x to register %d (rd = %d, rs = %d, rt = %d)", res, dst_reg_idx, rd, rs, rt);
        }

        if (_regs.s.ex != EX_NONE) {
            LOGF("Error code: %08x", _regs.s.ex);
        }

        if (_regs.s.pc == _exit_addr) {
            break;
        }
        _regs.s.pc = next_pc;

        instr_cnt++;
        POSEDGE_CPU();
    }
}

void cpu::signal_ex(exception_e ex) {
    _regs.s.ex |= ex;
}
