
#include "raim_cop.h"

#include "system.h"
#include "isa.h"

#include "systemc.h"

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
    int i = _regs.cur_sat_idx;
    int j;
    int k;

    switch (rt) {
    case RPU_VR_LOSX: _regs.G[i][0] = fres; break;
    case RPU_VR_LOSY: _regs.G[i][1] = fres; break;
    case RPU_VR_LOSZ: _regs.G[i][2] = fres; break;
    case RPU_VR_C:    _regs.C[i] = (uint8_t)res; break;

    case RPU_VR_SIGMA_TROPO2: _regs.sig_tropo2 = fres; break;
    case RPU_VR_SIGMA_USER2:  _regs.sig_user2 = fres; break;
    case RPU_VR_SIGMA_URA2:   _regs.sig_ura2 = fres; break;
    case RPU_VR_SIGMA_URE2:   _regs.sig_ure2 = fres; break;

    case RPU_VR_B_NOM:  _regs.b_nom[i] = fres; break;
    case RPU_VR_K_FAX:  _regs.k_fa[0] = fres; break;
    case RPU_VR_K_FAY:  _regs.k_fa[1] = fres; break;
    case RPU_VR_K_FAZ:  _regs.k_fa[2] = fres; break;
    case RPU_VR_K_FA_R: _regs.k_fa_r = fres; break;
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

        // ==============================
        // ===== INSTRUCTION DECODE =====
        // ==============================

        // decode operation from the instruction queue
        if (_instr_q.dequeue(ir)) {
            switch (GET_INSTR_COP_OP(ir)) {
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
