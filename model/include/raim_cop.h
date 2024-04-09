
#include "system.h"

#include "systemc.h"

#include "cpu.h"
#include "memory.h"
#include "isa.h"
#include "queue.hpp"

#ifndef RAIM_COP_H
#define RAIM_COP_H

// ===========================
// ===== RAIM PARAMETERS =====
// ===========================
#define RAIM_N_SV_MAX 16
#define RAIM_N_SS_MAX 17 // 1 (AIV) + 16 (1 faulty satellite)

// =================================
// ===== RPU DESIGN PARAMETERS =====
// =================================
#define RPU_INSTR_Q_SIZE 16
#define RPU_INSTR_Q_MASK 0xf
#define RPU_PSEUDO_MAX_IT 10

/** Concrete register collection. */
typedef struct {
    float alpha0;
    float G[RAIM_N_SV_MAX][3];
    uint8_t C[RAIM_N_SV_MAX];
    uint8_t N_sv;
    uint8_t N_const;
    uint8_t N_ss;
    uint8_t ss_k_aiv;
    float sig_tropo2;
    float sig_user2;
    float sig_ura2;
    float sig_ure2;
    float sig_ura2_all[RAIM_N_SV_MAX];
    float b_nom[RAIM_N_SV_MAX];
    float w_sqrt[RAIM_N_SV_MAX];
    float w_acc_sqrt[RAIM_N_SV_MAX];
    float u[RAIM_N_SV_MAX][7];
    uint32_t idx_ss[RAIM_N_SS_MAX];
    float s[RAIM_N_SS_MAX][7][RAIM_N_SV_MAX];
    float spr[7][RAIM_N_SV_MAX];
    float y[RAIM_N_SV_MAX];
    float k_fa[3];
    float k_fa_r;
    float sig_q2[RAIM_N_SS_MAX][3];
    float bias_q[RAIM_N_SS_MAX][3];
    float ss_mag[RAIM_N_SS_MAX];
    float sig_ssq2[RAIM_N_SS_MAX][3];
} rpu_regs_t;

/** Concrete RAIM coprocessor. */
class raim_cop : public sc_module, public coprocessor_if {

    public:

        /** Constructor. */
        SC_HAS_PROCESS(raim_cop);
        raim_cop(sc_module_name name, uint32_t cop_opcode);

        /** Destructor. */
        ~raim_cop();

        /** coprocessor_if overrides. */
        bool execute(uint32_t ir, int32_t rt, int32_t &res);
        bool get_regs(uint32_t rt, int32_t &res);
        void set_regs(uint32_t rt, int32_t res);
        bool get_condition_code(uint8_t &cc);
        bool get_next_pc_offset(int32_t &next_pc_offset);

    private:

        /** Intermediate values. */
        bool _has_next_pc_offset;

        /** Instruction queue. */
        bool _instr_q_overflow;
        uint32_t _instrs[RPU_INSTR_Q_SIZE];
        queue<uint32_t> _instr_q;

        /** Registers. */
        rpu_regs_t _regs;
        uint8_t _rpu_cpsr;
        int32_t _next_pc_offset;

        void main();

};

#endif // RAIM_COP_H
