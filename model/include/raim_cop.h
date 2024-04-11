
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
#define RPU_INSTR_Q_SIZE  16
#define RPU_INSTR_Q_MASK  0xf
#define RPU_PSEUDO_MAX_IT 10
#define RPU_N_FUs         2

/** Concrete register collection. */
typedef struct {
    uint32_t tst_i;
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
    float b_nom[RAIM_N_SV_MAX];
    float w_sqrt[RAIM_N_SV_MAX];
    float w_acc_sqrt[RAIM_N_SV_MAX];
    float u[RAIM_N_SV_MAX][7];
    uint32_t idx_ss[RAIM_N_SS_MAX];
    uint32_t idx_faulty_sv;
    float s[RAIM_N_SS_MAX][7][RAIM_N_SV_MAX];
    float spr[7][RAIM_N_SV_MAX];
    float y[RAIM_N_SV_MAX];
    float k_fa[3];
    float k_fa_r;
    float sig_q2[RAIM_N_SS_MAX][3];
    float bias_q[RAIM_N_SS_MAX][3];
    float sig_ssq2[RAIM_N_SS_MAX][3];
} rpu_regs_t;

/** Busy flags attached to each register. */
typedef struct {
    bool tst_i;
    bool alpha0;
    bool G[RAIM_N_SV_MAX];
    bool C[RAIM_N_SV_MAX];
    bool N_sv;
    bool N_const;
    bool N_ss;
    bool ss_k_aiv;
    bool sig_tropo2;
    bool sig_user2;
    bool sig_ura2;
    bool sig_ure2;
    bool b_nom[RAIM_N_SV_MAX];
    bool w_sqrt[RAIM_N_SV_MAX];
    bool w_acc_sqrt[RAIM_N_SV_MAX];
    bool u;
    bool idx_ss[RAIM_N_SS_MAX];
    bool idx_faulty_sv;
    bool s[RAIM_N_SS_MAX]; // each matrix is independent
    bool spr[RAIM_N_SV_MAX]; // each column is independent
    bool y;
    bool k_fa;
    bool k_fa_r;
    bool sig_q2[RAIM_N_SS_MAX]; // each row is independent
    bool bias_q[RAIM_N_SS_MAX]; // each row is independent
    bool sig_ssq2[RAIM_N_SS_MAX]; // each row is independent
} rpu_regs_status_t;

/** Concrete functional unit. */
class raim_cop_fu : public sc_module {

    public:

        /** Constructor. */
        SC_HAS_PROCESS(raim_cop_fu);
        raim_cop_fu(sc_module_name name, rpu_regs_status_t *regs_status, rpu_regs_t *regs);

        /** Issue an instruction to the functional unit with a buffered immediate value. */
        bool issue_instr(uint32_t ir, int32_t rt);

        /** Read the CPSR resulting from the executed instruction. */
        uint8_t get_cpsr();

        /** Clear the CPSR. */
        void clear_cpsr();

    private:

        /** Status. */
        uint32_t _ir;
        int32_t _rt;
        uint8_t _cpsr;

        /** Pointers. */
        rpu_regs_status_t *_regs_status;
        rpu_regs_t *_regs;

        /** Main thread. */
        void main();

};

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
        void print_statistics();

    private:

        /** Intermediate values. */
        bool _has_next_pc_offset;

        /** Instruction queue. */
        uint32_t _instrs[RPU_INSTR_Q_SIZE];
        queue<uint32_t> _instr_q;
        int32_t _rt_vals[RPU_INSTR_Q_SIZE];
        queue<int32_t> _rt_val_q;

        /** Functional units. */
        raim_cop_fu *_fus[RPU_N_FUs];

        /** Registers. */
        rpu_regs_status_t _regs_status;
        rpu_regs_t _regs;
        uint8_t _rpu_cpsr;
        int32_t _next_pc_offset;

        /** Statistics. */
        uint32_t _cc_spinning;

        /** Main thread function. */
        void main();

};

#endif // RAIM_COP_H
