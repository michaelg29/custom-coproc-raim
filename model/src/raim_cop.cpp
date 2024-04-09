
#include "raim_cop.h"

#include "system.h"
#include "isa.h"

#include "systemc.h"

#include <math.h>

void print_mat(char *mat_name, int id, float *mat, int R, int C, int rowpad) {
    LOGF("%s (id %d): [", mat_name, id);
    int i = 0;
    for (int r = 0; r < R; r++, i += rowpad) {
        for (int c = 0; c < C; c++, i++) {
            printf("%f ",mat[i]);
        }
        printf("\n");
    }
    printf("]\n");
}

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

    default: {
        if (rt >= RPU_VR_Yi) {
            i = rt - RPU_VR_Yi;
            _regs.y[i] = fres;
        }
        break;
    }
    };
}

bool raim_cop::get_condition_code(uint8_t &cc) {
    cc = _rpu_cpsr;
    return true;
}

bool raim_cop::get_next_pc_offset(int32_t &next_pc_offset) {
    next_pc_offset = _next_pc_offset;
    return _has_next_pc_offset;
}

void raim_cop::main() {
    uint32_t ir;
    uint32_t opcode;
    bool ir_valid;
    int i, k, N, n, r, c, q, d;
    uint32_t init_mask;
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
        init_mask = 1 << (_regs.N_sv - 1);

        // ==============================
        // ===== INSTRUCTION DECODE =====
        // ==============================

        // take new operation from the instruction queue
        if (_instr_q.dequeue(ir)) {
            // decode intermediate value
            d = GET_INSTR_REG(ir, 6);

            // decode operation
            opcode = GET_INSTR_COP_OP(ir);
            switch (opcode) {
            case RPU_NEWSV: {
                // calculate weight
                _regs.w_sqrt[i] = 1.0f / sqrt(_regs.sig_tropo2 + _regs.sig_user2 + _regs.sig_ura2);
                _regs.w_acc_sqrt[i] = 1.0f / sqrt(_regs.sig_tropo2 + _regs.sig_user2 + _regs.sig_ure2);
                _regs.sig_ura2_all[i] = _regs.sig_ura2;

                LOGF("[%s] Completed SV %d", this->name(), i);
                LOGF("LOS %f %f %f, constellation %d", _regs.G[i][0], _regs.G[i][1], _regs.G[i][2], _regs.C[i]);
                LOGF("st2 %f, sr2 %f, sa2 %f, se2 %f, bn %f", _regs.sig_tropo2, _regs.sig_user2, _regs.sig_ura2, _regs.sig_ure2, _regs.b_nom[i]);
                LOGF("=> W_sqrt %f, W_acc_sqrt %f", _regs.w_sqrt[i], _regs.w_acc_sqrt[i]);

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
                mask = init_mask;
                for (r = _regs.N_sv - 1; r >= 0; r--, mask >>= 1) {
                    if (!(_regs.idx_ss[k] & mask)) {
                        for (c = 3 + 4; c >= 0; c--) {
                            _regs.s[k][c][r] = 0.0f;
                        }
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
                LOGF("[%s] Subset %d has indices %03x, mask %03x", this->name(), k, _regs.idx_ss[k], 1 << (_regs.N_sv - 1));
                for (n = 0; n < RPU_PSEUDO_MAX_IT; n++) {
                    // SPR <- 2I_N - Ut_n U
                    // [7][RAIM_N_SV_MAX] x [RAIM_N_SV_MAX][7]
                    for (c = N - 1; c >= 0; c--) {
                        for (r = N - 1; r >= 0; r--) {
                            dotp = r == c ? 2.0f : 0.0f;
                            mask = init_mask;
                            for (q = _regs.N_sv - 1; q >= 0; q--, mask >>= 1) {
                                if (!(_regs.idx_ss[k] & mask)) {
                                    continue;
                                }

                                dotp -= _regs.s[k][r][q] * _regs.u[q][c];
                            }
                            _regs.spr[r][c] = dotp;
                        }
                    }

                    // S[k] <- SPR Ut_n
                    mask = init_mask;
                    for (c = _regs.N_sv - 1; c >= 0; c--, mask >>= 1) {
                        if (!(_regs.idx_ss[k] & mask)) {
                            for (r = N - 1; r >= 0; r--) {
                                _regs.s[k][r][c] = 0.0f;
                            }
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
                mask = init_mask;
                for (c = _regs.N_sv - 1; c >= 0; c--, mask >>= 1) {
                    if (_regs.idx_ss[k] & mask) {
                        for (r = N - 1; r >= 0; r--) {
                            _regs.s[k][r][c] = _regs.s[k][r][c] * _regs.w_sqrt[c];
                        }
                    }
                    else {
                        for (r = N - 1; r >= 0; r--) {
                            _regs.s[k][r][c] = 0.0f;
                        }
                    }
                }

                print_mat((char*)"WLS matrix", k, (float*)_regs.s[k], N, _regs.N_sv, RAIM_N_SV_MAX - _regs.N_sv);
                break;
            }
            case RPU_MULY: {
                // SPR[*][d] <- S[k] y
                for (r = N - 1; r >= 0; r--) {
                    dotp = 0.0f;
                    mask = init_mask;
                    for (c = _regs.N_sv - 1; c >= 0; c--, mask >>= 1) {
                        if (!(_regs.idx_ss[k] & mask)) {
                            continue;
                        }

                        dotp += _regs.s[k][r][c] * _regs.y[c];
                    }
                    _regs.spr[r][d] = dotp;
                }
                break;
            }
            case RPU_MOVD: {
                // S[k] <- SPR
                mask = init_mask;
                for (c = _regs.N_sv - 1; c >= 0; c--, mask >>= 1) {
                    if (!(_regs.idx_ss[k] & mask)) {
                        for (r = N - 1; r >= 0; r--) {
                            _regs.s[k][r][c] = 0.0f;
                        }
                        continue;
                    }
                    for (r = N - 1; r >= 0; r--) {
                        _regs.s[k][r][c] = _regs.spr[r][c];
                    }
                }
                break;
            }
            case RPU_POSVAR:
            case RPU_SSVAR: {
                // RPU_POSVAR: sig_q2[k][*] <- ((S[k] W_sqrt^{-1})(W_sqrt^{-1} S[k]^T)[*][*]
                // RPU_SSVAR: sig_q_ss2[k][*] <- ((S[k] W_acc_sqrt^{-1})(W_acc_sqrt^{-1} S[k]^T)[*][*]

                // get input and output pointers
                float *w_sqrt_src = NULL;
                float *dst = NULL;
                if (opcode == RPU_POSVAR) {
                    w_sqrt_src = _regs.w_sqrt;
                    dst = _regs.sig_q2[k];
                }
                else if (opcode == RPU_SSVAR) {
                    w_sqrt_src = _regs.w_acc_sqrt;
                    dst = _regs.sig_ssq2[k];
                }

                // SPR <- S[k] w_sqrt_src^{-1}, right-weight
                for (c = _regs.N_sv - 1; c >= 0; c--) {
                    for (r = N - 1; r >= 0; r--) {
                        _regs.spr[r][c] = _regs.s[k][r][c] / w_sqrt_src[c];
                    }
                }

                // dst[q] <- (SPR SPR^T)[q][q], q = 1,2,3
                for (q = 2; q >= 0; q--) {
                    // dot product of q-th row of SPR with q-th column of SPR^T
                    // dot product of q-th row of SPR with itself
                    dotp = 0.0f;
                    for (c = _regs.N_sv - 1; c >= 0; c--) {
                        dotp += _regs.spr[q][c] * _regs.spr[q][c];
                    }
                    dst[q] = dotp;
                }

                LOGF("[%s] stddev for subset %d is [%f %f %f]", this->name(), k, sqrt(dst[0]), sqrt(dst[1]), sqrt(dst[2]));

                break;
            }
            case RPU_BIAS: {
                // bias_q[k][*] <- sum(|S[k][*][i]| * b_nom[i])
                for (q = 2; q >= 0; q--) {
                    dotp = 0.0f;
                    mask = init_mask;
                    for (i = _regs.N_sv - 1; i >= 0; i--, mask >>= 1) {
                        if (!(_regs.idx_ss[k] & mask)) {
                            continue;
                        }
                        dotp += fabs(_regs.s[k][q][i]) * _regs.b_nom[i];
                    }
                    _regs.bias_q[k][q] = dotp;
                }
                LOGF("[%s] bias for subset %d is [%f %f %f]", this->name(), k, _regs.bias_q[k][0], _regs.bias_q[k][1], _regs.bias_q[k][2]);
                break;
            }
            case RPU_CALCSS: {
                // S[k] <- S[k] - S[ss_k_aiv]
                LOGF("[%s] AIV is %d", this->name(), _regs.ss_k_aiv);
                for (c = _regs.N_sv - 1; c >= 0; c--) {
                    for (r = N-1; r >= 0; r--) {
                        _regs.s[k][r][c] = _regs.s[k][r][c] - _regs.s[_regs.ss_k_aiv][r][c];
                    }
                }
                break;
            }
            case RPU_SS: {
                // ss_mag[k] <- mag(SPR[*][d])
                dotp = 0.0f;
                for (q = 2; q >= 0; q--) {
                    dotp += _regs.spr[q][d] * _regs.spr[q][d];
                }
                _regs.ss_mag[k] = sqrt(dotp);
                break;
            }
            case RPU_NEWSS: {
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
