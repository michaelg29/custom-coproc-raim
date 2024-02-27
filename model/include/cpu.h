
#include "system.h"

#include "systemc.h"

#include "memory.h"

#ifndef CPU_H
#define CPU_H

/** Register collection. */
typedef struct {
    uint32_t pc;
    uint32_t ir;
} regs_t;

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
        regs_t _regs;

        /** Main function. */
        void main();

};

#endif // CPU_H
