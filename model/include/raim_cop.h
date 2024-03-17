
#include "system.h"

#include "systemc.h"

#include "cpu.h"
#include "memory.h"
#include "isa.h"

#ifndef RAIM_COP_H
#define RAIM_COP_H

/** Register collection. */
typedef struct {

} rpu_regs_s;

/** Concrete RAIM coprocessor. */
class raim_cop : public sc_module, public coprocessor_if {

    public:

        /** Constructor. */
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

        /** Registers. */
        rpu_regs_s _regs;
        uint8_t _condition_code;
        int32_t _next_pc_offset;

};

#endif // RAIM_COP_H
