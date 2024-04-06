
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
    bool write_back = false;
    uint32_t rrs = GET_INSTR_REG(ir, 11);

    switch (GET_INSTR_COP_OP(ir)) {
    case RPU_RST: {
        // reset registers
        memset(&_regs, 0, sizeof(rpu_regs_t));
        break;
    }
    case RPU_MF: {
        // move RPU register rrs to res
        write_back = get_regs(rrs, res);
        break;
    }
    case RPU_MT: {
        // move rt to RPU register rrs
        set_regs(rrs, rt);
        break;
    }
    default: {
        // enqueue instruction
        _instr_q_overflow = !_instr_q.enqueue(ir);
        break;
    }
    }

    return false;
}

bool raim_cop::get_regs(uint32_t rt, int32_t &res) {
    // decode register address
    switch (rt) {
    case RPU_VR_EXC: res = _prev_ex; break;
    default: return false;
    }

    return true;
}

void raim_cop::set_regs(uint32_t rt, int32_t res) {
    float fres = *(float*)&res;
    uint8_t bres = (uint8_t)res;
    int i = _regs.N_sv;
    int j;
    int k = _regs.N_ss;

    // decode register address
    switch (rt) {
    case RPU_VR_AL0: _regs.alpha0 = fres; break;

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

    case RPU_VR_ST2: _regs.sig_tropo2 = fres; break;
    case RPU_VR_SR2: _regs.sig_user2 = fres; break;
    case RPU_VR_SA2: _regs.sig_ura2 = fres; break;
    case RPU_VR_SE2: _regs.sig_ure2 = fres; break;
    case RPU_VR_BN:  _regs.b_nom[i] = fres; break;

    case RPU_VR_IDX: _regs.idx_ss[k] = (uint32_t)res; break;

    case RPU_VR_KX:  _regs.k_fa[0] = fres; break;
    case RPU_VR_KY:  _regs.k_fa[1] = fres; break;
    case RPU_VR_KZ:  _regs.k_fa[2] = fres; break;
    case RPU_VR_KR:  _regs.k_fa_r = fres; break;
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
    int i, k, N, n, r, c, q;
    uint32_t mask;
    float dotp;

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
        k = _regs.N_ss;
        N = 3 + _regs.N_const;

        // ==============================
        // ===== INSTRUCTION DECODE =====
        // ==============================

        // take new operation from the instruction queue
        if (_instr_q.dequeue(ir)) {
            switch (GET_INSTR_COP_OP(ir)) {
            case RPU_NEWSV: {
                // calculate weight
                _regs.w_sqrt[i] = 1.0f / sqrt(_regs.sig_tropo2 + _regs.sig_user2 + _regs.sig_ura2);
                _regs.c_acc[i] = _regs.sig_tropo2 + _regs.sig_user2 + _regs.sig_ure2;

                LOGF("[%s] Completed SV %d", this->name(), i);
                LOGF("LOS %f %f %f, constellation %d", _regs.G[i][0], _regs.G[i][1], _regs.G[i][2], _regs.C[i]);
                LOGF("st2 %f, sr2 %f, sa2 %f, se2 %f, bn %f", _regs.sig_tropo2, _regs.sig_user2, _regs.sig_ura2, _regs.sig_ure2, _regs.b_nom[i]);
                LOGF("=> W_sqrt %f, c_acc %f", _regs.w_sqrt[i], _regs.c_acc[i]);

                // increment cursor
                if (_regs.N_sv < RAIM_N_SV_MAX) {
                    _regs.N_sv++;
                }
                break;
            }
            case RPU_CALCU: {
                // U <- w_sqrt * G
                // w_sqrt is diagonal => single multiplication for each element
                // result is size of G
                for (r = _regs.N_sv - 1; r >= 0; r--) {
                    // first 3 columns
                    for (c = 3 - 1; c >= 0; c--) {
                        _regs.u[r][c] = _regs.G[r][c] * _regs.w_sqrt[r];
                    }

                    // constellation columns
                    c = 3 + _regs.C[r];
                    _regs.u[r][c] = _regs.w_sqrt[r];
                }
                break;
            }
            case RPU_INITP: {
                mask = 1 << (_regs.N_sv - 1);
                for (r = _regs.N_sv - 1; r >= 0; r--, mask >>= 1) {
                    if (!(_regs.idx_ss[k] & mask)) {
                        continue;
                    }

                    // first 3 columns
                    for (c = 3 - 1; c >= 0; c--) {
                        _regs.s[k][c][r] = _regs.alpha0 * _regs.u[r][c];
                    }

                    // constellation columns
                    c = 3 + _regs.C[r];
                    _regs.s[k][c][r] = _regs.alpha0 * _regs.w_sqrt[r];
                }
                break;
            }
            case RPU_CALCP: {
                // S[k] <- pseudoinv(U)
                for (n = 0; n < RPU_PSEUDO_MAX_IT; n++) {
                    // SPR <- 2I_N - Ut_n U
                    mask = 1 << (_regs.N_sv - 1);
                    for (c = _regs.N_sv - 1; c >= 0; c--, mask >>= 1) {
                        if (!(_regs.idx_ss[k] & mask)) {
                            continue;
                        }

                        for (r = N - 1; r >= 0; r--) {
                            dotp = r == c ? 2.0f : 0.0f;
                            for (q = _regs.N_sv - 1; q >= 0; q--) {
                                dotp -= _regs.s[k][r][q] * _regs.u[q][c];
                            }
                            _regs.spr[r][c] = dotp;
                        }
                    }

                    // S[k] <- SPR Ut_n
                    mask = 1 << (_regs.N_sv - 1);
                    for (c = _regs.N_sv - 1; c >= 0; c--, mask >>= 1) {
                        if (!(_regs.idx_ss[k] & mask)) {
                            continue;
                        }

                        for (r = N - 1; r >= 0; r--) {
                            dotp = 0.0f;
                            for (q = N - 1; q >= 0; q--) {
                                dotp += _regs.spr[r][q] * _regs.s[k][q][c];
                            }
                            _regs.s[k][r][c] = dotp;
                        }
                    }
                }
                break;
            }
            case RPU_WLS: {
                // S[k] <- S[k] w_sqrt
                // w_sqrt is diagonal => single multiplication for each element
                mask = 1 << (_regs.N_sv - 1);
                for (c = _regs.N_sv - 1; c >= 0; c--, mask >>= 1) {
                    if (!(_regs.idx_ss[k] & mask)) {
                        continue;
                    }

                    for (r = 3 + _regs.N_const - 1; r >= 0; r--) {
                        _regs.s[k][r][c] = _regs.w_sqrt[c] * _regs.s[k][r][c];
                    }
                }

                LOGF("[%s] LS matrix for subset %d (%08x): [", this->name(), k, _regs.idx_ss[k]);
                for (r = 0; r < 3 + _regs.N_const; r++) {
                    for (c = 0; c < _regs.N_sv; c++) {
                        printf("%f ", _regs.s[k][r][c]);
                    }
                    printf("\n");
                }
                printf("]\n");

                // increment cursor
                if (_regs.N_ss < RAIM_N_SS_MAX) {
                    _regs.N_ss++;
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
