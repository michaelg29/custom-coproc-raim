
#include "cpu.h"

#include "system.h"

#include "systemc.h"

cpu::cpu(sc_module_name name, uint32_t start_addr, uint32_t exit_addr, uint32_t max_instr_cnt) : sc_module(name), _start_addr(start_addr), _exit_addr(exit_addr), _max_instr_cnt(max_instr_cnt) {
    SC_THREAD(main);

    // initial register values
    _regs.pc = _start_addr;
}

cpu::~cpu() {}

void cpu::main() {
    uint32_t instr_cnt = 0;

    while (!_max_instr_cnt || instr_cnt < _max_instr_cnt) {
        // =======================
        // == INSTRUCTION FETCH ==
        // =======================
        mem->read(_regs.pc, _regs.ir);
        LOGF("[%s]: Read instr %08x from addr %08x", this->name(), _regs.ir, _regs.pc);

        // ========================
        // == INSTRUCTION DECODE ==
        // ========================

        // =============
        // == EXECUTE ==
        // =============

        // ============
        // == MEMORY ==
        // ============

        // ================
        // == WRITE-BACK ==
        // ================
        if (_regs.pc == _exit_addr) {
            break;
        }
        _regs.pc = _regs.pc + 4;

        instr_cnt++;
        POSEDGE_CPU();
    }
}
