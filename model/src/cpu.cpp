
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

coprocessor_if::coprocessor_if(uint32_t cop_opcode)
    : _prev_ex(EX_NONE), _cop_opcode(cop_opcode) {}

void coprocessor_if::signal_ex(exception_e ex) {
    _prev_ex = ex;
}

bool coprocessor_if::get_condition_code(uint8_t &cc) {
    return false;
}

exception_e coprocessor_if::get_exception() {
    exception_e ret = _prev_ex;
    _prev_ex = EX_NONE;
    return ret;
}

bool coprocessor_if::get_next_pc_offset(int32_t &next_pc_offset) {
    return false;
}

stubbed_cop::stubbed_cop()
    : coprocessor_if(0) {}

bool stubbed_cop::execute(uint32_t ir, int32_t rt, int32_t &res) {
    _prev_ex = EX_COP_UNUSABLE;
    return false;
}

bool stubbed_cop::get_regs(uint32_t rt, int32_t &res) {
    _prev_ex = EX_COP_UNUSABLE;
    return false;
}

void stubbed_cop::set_regs(uint32_t rt, int32_t res) {}

cpu::cpu(sc_module_name name, uint32_t start_addr, uint32_t exit_addr, uint32_t max_instr_cnt) : sc_module(name), _start_addr(start_addr), _exit_addr(exit_addr), _max_instr_cnt(max_instr_cnt) {
    SC_THREAD(main);

    // initial register values
    memset(&_regs, 0, sizeof(regs_t));
    _regs.s.pc = _start_addr;
}

cpu::~cpu() {}

void cpu::main() {
    uint32_t instr_cnt = 0;

    // decoded instruction values
    int32_t opcode;
    int32_t rs, rt, rd;
    int32_t immd;

    // intermediate co-processor values
    uint8_t condition_code;
    int32_t cop_idx;
    sc_port<coprocessor_if> *target_cops[4];
    target_cops[0] = NULL;
    target_cops[1] = &cop1;
    target_cops[2] = &cop2;
    target_cops[3] = &cop3;

    // computation result
    int32_t next_pc;
    int32_t dst_reg_idx;
    int32_t res;
    uint32_t ures;
    int64_t res64;
    uint64_t ures64;
    char buf[5];

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
        cop_idx = opcode & 0b11;

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
                LOGF("[%s] Syscall code %d", this->name(), _regs.s.v0);
                dst_reg_idx = -1;
                switch (_regs.s.v0) {
                case SYSCALL_PINT: {
                    // print integer in $a0
                    printf("%d", _regs.s.a0);
                    break;
                }
                case SYSCALL_PSTR: {
                    // print string with address in $a0
                    res = _regs.s.a0;
                    while (true) {
                        // read memory
                        mem->read(res, ures);

                        // print as char*
                        *((uint32_t*)buf) = ures;
                        buf[4] = 0;
                        printf("%s", buf);

                        // check bytes for terminator
                        if (!(buf[0] && buf[1] && buf[2] && buf[3])) {
                            break;
                        }

                        // increment cursor
                        res += 4;
                    }
                    break;
                }
                case SYSCALL_EXIT: {
                    // exit program
                    _max_instr_cnt = 1;
                    break;
                }
                case SYSCALL_PCHAR: {
                    // print character in $a0
                    printf("%c", _regs.s.a0);
                    break;
                }
                };
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
                if (cop1->get_condition_code(condition_code) && condition_code == 0) {
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
        case OPCODE_BGTZ: {
        //case OPCODE_BGTZL: {
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
        case OPCODE_BNE: {
        //case OPCODE_BNEL: {
            // get immediate and left shift
            immd = sign_extend_immd(_regs.s.ir, 2);

            // conditional PC-relative branch
            if (_regs.w[rs] != _regs.w[rt]) {
                next_pc = _regs.s.pc + immd;
            }
            break;
        }
        case OPCODE_COP1:
        case OPCODE_COP2: {
            // execute on target co-processor
            immd = opcode & 0b11;
            dst_reg_idx = (*target_cops[immd])->execute(_regs.s.ir, _regs.w[rt], res) ? rt : -1;
            break;
        }
        case OPCODE_COP1X:
        case OPCODE_COP2X:
        case OPCODE_COP3X: {
            // calculate new co-processor index
            cop_idx = (opcode >> 1) & 0b11;

            // decode operation
            switch (GET_INSTR_COP_OP(_regs.s.ir)) {
            case OPCODE_LWXCZ: {
                // construct address
                immd = _regs.w[rs] + _regs.w[rt];
                if (immd & 0b11) {
                    signal_ex(EX_ADDRESS);
                }
                else {
                    // load 4B word from memory
                    mem->read(immd, ures);
                    (*target_cops[cop_idx])->set_regs(GET_INSTR_REG(_regs.s.ir, 6), (int32_t)ures);
                }
                break;
            }
            case OPCODE_SWXCZ: {
                // construct address
                immd = _regs.w[rs] + _regs.w[rt];
                if (immd & 0b11) {
                    signal_ex(EX_ADDRESS);
                }
                else {
                    // write word to memory
                    (*target_cops[cop_idx])->get_regs(GET_INSTR_REG(_regs.s.ir, 11), res);
                    mem->write(immd, (uint32_t)res, 4);
                }
                break;
            }
            /*case : {
                LOGF("Unimplemented FPU opcode: %02x", GET_INSTR_COP_OP(_regs.s.ir));
                dst_reg_idx = -1;
                signal_ex(EX_NOIMP);
                break;
            }*/
            default: {
                dst_reg_idx = cop1->execute(_regs.s.ir, _regs.w[rt], res) ? rt : -1;
                break;
            }
            }
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
        case OPCODE_LWC1:
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
                mem->read(immd, ures);
                (*target_cops[cop_idx])->set_regs(rt, (int32_t)ures);
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
        case OPCODE_SWC1:
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
                // write word to memory
                (*target_cops[cop_idx])->get_regs(rt, res);
                mem->write(immd, (uint32_t)res, 4);
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

        // capture exceptions
        if (_regs.s.ex != EX_NONE) {
            LOGF("Error code: %08x", _regs.s.ex);
        }
        if ((ures = cop1->get_exception()) != EX_NONE) {
            LOGF("COP1 error code: %08x", ures);
        }
        if ((ures = cop2->get_exception()) != EX_NONE) {
            LOGF("COP2 error code: %08x", ures);
        }
        if ((ures = cop3->get_exception()) != EX_NONE) {
            LOGF("COP3 error code: %08x", ures);
        }

        // update PC
        if (_regs.s.pc == _exit_addr) {
            break;
        }
        _regs.s.pc = next_pc;
        if (cop1->get_next_pc_offset(res)) {
            _regs.s.pc = _regs.s.pc + res;
        }
        if (cop2->get_next_pc_offset(res)) {
            _regs.s.pc = _regs.s.pc + res;
        }
        if (cop3->get_next_pc_offset(res)) {
            _regs.s.pc = _regs.s.pc + res;
        }

        instr_cnt++;
        POSEDGE_CPU();
    }

    sc_stop();
}

void cpu::signal_ex(exception_e ex) {
    _regs.s.ex |= ex;
}
