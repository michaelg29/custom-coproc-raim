
#include "system.h"
#include "sc_fault_inject.hpp"
#include "sc_trace.hpp"

#include "systemc.h"
#include <iostream>
#include <string>

// simulation components
#include "cpu.h"
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
    
    cpu *c = new cpu("cpu", 0x00400000, 0x00400018, 4);
    c->mem(*mem);

    // ==============================
    // ===== RUN THE SIMULATION =====
    // ==============================

    std::cout << "Starting simulation..." << std::endl;
    sc_time duration = sc_fault_injector::simulate();

    // ===================
    // ===== CLEANUP =====
    // ===================

    delete mem;

    std::cout << "Simulated for " << duration << std::endl;

    sc_tracer::close();

    return 0;
}