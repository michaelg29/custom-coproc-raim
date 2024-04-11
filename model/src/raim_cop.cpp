
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

bool all_true(bool *arr, int N) {
    for (int i = 0; i < N; i++) {
        if (!arr) return false;
    }
    return true;
}

void clear_all(bool *arr, int N) {
    for (int i = 0; i < N; i++) {
        arr[i] = false;
    }
}

void set_all(bool *arr, int N) {
    for (int i = 0; i < N; i++) {
        arr[i] = true;
    }
}

raim_cop_fu::raim_cop_fu(sc_module_name name, rpu_regs_status_t *regs_status, rpu_regs_t *regs)
    : sc_module(name), _regs_status(regs_status), _regs(regs) {
    SC_THREAD(main);
}

bool raim_cop_fu::issue_instr(uint32_t ir, int32_t rt) {
    // reject if busy
    if (_ir) return false;

    _ir = ir;
    _rt = rt;
    return true;
}

void raim_cop_fu::main() {
    uint32_t ir;
    uint32_t opcode;
    bool ir_valid;
    int i, k, N, n, r, c, q, d;
    uint32_t init_mask;
    uint32_t mask;
    float dotp;

    while (true) {
        i = _regs->N_sv;
        k = _rt / 4;
        N = 3 + _regs->N_const;
        init_mask = 1 << (_regs->N_sv - 1);

        // ==============================
        // ===== INSTRUCTION DECODE =====
        // ==============================

        // take new operation from the instruction queue
        if (_ir) {
            // decode intermediate value
            d = GET_INSTR_REG(ir, 6);

            // execute operation then free used registers
            opcode = GET_INSTR_COP_OP(_ir);
            switch (opcode) {
            case RPU_NEWSV: {
                // increment cursor
                if (_regs->N_sv < RAIM_N_SV_MAX) {
                    _regs->N_sv++;
                }

                // calculate weight
                _regs->w_sqrt[i] = 1.0f / sqrt(_regs->sig_tropo2 + _regs->sig_user2 + _regs->sig_ura2);
                _regs->w_acc_sqrt[i] = 1.0f / sqrt(_regs->sig_tropo2 + _regs->sig_user2 + _regs->sig_ure2);
                DELAY_CC(3); // 3 operations

                LOGF("[%s] Completed SV %d", this->name(), i);
                LOGF("LOS %f %f %f, constellation %d, y %f", _regs->G[i][0], _regs->G[i][1], _regs->G[i][2], _regs->C[i], _regs->y[i]);
                LOGF("st2 %f, sr2 %f, sa2 %f, se2 %f, bn %f", _regs->sig_tropo2, _regs->sig_user2, _regs->sig_ura2, _regs->sig_ure2, _regs->b_nom[i]);
                LOGF("=> W_sqrt %f, W_acc_sqrt %f", _regs->w_sqrt[i], _regs->w_acc_sqrt[i]);

                // free registers
                _regs_status->sig_tropo2    = true;
                _regs_status->sig_user2     = true;
                _regs_status->sig_ura2      = true;
                _regs_status->sig_ure2      = true;
                _regs_status->w_sqrt[i]     = true;
                _regs_status->w_acc_sqrt[i] = true;
                _regs_status->N_sv          = true;
                break;
            }
            case RPU_CALCU: {
                // U <- w_sqrt * G
                // w_sqrt is diagonal => single multiplication for each element
                // result is size of G
                _cpsr |= RPU_MC;
                for (r = _regs->N_sv - 1; r >= 0; r--) {
                    // first 3 columns
                    for (c = 3 - 1; c >= 0; c--) {
                        _regs->u[r][c] = _regs->G[r][c] * _regs->w_sqrt[r];
                    }

                    // constellation columns
                    c = 3 + _regs->C[r];
                    _regs->u[r][c] = _regs->w_sqrt[r];

                    DELAY_CC(4); // 4 total operations
                }

                // free registers
                _regs_status->u = true;
                set_all(_regs_status->G, RAIM_N_SV_MAX);
                set_all(_regs_status->w_sqrt, RAIM_N_SV_MAX);
                break;
            }
            case RPU_INITP: {
                mask = init_mask;
                _cpsr |= RPU_MC;
                for (r = _regs->N_sv - 1; r >= 0; r--, mask >>= 1) {
                    if (!(_regs->idx_ss[k] & mask)) {
                        for (c = 3 + 4; c >= 0; c--) {
                            _regs->s[k][c][r] = 0.0f;
                        }
                        continue;
                    }

                    // first 3 columns
                    for (c = 3 - 1; c >= 0; c--) {
                        _regs->s[k][c][r] = _regs->alpha0 * _regs->u[r][c];
                    }

                    // constellation columns
                    c = 3 + _regs->C[r];
                    _regs->s[k][c][r] = _regs->alpha0 * _regs->w_sqrt[r];

                    DELAY_CC(4); // 4 total operations
                }

                // free registers
                _regs_status->u      = true;
                _regs_status->alpha0 = true;
                _regs_status->s[k]   = true;
                break;
            }
            case RPU_CALCP: {
                // S[k] <- pseudoinv(U)
                LOGF("[%s] Subset %d has indices %03x, mask %03x", this->name(), k, _regs->idx_ss[k], init_mask);
                _cpsr |= RPU_MC;
                for (n = 0; n < RPU_PSEUDO_MAX_IT; n++) {
                    // SPR <- 2I_N - Ut_n U
                    // [7][RAIM_N_SV_MAX] x [RAIM_N_SV_MAX][7]
                    for (c = N - 1; c >= 0; c--) {
                        for (r = N - 1; r >= 0; r--) {
                            dotp = r == c ? 2.0f : 0.0f;
                            mask = init_mask;
                            for (q = _regs->N_sv - 1; q >= 0; q--, mask >>= 1) {
                                if (!(_regs->idx_ss[k] & mask)) {
                                    continue;
                                }

                                dotp -= _regs->s[k][r][q] * _regs->u[q][c];
                            }
                            _regs->spr[r][c] = dotp;
                        }
                    }
                    DELAY_CC(1);

                    // S[k] <- SPR Ut_n
                    mask = init_mask;
                    for (c = _regs->N_sv - 1; c >= 0; c--, mask >>= 1) {
                        if (!(_regs->idx_ss[k] & mask)) {
                            for (r = N - 1; r >= 0; r--) {
                                _regs->s[k][r][c] = 0.0f;
                            }
                            continue;
                        }

                        for (r = N - 1; r >= 0; r--) {
                            dotp = 0.0f;
                            for (q = N - 1; q >= 0; q--) {
                                dotp += _regs->spr[r][q] * _regs->s[k][q][c];
                            }
                            _regs->s[k][r][c] = dotp;
                        }
                    }
                }

                // free registers
                _regs_status->u         = true;
                _regs_status->s[k]      = true;
                _regs_status->idx_ss[k] = true;
                set_all(_regs_status->spr, RAIM_N_SV_MAX);
                break;
            }
            case RPU_WLS: {
                // S[k] <- S[k] w_sqrt
                // w_sqrt is diagonal => single multiplication for each element
                mask = init_mask;
                _cpsr |= RPU_MC;
                for (c = _regs->N_sv - 1; c >= 0; c--, mask >>= 1) {
                    if (_regs->idx_ss[k] & mask) {
                        for (r = N - 1; r >= 0; r--) {
                            _regs->s[k][r][c] = _regs->s[k][r][c] * _regs->w_sqrt[c];
                            DELAY_CC(1);
                        }
                    }
                    else {
                        for (r = N - 1; r >= 0; r--) {
                            _regs->s[k][r][c] = 0.0f;
                        }
                    }
                }

                print_mat((char*)"WLS matrix", k, (float*)_regs->s[k], N, _regs->N_sv, RAIM_N_SV_MAX - _regs->N_sv);
                _cpsr ^= RPU_MC;

                // free registers
                _regs_status->s[k] = true;
                set_all(_regs_status->w_sqrt, RAIM_N_SV_MAX);
                break;
            }
            case RPU_MULY: {
                // SPR[*][d] <- S[k] y
                for (r = N - 1; r >= 0; r--) {
                    dotp = 0.0f;
                    mask = init_mask;
                    for (c = _regs->N_sv - 1; c >= 0; c--, mask >>= 1) {
                        if (!(_regs->idx_ss[k] & mask)) {
                            continue;
                        }

                        dotp += _regs->s[k][r][c] * _regs->y[c];
                        DELAY_CC(1);
                    }
                    _regs->spr[r][d] = dotp;
                }

                // free registers
                _regs_status->s[k] = true;
                _regs_status->y    = true;
                break;
            }
            case RPU_MOVD: {
                // S[k] <- SPR
                mask = init_mask;
                for (c = _regs->N_sv - 1; c >= 0; c--, mask >>= 1) {
                    if (!(_regs->idx_ss[k] & mask)) {
                        for (r = N - 1; r >= 0; r--) {
                            _regs->s[k][r][c] = 0.0f;
                        }
                        continue;
                    }
                    for (r = N - 1; r >= 0; r--) {
                        _regs->s[k][r][c] = _regs->spr[r][c];
                    }
                }

                // free registers
                _regs_status->s[k] = true;
                set_all(_regs_status->spr, RAIM_N_SV_MAX);
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
                    w_sqrt_src = _regs->w_sqrt;
                    dst = _regs->sig_q2[k];
                }
                else if (opcode == RPU_SSVAR) {
                    w_sqrt_src = _regs->w_acc_sqrt;
                    dst = _regs->sig_ssq2[k];
                }

                // SPR <- S[k] w_sqrt_src^{-1}, right-weight
                for (c = _regs->N_sv - 1; c >= 0; c--) {
                    for (r = N - 1; r >= 0; r--) {
                        _regs->spr[r][c] = _regs->s[k][r][c] / w_sqrt_src[c];
                    }
                    //DELAY_CC(1);
                }

                // dst[q] <- (SPR SPR^T)[q][q], q = 1,2,3
                for (q = 2; q >= 0; q--) {
                    // dot product of q-th row of SPR with q-th column of SPR^T
                    // dot product of q-th row of SPR with itself
                    dotp = 0.0f;
                    for (c = _regs->N_sv - 1; c >= 0; c--) {
                        dotp += _regs->spr[q][c] * _regs->spr[q][c];
                    }
                    dst[q] = dotp;
                    //DELAY_CC(1);
                }

                LOGF("[%s] stddev for subset %d is [%f %f %f]", this->name(), k, sqrt(dst[0]), sqrt(dst[1]), sqrt(dst[2]));

                // free registers
                _regs_status->s[k] = true;
                if (opcode == RPU_POSVAR) {
                    set_all(_regs_status->w_sqrt, RAIM_N_SV_MAX);
                }
                else if (opcode == RPU_SSVAR) {
                    set_all(_regs_status->w_acc_sqrt, RAIM_N_SV_MAX);
                }
                break;
            }
            case RPU_BIAS: {
                // bias_q[k][*] <- sum(|S[k][*][i]| * b_nom[i])
                for (q = 2; q >= 0; q--) {
                    dotp = 0.0f;
                    mask = init_mask;
                    for (i = _regs->N_sv - 1; i >= 0; i--, mask >>= 1) {
                        if (!(_regs->idx_ss[k] & mask)) {
                            continue;
                        }
                        dotp += fabs(_regs->s[k][q][i]) * _regs->b_nom[i];
                    }
                    _regs->bias_q[k][q] = dotp;
                    //DELAY_CC(1);
                }
                LOGF("[%s] bias for subset %d is [%f %f %f]", this->name(), k, _regs->bias_q[k][0], _regs->bias_q[k][1], _regs->bias_q[k][2]);

                // free registers
                _regs_status->s[k] = true;
                set_all(_regs_status->b_nom, RAIM_N_SV_MAX);
                break;
            }
            case RPU_CALCSS: {
                // S[k] <- S[k] - S[ss_k_aiv]
                LOGF("[%s] AIV is %d", this->name(), _regs->ss_k_aiv);
                for (c = _regs->N_sv - 1; c >= 0; c--) {
                    for (r = N-1; r >= 0; r--) {
                        _regs->s[k][r][c] = _regs->s[k][r][c] - _regs->s[_regs->ss_k_aiv][r][c];
                    }
                }
                //DELAY_CC(1);

                // free registers
                _regs_status->s[k]               = true;
                _regs_status->s[_regs->ss_k_aiv] = true;
                break;
            }
            case RPU_NEWSS: {
                // increment cursor
                if (_regs->N_ss < RAIM_N_SS_MAX) {
                    _regs->N_ss++;
                }
                break;
            }
            case RPU_TSTG: {
                // global test: fails if any SPR[*][d] > sig_ssq2[k][*] * K_fa[*]
                for (q = 2; q >= 0; q--) {
                    if ((_regs->sig_ssq2[k][q] * _regs->k_fa[q] - fabs(_regs->spr[q][d])) < 0.0f) {
                        _cpsr |= RPU_FD;
                    }
                }
                DELAY_CC(1);

                // free registers
                _regs_status->spr[d]      = true;
                _regs_status->sig_ssq2[k] = true;
                _regs_status->k_fa        = true;
                break;
            }
            case RPU_TSTL: {
                // local test for satellite tst_i
                // fails if y[i] > K_fa,r / W_sqrt[i]
                if ((_regs->k_fa_r / _regs->w_sqrt[_regs->tst_i] - _regs->y[_regs->tst_i]) < 0.0f) {
                    _cpsr |= RPU_FL;
                    _regs->idx_faulty_sv |= (1 << _regs->tst_i);
                }
                DELAY_CC(1);

                // free registers
                _regs_status->y      = true;
                _regs_status->k_fa_r = true;
                set_all(_regs_status->w_sqrt, RAIM_N_SV_MAX);
                break;
            }
            };

            _ir = 0;
        }

        POSEDGE_CPU();
    }
}

uint8_t raim_cop_fu::get_cpsr() {
    return _cpsr;
}

void raim_cop_fu::clear_cpsr() {
    _cpsr = RPU_OKAY;
}

raim_cop::raim_cop(sc_module_name name, uint32_t cop_opcode)
    : sc_module(name), coprocessor_if(cop_opcode), _instr_q(_instrs, RPU_INSTR_Q_SIZE, RPU_INSTR_Q_MASK), _rt_val_q(_rt_vals, RPU_INSTR_Q_SIZE, RPU_INSTR_Q_MASK), _cc_spinning(0) {
    SC_THREAD(main);

    // initial register values
    memset(&_regs_status, 1, sizeof(rpu_regs_status_t));
    memset(&_regs, 0, sizeof(rpu_regs_t));

    // create functional units
    for (int i = 0; i < RPU_N_FUs; i++) {
        _fus[i] = new raim_cop_fu(("rpu_fu_" + std::to_string(i)).c_str(),
            &_regs_status, &_regs);
    }
}

raim_cop::~raim_cop() {
    for (int i = 0; i < RPU_N_FUs; i++) {
        delete _fus[i];
    }
}

bool raim_cop::execute(uint32_t ir, int32_t rt, int32_t &res) {
    bool write_back = false;
    uint32_t rrs = GET_INSTR_REG(ir, 11);
    uint32_t fmt = GET_INSTR_REG(ir, 21);
    uint32_t flags = GET_INSTR_REG(ir, 16);

    // decode instruction format
    switch (fmt) {
    case RPU_FMT_RST: {
        // reset registers
        _rpu_cpsr = RPU_OKAY;
        for (rrs = 0; rrs < RPU_N_FUs; rrs++) {
            _fus[rrs]->clear_cpsr();
        }
        memset(&_regs, 0, sizeof(rpu_regs_t));
        break;
    }
    case RPU_FMT_CLRC: {
        // clear CPSR
        _rpu_cpsr = RPU_OKAY;
        _regs.idx_faulty_sv = 0;
        for (rrs = 0; rrs < RPU_N_FUs; rrs++) {
            _fus[rrs]->clear_cpsr();
        }
        break;
    }
    case RPU_FMT_MF: {
        // move RPU register rrs to res
        write_back = get_regs(rrs, res);
        break;
    }
    case RPU_FMT_BC: {
        // get immediate and left shift
        _next_pc_offset = sign_extend_immd(ir, 2);

        // conditional PC-relative branch
        switch (flags) {
        case RPU_OKAY: {
            _has_next_pc_offset = (_rpu_cpsr & 0b11111) == flags;
            break;
        }
        case RPU_MC: {
            if ((_rpu_cpsr & RPU_MC) > 0) {
                _cc_spinning++;
            }
        }
        default: {
            _has_next_pc_offset = ((_rpu_cpsr & 0b11111) & flags) > 0;
            break;
        }
        };
        LOGF("[%s] cond branch (flag %02x), cpsr is %02x -> %d", this->name(), flags, _rpu_cpsr, _has_next_pc_offset);

        break;
    }
    case RPU_FMT_MT:
    case RPU_FMT_CP: {
        // spin while not able to enqueue
        while (!_instr_q.enqueue(ir)) {
            _cc_spinning++;
            POSEDGE_CPU();
        }

        // enqueue register value
        LOGF("[%s] enqueuing %08x with instr %08x", this->name(), rt, ir);
        _rt_val_q.enqueue(rt);
        break;
    }
    default: {
        // all other formats must enqueue
        // spin while not able to enqueue
        while (!_instr_q.enqueue(ir)) {
            _cc_spinning++;
            POSEDGE_CPU();
        }
        break;
    }
    };

    return write_back;
}

bool raim_cop::get_regs(uint32_t rt, int32_t &res) {
    // decode register address
    switch (rt) {
    case RPU_VR_IDX: res = _regs.idx_faulty_sv; break;
    default: return false;
    };

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
        if (rt == RPU_VR_Yi + 15) {
            _regs.y[i] = fres;
        }
        else if (rt >= RPU_VR_Yi) {
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
    bool ret = _has_next_pc_offset;
    next_pc_offset = _next_pc_offset;
    _has_next_pc_offset = false;
    return ret;
}

void raim_cop::main() {
    uint32_t ir, opcode, rrs;
    bool ir_valid, ir_issued;
    int i, k, N, d;
    int32_t rt;

    // initial values
    ir = 0;

    while (true) {
        // default values
        if (_instr_q.is_full()) {
            _prev_ex = EX_COP_UNUSABLE;
        }
        else {
            _prev_ex = EX_NONE;
        }
        ir_valid = true;
        ir_issued = false;

        i = _regs.N_sv;
        k = _regs.N_ss;
        N = 3 + _regs.N_const;

        // ==============================
        // ===== INSTRUCTION DECODE =====
        // ==============================

        // assert or de-assert full flag
        _rpu_cpsr = RPU_OKAY;
        if (_instr_q.is_full()) {
            _rpu_cpsr |= RPU_FULL;
        }
        else {
            _rpu_cpsr &= ~RPU_FULL;
        }

        // decode immediate value
        d = GET_INSTR_REG(ir, 6);

        // take new operation from the instruction queue
        if (!ir && _instr_q.dequeue(ir)) {
            // decode instruction format
            switch (GET_INSTR_REG(ir, 21)) {
            case RPU_FMT_NONE: {
                break;
            }
            case RPU_FMT_MT: {
                // move rt to RPU register rrs
                rrs = GET_INSTR_REG(ir, 11);
                _rt_val_q.dequeue(rt);
                set_regs(rrs, rt);
                break;
            }
            case RPU_FMT_CP: {
                // dequeue intermediate value
                _rt_val_q.dequeue(rt);
                break;
            }
            default: {
                LOGF("Invalid RPU format: %02x", GET_INSTR_REG(ir, 21));
                signal_ex(EX_INVALID);
                ir_valid = false;
                break;
            }
            };
        }

        if (ir && ir_valid) {
            // decode operation, ensure no data dependency violations
            opcode = GET_INSTR_COP_OP(ir);
            switch (opcode) {
            case RPU_NOP: {
                ir = 0;
                break;
            }
            case RPU_NEWSV: {
                if (_regs_status.sig_user2     &&
                    _regs_status.sig_tropo2    &&
                    _regs_status.sig_ura2      &&
                    _regs_status.sig_ure2      &&
                    _regs_status.w_sqrt[i]     &&
                    _regs_status.w_acc_sqrt[i] &&
                    _regs_status.N_sv) {
                    _regs_status.sig_user2     = false;
                    _regs_status.sig_tropo2    = false;
                    _regs_status.sig_ura2      = false;
                    _regs_status.sig_ure2      = false;
                    _regs_status.w_sqrt[i]     = false;
                    _regs_status.w_acc_sqrt[i] = false;
                    _regs_status.N_sv          = false;
                    ir_issued = true;
                }
                break;
            }
            case RPU_CALCU: {
                // U <- w_sqrt * G
                if (_regs_status.u &&
                    all_true(_regs_status.G, RAIM_N_SV_MAX) &&
                    all_true(_regs_status.w_sqrt, RAIM_N_SV_MAX)) {
                    _regs_status.u = false;
                    clear_all(_regs_status.G, RAIM_N_SV_MAX);
                    clear_all(_regs_status.w_sqrt, RAIM_N_SV_MAX);
                    ir_issued = true;
                }
                break;
            }
            case RPU_INITP: {
                // S[k] <- alpha * U
                if (_regs_status.u      &&
                    _regs_status.alpha0 &&
                    _regs_status.s[k]) {
                    _regs_status.u      = false;
                    _regs_status.alpha0 = false;
                    _regs_status.s[k]   = false;
                    ir_issued = true;
                }
                break;
            }
            case RPU_CALCP: {
                // S[k] <- pseudoinv(U)
                if (_regs_status.u         &&
                    _regs_status.s[k]      &&
                    _regs_status.idx_ss[k] &&
                    all_true(_regs_status.spr, RAIM_N_SV_MAX)) {
                    _regs_status.u         = false;
                    _regs_status.s[k]      = false;
                    _regs_status.idx_ss[k] = false;
                    clear_all(_regs_status.spr, RAIM_N_SV_MAX);
                    ir_issued = true;
                }
                break;
            }
            case RPU_WLS: {
                // S[k] <- S[k] w_sqrt
                if (_regs_status.s[k] &&
                    all_true(_regs_status.w_sqrt, RAIM_N_SV_MAX)) {
                    _regs_status.s[k] = false;
                    clear_all(_regs_status.w_sqrt, RAIM_N_SV_MAX);
                    ir_issued = true;
                }
                break;
            }
            case RPU_MULY: {
                // SPR[*][d] <- S[k] y
                if (_regs_status.s[k] &&
                    _regs_status.y) {
                    _regs_status.s[k] = false;
                    _regs_status.y = false;
                    ir_issued = true;
                }
                break;
            }
            case RPU_MOVD: {
                // S[k] <- SPR
                if (_regs_status.s[k] &&
                    all_true(_regs_status.spr, RAIM_N_SV_MAX)) {
                    _regs_status.s[k] = false;
                    clear_all(_regs_status.spr, RAIM_N_SV_MAX);
                    ir_issued = true;
                }
                break;
            }
            case RPU_POSVAR: {
                // RPU_POSVAR: sig_q2[k][*] <- ((S[k] W_sqrt^{-1})(W_sqrt^{-1} S[k]^T)[*][*]
                if (_regs_status.s[k] &&
                    all_true(_regs_status.w_sqrt, RAIM_N_SV_MAX)) {
                    _regs_status.s[k] = true;
                    clear_all(_regs_status.w_sqrt, RAIM_N_SV_MAX);
                    ir_issued = true;
                }
                break;
            }
            case RPU_SSVAR: {
                // RPU_SSVAR: sig_q_ss2[k][*] <- ((S[k] W_acc_sqrt^{-1})(W_acc_sqrt^{-1} S[k]^T)[*][*]
                if (_regs_status.s[k] &&
                    all_true(_regs_status.w_acc_sqrt, RAIM_N_SV_MAX)) {
                    _regs_status.s[k] = false;
                    clear_all(_regs_status.w_acc_sqrt, RAIM_N_SV_MAX);
                    ir_issued = true;
                }
                break;
            }
            case RPU_BIAS: {
                // bias_q[k][*] <- sum(|S[k][*][i]| * b_nom[i])
                if (_regs_status.s[k] &&
                    all_true(_regs_status.b_nom, RAIM_N_SV_MAX)) {
                    _regs_status.s[k] = false;
                    clear_all(_regs_status.b_nom, RAIM_N_SV_MAX);
                    ir_issued = true;
                }
                break;
            }
            case RPU_CALCSS: {
                // S[k] <- S[k] - S[ss_k_aiv]
                if (_regs_status.s[k]              &&
                    _regs_status.s[_regs.ss_k_aiv]) {
                    _regs_status.s[k]              = false;
                    _regs_status.s[_regs.ss_k_aiv] = false;
                    ir_issued = true;
                }
                break;
            }
            case RPU_NEWSS: {
                ir_issued = true;
                break;
            }
            case RPU_TSTG: {
                // global test: fails if any SPR[*][d] > sig_ssq2[k][*] * K_fa[*]
                if (_regs_status.spr[d]      &&
                    _regs_status.sig_ssq2[k] &&
                    _regs_status.k_fa) {
                    _regs_status.spr[d]      = false;
                    _regs_status.sig_ssq2[k] = false;
                    _regs_status.k_fa        = false;
                    ir_issued = true;
                }
                break;
            }
            case RPU_TSTL: {
                // local test for satellite tst_i
                // fails if y[i] > K_fa,r / W_sqrt[i]
                if (_regs_status.y      &&
                    _regs_status.k_fa_r &&
                    all_true(_regs_status.w_sqrt, RAIM_N_SV_MAX)) {
                    _regs_status.y      = false;
                    _regs_status.k_fa_r = false;
                    clear_all(_regs_status.w_sqrt, RAIM_N_SV_MAX);
                    ir_issued = true;
                }
                break;
            }
            default: {
                LOGF("Invalid RPU opcode: %02x", opcode);
                signal_ex(EX_INVALID);
                ir_valid = false;
                break;
            }
            };

            // can issue IR with no data dependency violations
            if (ir_valid && ir_issued) {
                // look for open FU
                for (i = 0; i < RPU_N_FUs; i++) {
                    if (_fus[i]->issue_instr(ir, rt)) {
                        ir = 0;
                        rt = 0;
                        break;
                    }
                    else {
                        LOGF("[%s] FU %d busy", this->name(), i);
                    }
                }
                if (i == RPU_N_FUs) {
                    ir_issued = false;
                }
            }
        }

        // check condition flags
        for (i = 0; i < RPU_N_FUs; i++) {
            _rpu_cpsr |= _fus[i]->get_cpsr();
        }

        POSEDGE_CPU();
    }
}

void raim_cop::print_statistics() {
    LOGF("\n=====\nStatistics for %s\n=====", this->name());
    printf("Number of cycles stalled: %d\n", _cc_spinning);

}
