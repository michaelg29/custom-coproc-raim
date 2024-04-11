
#include "system.h"

#include "systemc.h"

#include "memory.h"
#include "isa.h"

#ifndef CPU_H
#define CPU_H

// ============================
// ===== HELPER FUNCTIONS =====
// ============================

int32_t sign_extend_immd(uint32_t ir, uint32_t shift);
int32_t zero_extend_immd(uint32_t ir, uint32_t shift);

// ===============================
// ===== REGISTER COLLECTION =====
// ===============================

/** Register collection. */
#define N_REGS 32
typedef struct {
    int32_t r0;
    int32_t at;
    int32_t v0;
    int32_t v1;
    int32_t a0;
    int32_t a1;
    int32_t a2;
    int32_t a3;
    int32_t t0;
    int32_t t1;
    int32_t t2;
    int32_t t3;
    int32_t t4;
    int32_t t5;
    int32_t t6;
    int32_t t7;
    int32_t s0;
    int32_t s1;
    int32_t s2;
    int32_t s3;
    int32_t s4;
    int32_t s5;
    int32_t s6;
    int32_t s7;
    int32_t t8;
    int32_t t9;
    int32_t k0;
    int32_t k1;
    int32_t gp;
    int32_t sp;
    int32_t s8;
    int32_t ra;
    uint32_t pc; // program counter
    uint32_t ir; // instruction register
    uint32_t ex; // exception register
    int32_t lo;
    int32_t hi;
} regs_t;

typedef union {
    regs_t s;
    int32_t w[N_REGS];
    uint32_t uw[N_REGS];
    int64_t l[N_REGS >> 1];
    uint64_t ul[N_REGS >> 1];
} regs_u;

// =============================
// ===== SYSTEM COMPONENTS =====
// =============================

/** Coprocessor to use inside the CPU. */
class coprocessor_if : virtual public sc_interface {

    public:

        /** Constructor. */
        coprocessor_if(uint32_t cop_opcode);

        /**
         * Execute an instruction on the coprocessor.
         *
         * @param ir  The 32-bit instruction.
         * @param rt  The received 32-bit general purpose register value.
         * @param res The output 32-bit value.
         * @returns Whether the coprocessor wrote a value to the result.
         */
        virtual bool execute(uint32_t ir, int32_t rt, int32_t &res) = 0;

        /** Get the value in a coprocessor register. */
        virtual bool get_regs(uint32_t rt, int32_t &res) = 0;

        /** Set the value in a coprocessor register. */
        virtual void set_regs(uint32_t rt, int32_t res) = 0;

        /** Get the condition code. */
        virtual bool get_condition_code(uint8_t &cc) = 0;

        /** Determine if the coprocessor signaled an exception in the previous instruction. */
        exception_e get_exception();

        /** Get the offset for the next program counter if the coprocessor has updated it. */
        virtual bool get_next_pc_offset(int32_t &next_pc_offset) = 0;

        /** Print locally-collected statistics. */
        virtual void print_statistics() = 0;

    protected:

        /** Protected variables. */
        uint32_t _cop_opcode;

        /** Raise an internal exception. */
        exception_e _prev_ex;
        void signal_ex(exception_e ex);

};

/** Stubbed co-processor with no internal functionality. */
class stubbed_cop : public coprocessor_if {

    public:

        /** Constructor. */
        stubbed_cop();

        /** coprocessor_if overrides. */
        bool execute(uint32_t ir, int32_t rt, int32_t &res);
        bool get_regs(uint32_t rt, int32_t &res);
        void set_regs(uint32_t rt, int32_t res);
        bool get_condition_code(uint8_t &cc);
        bool get_next_pc_offset(int32_t &next_pc_offset);
        void print_statistics();

};

/** Concrete CPU module. */
class cpu : public sc_module {

    public:

        /** Interfaces. */
        sc_port<memory_if> mem;
        sc_port<coprocessor_if> cop1;
        sc_port<coprocessor_if> cop2;
        sc_port<coprocessor_if> cop3;

        /** Constructor. */
        SC_HAS_PROCESS(cpu);
        cpu(sc_module_name name, uint32_t start_addr, uint32_t exit_addr, uint32_t max_instr_cnt);

        /** Destructor. */
        ~cpu();

    private:

        /** Private variables. */
        uint32_t _start_addr;
        uint32_t _exit_addr;
        uint32_t _max_instr_cnt;

        /** Registers. */
        regs_u _regs;

        /** Main function. */
        void main();

        void signal_ex(exception_e ex);

};

#endif // CPU_H
