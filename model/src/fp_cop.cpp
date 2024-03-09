
#include "fp_cop.h"

#include "system.h"
#include "isa.h"

#include "systemc.h"

fp_cop::fp_cop(sc_module_name name)
    : sc_module(name), _prev_ex(EX_NONE) {

}

fp_cop::~fp_cop() {

}

bool fp_cop::execute(uint32_t ir, int32_t rt, int32_t &res) {
    // default values
    _prev_ex = EX_NONE;
    _has_next_pc_offset = false;

    // decode instruction values
    _fs = GET_INSTR_REG(ir, 11);
    _ft = GET_INSTR_REG(ir, 16);
    _fd = GET_INSTR_REG(ir, 6);
    _fr = GET_INSTR_REG(ir, 21);

    _fpu_fmt = GET_INSTR_FPU_FMT(ir);
    _branch_flags = GET_INSTR_FPU_FLAGS(ir);

    // decode format
    if (_fpu_fmt == FPU_FMT_BC) {
        // branch instruction
        switch (_branch_flags) {
        case 0b00:
        case 0b10: {
            // get immediate and left shift
            _next_pc_offset = sign_extend_immd(ir, 2);

            // conditional PC-relative branch
            _has_next_pc_offset = _condition_code == 0b000;
            break;
        }
        case 0b01:
        case 0b11: {
            // get immediate and left shift
            _next_pc_offset = sign_extend_immd(ir, 2);

            // conditional PC-relative branch
            _has_next_pc_offset = _condition_code == 0b001;
            break;
        }
        }
        return false;
    }
    else if (GET_INSTR_BITS(ir, 4, 0b11) == 0b11) {
        // set the condition flag
        switch (GET_INSTR_BITS(ir, 0, 0xf)) {
        case FPU_F:    _condition_code = 0; break;
        case FPU_UN:   _condition_code = 0; break;
        case FPU_EQ:   _condition_code = _regs.s[_fs] == _regs.s[_ft] ? 1 : 0; break;
        case FPU_UEQ:  _condition_code = _regs.s[_fs] == _regs.s[_ft] ? 1 : 0; break;
        case FPU_OLT:  _condition_code = _regs.s[_fs] < _regs.s[_ft] ? 1 : 0; break;
        case FPU_ULT:  _condition_code = _regs.s[_fs] < _regs.s[_ft] ? 1 : 0; break;
        case FPU_OLE:  _condition_code = _regs.s[_fs] <= _regs.s[_ft] ? 1 : 0; break;
        case FPU_ULE:  _condition_code = _regs.s[_fs] <= _regs.s[_ft] ? 1 : 0; break;
        case FPU_SF:   _condition_code = 0; break;
        case FPU_NGLE: _condition_code = 0; break;
        case FPU_SEQ:  _condition_code = _regs.s[_fs] == _regs.s[_ft] ? 1 : 0; break;
        case FPU_NGL:  _condition_code = _regs.s[_fs] == _regs.s[_ft] ? 1 : 0; break;
        case FPU_LT:   _condition_code = _regs.s[_fs] < _regs.s[_ft] ? 1 : 0; break;
        case FPU_NGE:  _condition_code = _regs.s[_fs] < _regs.s[_ft] ? 1 : 0; break;
        case FPU_LE:   _condition_code = _regs.s[_fs] <= _regs.s[_ft] ? 1 : 0; break;
        case FPU_NGT:  _condition_code = _regs.s[_fs] <= _regs.s[_ft] ? 1 : 0; break;
        }
        return false;
    }
    else if (_fpu_fmt == FPU_FMT_CF) {
        res = *(int32_t*)&_regs.s[_fs];
        return true;
    }
    else if (_fpu_fmt == FPU_FMT_CT) {
        _regs.s[_fs] = *(float*)&rt;
        return false;
    }
    else if (_fpu_fmt == FPU_FMT_MFC) {
        res = *(int32_t*)&_regs.s[_fs];
        return true;
    }
    else if (_fpu_fmt == FPU_FMT_MT) {
        _regs.s[_fs] = *(float*)&rt;
        return false;
    }
    /*else if (_fpu_fmt != FPU_FMT_S) {
        LOGF("Unimplemented FPU format: %02x", _fpu_fmt);
        dst_reg_idx = -1;
        signal_ex(EX_NOIMP);
        break;
    }*/

    // decode operation
    switch (GET_INSTR_FPU_OP(ir)) {
    case FPU_ABS: {
        _regs.s[_fd] = fabs(_regs.s[_fs]);
        break;
    }
    case FPU_ADD: {
        _regs.s[_fd] = _regs.s[_fs] + _regs.s[_ft];
        break;
    }
    case FPU_DIV: {
        _regs.s[_fd] = _regs.s[_fs] / _regs.s[_ft];
        break;
    }
    case FPU_MADD_S: {
        _regs.s[_fd] = _regs.s[_fs] * _regs.s[_ft] + _regs.s[_fr];
        break;
    }
    case FPU_MOV: {
        _regs.s[_fd] = _regs.s[_fs];
        break;
    }
    case FPU_MOVCF: {
        // branch flags must be equal to the condition code
        if (!(_branch_flags ^ _condition_code)) {
            _regs.s[_fd] = _regs.s[_fs];
        }
        break;
    }
    case FPU_MOVN: {
        if (rt != 0) {
            _regs.s[_fd] = _regs.s[_fs];
        }
        break;
    }
    case FPU_MOVZ: {
        if (rt == 0) {
            _regs.s[_fd] = _regs.s[_fs];
        }
        break;
    }
    case FPU_MSUB_S: {
        _regs.s[_fd] = _regs.s[_fs] * _regs.s[_ft] - _regs.s[_fr];
        break;
    }
    case FPU_MUL: {
        _regs.s[_fd] = _regs.s[_fs] * _regs.s[_ft];
        break;
    }
    case FPU_NEG: {
        _regs.s[_fd] = -_regs.s[_fs];
        break;
    }
    case FPU_NMADD_S: {
        _regs.s[_fd] = -(_regs.s[_fs] * _regs.s[_ft] + _regs.s[_fr]);
        break;
    }
    case FPU_NMSUB_S: {
        _regs.s[_fd] = -(_regs.s[_fs] * _regs.s[_ft] - _regs.s[_fr]);
        break;
    }
    case FPU_RECIP: {
        _regs.s[_fd] = 1.0f / _regs.s[_fs];
        break;
    }
    case FPU_RSQRT: {
        _regs.s[_fd] = 1.0f / sqrt(_regs.s[_fs]);
        break;
    }
    case FPU_SQRT: {
        _regs.s[_fd] = sqrt(_regs.s[_fs]);
        break;
    }
    case FPU_SUB: {
        _regs.s[_fd] = _regs.s[_fs] - _regs.s[_ft];
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
        LOGF("Unimplemented FPU opcode: %02x", GET_INSTR_FPU_OP(ir));
        signal_ex(EX_NOIMP);
        break;
    }
    default: {
        LOGF("Invalid FPU opcode: %02x", GET_INSTR_FPU_OP(ir));
        signal_ex(EX_INVALID);
        break;
    }
    }

    return false;
}

bool fp_cop::get_regs(uint32_t ft, int32_t &res) {
    res = *(int32_t*)&_regs.s[ft];
    return true;
}

void fp_cop::set_regs(uint32_t ft, int32_t res) {
    _regs.s[ft] = *(float*)&res;
}

bool fp_cop::get_next_pc_offset(int32_t &next_pc_offset) {
    next_pc_offset = _next_pc_offset;
    return _has_next_pc_offset;
}

exception_e fp_cop::get_exception() {
    return _prev_ex;
}

void fp_cop::signal_ex(exception_e ex) {
    _prev_ex = ex;
}
