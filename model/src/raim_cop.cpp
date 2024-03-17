
#include "raim_cop.h"

#include "system.h"
#include "isa.h"

#include "systemc.h"

raim_cop::raim_cop(sc_module_name name, uint32_t cop_opcode)
    : sc_module(name), coprocessor_if(cop_opcode) {

}

raim_cop::~raim_cop() {

}

bool raim_cop::execute(uint32_t ir, int32_t rt, int32_t &res) {
    // default values
    _prev_ex = EX_NONE;
    _has_next_pc_offset = false;

    // decode operation
    switch (GET_INSTR_COP_OP(ir)) {
    default: {
        LOGF("Invalid RPU opcode: %02x", GET_INSTR_COP_OP(ir));
        signal_ex(EX_INVALID);
        break;
    }
    }

    return false;
}

bool raim_cop::get_regs(uint32_t rt, int32_t &res) {
    return false;
}

void raim_cop::set_regs(uint32_t rt, int32_t res) {

}

bool raim_cop::get_condition_code(uint8_t &cc) {
    cc = _condition_code;
    return true;
}

bool raim_cop::get_next_pc_offset(int32_t &next_pc_offset) {
    next_pc_offset = _next_pc_offset;
    return _has_next_pc_offset;
}
