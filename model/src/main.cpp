
#include "system.h"
#include "sc_fault_inject.hpp"
#include "sc_trace.hpp"

#include "systemc.h"
#include <iostream>
#include <string>

// simulation components
#include "cpu.h"
#include "isa.h"
#include "fp_cop.h"
#include "memory.h"

// Statistics collecting classes
sc_fault_injector sc_fault_injector::injector;
sc_tracer sc_tracer::tracer;

int sc_main(int argc, char* argv[]) {
    if (!parse_cmd_line(argc, argv)) {
        return 1;
    }

    // initial state
    std::cout << "Initial state:" << std::endl;

    // ======================================
    // ===== CREATE AND CONNECT MODULES =====
    // ======================================

    // memory has max 64 cursors and 1<<13 words = 1<<15 bytes
    memory *mem = new memory("mem", 64, 1<<13);

    // cpu and coprocessors
    coprocessor_if *empty_cop = new stubbed_cop();
    coprocessor_if *cop1 = new fp_cop("fp_cop", OPCODE_COP1);
    cpu *c = new cpu("cpu", 0x00400000, 0, 0);
    c->mem(*mem);
    c->cop1(*cop1);
    c->cop2(*empty_cop);
    c->cop3(*empty_cop);

    // ==============================
    // ===== RUN THE SIMULATION =====
    // ==============================

    std::cout << "Starting simulation..." << std::endl;
    sc_time duration = sc_fault_injector::simulate();

    // final state
    std::cout << "Final state:" << std::endl;
    mem->print();

    // ===================
    // ===== CLEANUP =====
    // ===================

    delete mem;

    std::cout << "Simulated for " << duration << std::endl;

    sc_tracer::close();

    return 0;
}