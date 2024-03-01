
#include "system.h"

#include "systemc.h"

#include "memory.h"

#ifndef CPU_H
#define CPU_H

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
    uint32_t pc;
    uint32_t ir;
    int32_t lo;
    int32_t hi;
} regs_t;

typedef union {
    regs_t s;
    int32_t a[N_REGS];
    uint32_t ua[N_REGS];
} regs_u;

/** Concrete CPU module. */
class cpu : public sc_module {

    public:

        /** Interfaces. */
        sc_port<memory_if> mem;

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

};

#endif // CPU_H
