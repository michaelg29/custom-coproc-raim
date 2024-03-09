
#include "system.h"

#include "systemc.h"

#include "cpu.h"
#include "memory.h"
#include "isa.h"

#ifndef FP_COP_H
#define FP_COP_H

/** Concrete floating point coprocessor. */
class fp_cop : public sc_module, public coprocessor_if {

    public:

        /** Constructor. */
        fp_cop(sc_module_name name);

        /** Destructor. */
        ~fp_cop();

        /** coprocessor_if overrides. */
        bool execute(uint32_t ir, int32_t rt, int32_t &res);
        bool get_regs(uint32_t rt, int32_t &res);
        void set_regs(uint32_t rt, int32_t res);
        bool get_next_pc_offset(int32_t &next_pc_offset);
        exception_e get_exception();

    private:

        /** Intermediate values. */
        int32_t _fs, _ft, _fd, _fr;
        int32_t _fpu_fmt;
        int32_t _branch_flags;
        bool _has_next_pc_offset;

        /** Registers. */
        fp_regs_u _regs;
        uint8_t _condition_code;
        int32_t _next_pc_offset;

};

#endif // FP_COP_H
