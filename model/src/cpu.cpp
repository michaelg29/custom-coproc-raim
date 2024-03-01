
#include "cpu.h"

#include "system.h"
#include "isa.h"

#include "systemc.h"

cpu::cpu(sc_module_name name, uint32_t start_addr, uint32_t exit_addr, uint32_t max_instr_cnt) : sc_module(name), _start_addr(start_addr), _exit_addr(exit_addr), _max_instr_cnt(max_instr_cnt) {
    SC_THREAD(main);

    // initial register values
    _regs.s.pc = _start_addr;
}

cpu::~cpu() {}

void cpu::main() {
    uint32_t instr_cnt = 0;

    // decoded instruction values
    int32_t rs, rt, rd;
    int32_t immd;

    // computation result
    int32_t next_pc;
    int32_t dst_reg_idx;
    int32_t res;

    while (!_max_instr_cnt || instr_cnt < _max_instr_cnt) {
        // default values
        next_pc = _regs.s.pc + 4;
        dst_reg_idx = -1;

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

        switch (GET_INSTR_OPCODE(_regs.s.ir)) {
        case OPCODE_SPECIAL: {
            dst_reg_idx = rd;
            switch (GET_INSTR_SPECIAL_OP(_regs.s.ir)) {
            case SPECIAL_ADD:
            case SPECIAL_ADDU: {
                res = _regs.a[rt] + _regs.a[rs];
                break;
            }
            case SPECIAL_AND: {
                res = _regs.a[rt] & _regs.a[rs];
                break;
            }
            case SPECIAL_BREAK: {
                break;
            }
            case SPECIAL_DIV: {
                _regs.s.lo = _regs.a[rs] / _regs.a[rt];
                _regs.s.hi = _regs.a[rs] % _regs.a[rt];
                break;
            }
            case SPECIAL_DIVU: {
                _regs.s.lo = (int32_t)(_regs.ua[rs] / _regs.ua[rt]);
                _regs.s.hi = (int32_t)(_regs.ua[rs] % _regs.ua[rt]);
                break;
            }
            case SPECIAL_JALR: {
                next_pc = _regs.ua[rs];
                _regs.a[rd] = _regs.s.pc + 4; // should be +8 for a pipelined register
                break;
            }
            case SPECIAL_JR: {
                next_pc = _regs.ua[rs];
                break;
            }
            default: {
                LOGF("Unsupported SPECIAL opcode: %02x", GET_INSTR_SPECIAL_OP(_regs.s.ir));
                dst_reg_idx = -1;
                break;
            }
            }
            break;
        }
        case OPCODE_ADDI:
        case OPCODE_ADDIU: {
            dst_reg_idx = rt;

            // get immediate and sign extend
            immd = GET_INSTR_BITS(_regs.s.ir, 0, 0xffff);
            if (immd & 0x7000) immd |= 0xffff0000;

            // perform immediate addition
            res = _regs.a[rs] + immd;
            break;
        }
        case OPCODE_ANDI: {
            dst_reg_idx = rt;

            // get immediate and zero extend
            immd = GET_INSTR_BITS(_regs.s.ir, 0, 0xffff);

            // perform immediate logical AND
            res = _regs.a[rs] & immd;
            break;
        }
        case OPCODE_BEQ:
        case OPCODE_BEQL: {
            // get immediate and left shift
            immd = GET_INSTR_BITS(_regs.s.ir, 0, 0xffff) << 2;

            // conditional PC-relative branch
            if (_regs.a[rs] == _regs.a[rt]) {
                next_pc = _regs.s.pc + immd;
            }
            break;
        }
        case OPCODE_REGIMM: {
            // get immediate and left shift
            immd = GET_INSTR_BITS(_regs.s.ir, 0, 0xffff) << 2;

            switch (rt) {
            case REGIMM_BGEZ:
            case REGIMM_BGEZL: {
                // conditional PC-relative branch
                if (_regs.a[rs] >= 0) {
                    next_pc = _regs.s.pc + immd;
                }
                break;
            }
            case REGIMM_BGEZAL:
            case REGIMM_BGEZALL: {
                // conditional PC-relative branch to procedure call
                if (_regs.a[rs] >= 0) {
                    next_pc = _regs.s.pc + immd;
                    _regs.a[31] = _regs.s.pc + 4; // should be +8 for a pipelined register
                }
                break;
            }
            case REGIMM_BLTZ:
            case REGIMM_BLTZL: {
                // conditional PC-relative branch
                if (_regs.a[rs] < 0) {
                    next_pc = _regs.s.pc + immd;
                }
                break;
            }
            case REGIMM_BLTZAL:
            case REGIMM_BLTZALL: {
                // conditional PC-relative branch to procedure call
                if (_regs.a[rs] < 0) {
                    next_pc = _regs.s.pc + immd;
                    _regs.a[31] = _regs.s.pc + 4; // should be +8 for a pipelined register
                }
                break;
            }
            default: {
                LOGF("Unsupported REGIMM opcode: %02x", rt);
                dst_reg_idx = -1;
                break;
            }
            }
            break;
        }
        case OPCODE_BGTZ:
        case OPCODE_BGTZL: {
            // get immediate and left shift
            immd = GET_INSTR_BITS(_regs.s.ir, 0, 0xffff) << 2;

            // conditional PC-relative branch
            if (_regs.a[rs] > 0) {
                next_pc = _regs.s.pc + immd;
            }
            break;
        }
        case OPCODE_BLEZ:
        case OPCODE_BLEZL: {
            // get immediate and left shift
            immd = GET_INSTR_BITS(_regs.s.ir, 0, 0xffff) << 2;

            // conditional PC-relative branch
            if (_regs.a[rs] <= 0) {
                next_pc = _regs.s.pc + immd;
            }
            break;
        }
        case OPCODE_BNE:
        case OPCODE_BNEL: {
            // get immediate and left shift
            immd = GET_INSTR_BITS(_regs.s.ir, 0, 0xffff) << 2;

            // conditional PC-relative branch
            if (_regs.a[rs] != _regs.a[rt]) {
                next_pc = _regs.s.pc + immd;
            }
            break;
        }
        case OPCODE_COP0:
        case OPCODE_COP1:
        case OPCODE_COP2:
        case OPCODE_COP3: {
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
            _regs.a[31] = _regs.s.pc + 4; // should be +8 for a pipelined register
            break;
        }
        default: {
            LOGF("Unsupported instr opcode: %02x", GET_INSTR_OPCODE(_regs.s.ir));
            break;
        }
        }

        // ================
        // == WRITE-BACK ==
        // ================
        if (dst_reg_idx >= 0) {
            _regs.a[dst_reg_idx] = res;
            LOGF("Writing result %08x to register %d (rd = %d, rs = %d, rt = %d)", res, dst_reg_idx, rd, rs, rt);
        }

        if (_regs.s.pc == _exit_addr) {
            break;
        }
        _regs.s.pc = next_pc;

        instr_cnt++;
        POSEDGE_CPU();
    }
}
