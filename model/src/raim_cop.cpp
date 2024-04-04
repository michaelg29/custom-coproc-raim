
#include "raim_cop.h"

#include "system.h"
#include "isa.h"

#include "systemc.h"

#include <math.h>

raim_cop::raim_cop(sc_module_name name, uint32_t cop_opcode)
    : sc_module(name), coprocessor_if(cop_opcode), _instr_q(_instrs, RPU_INSTR_Q_SIZE, RPU_INSTR_Q_MASK), _instr_q_overflow(false) {
    SC_THREAD(main);

    // initial register values
    memset(&_regs, 0, sizeof(rpu_regs_t));
}

raim_cop::~raim_cop() {

}

bool raim_cop::execute(uint32_t ir, int32_t rt, int32_t &res) {
    _instr_q_overflow = !_instr_q.enqueue(ir);

    return false;
}

bool raim_cop::get_regs(uint32_t rt, int32_t &res) {
    return false;
}

void raim_cop::set_regs(uint32_t rt, int32_t res) {
    float fres = *(float*)&res;
    uint8_t bres = (uint8_t)res;
    int i = _regs.N_sv;
    int j;
    int k;

    // decode register address
    switch (rt) {
    case RPU_VR_LX: _regs.G[i][0] = fres; break;
    case RPU_VR_LY: _regs.G[i][1] = fres; break;
    case RPU_VR_LZ: _regs.G[i][2] = fres; break;
    case RPU_VR_C:    {
        _regs.C[i] = bres;
        // determine if new constellation
        if (bres >= _regs.N_const) {
            _regs.N_const++;
        }
        break;
    }

    case RPU_VR_ST: _regs.sig_tropo2 = fres; break;
    case RPU_VR_SR:  _regs.sig_user2 = fres; break;
    case RPU_VR_SA:   _regs.sig_ura2 = fres; break;
    case RPU_VR_SE:   _regs.sig_ure2 = fres; break;

    case RPU_VR_BN:  _regs.b_nom[i] = fres; break;
    case RPU_VR_KX:  _regs.k_fa[0] = fres; break;
    case RPU_VR_KY:  _regs.k_fa[1] = fres; break;
    case RPU_VR_KZ:  _regs.k_fa[2] = fres; break;
    case RPU_VR_KR: _regs.k_fa_r = fres; break;
    };
}

bool raim_cop::get_condition_code(uint8_t &cc) {
    cc = _condition_code;
    return true;
}

bool raim_cop::get_next_pc_offset(int32_t &next_pc_offset) {
    next_pc_offset = _next_pc_offset;
    return _has_next_pc_offset;
}

void raim_cop::main() {
    uint32_t ir;
    bool ir_valid;
    int i;

    while (true) {
        // default values
        if (_instr_q_overflow) {
            _prev_ex = EX_COP_UNUSABLE;
            _instr_q_overflow = false;
        }
        else {
            _prev_ex = EX_NONE;
        }
        _has_next_pc_offset = false;

        i = _regs.N_sv;

        // ==============================
        // ===== INSTRUCTION DECODE =====
        // ==============================

        // take new operation from the instruction queue
        if (_instr_q.dequeue(ir)) {
            switch (GET_INSTR_COP_OP(ir)) {
            case RPU_RST: {
                // reset registers
                memset(&_regs, 0, sizeof(rpu_regs_t));
                break;
            }
            case RPU_NEWSV: {
                // calculate weight
                _regs.w_sqrt[i] = 1.0f / sqrt(_regs.sig_tropo2 + _regs.sig_user2 + _regs.sig_ura2);
                _regs.c_acc[i] = _regs.sig_tropo2 + _regs.sig_user2 + _regs.sig_ure2;

                LOGF("[%s] Completed SV %d", this->name(), i);
                LOGF("LOS %f %f %f, constellation %d", _regs.G[i][0], _regs.G[i][1], _regs.G[i][2], _regs.C[i]);
                LOGF("st2 %f, sr2 %f, sa2 %f, se2 %f, bn %f", _regs.sig_tropo2, _regs.sig_user2, _regs.sig_ura2, _regs.sig_ure2, _regs.b_nom[i]);
                LOGF("=> W_sqrt %f, c_acc %f", _regs.w_sqrt[i], _regs.c_acc[i]);

                // increment cursor
                if (_regs.N_sv < RAIM_N_SV_MAX-1) {
                    _regs.N_sv++;
                }
                break;
            }
            default: {
                LOGF("Invalid RPU opcode: %02x", GET_INSTR_COP_OP(ir));
                signal_ex(EX_INVALID);
                break;
            }
            }
        }

        POSEDGE_CPU();
    }
}
