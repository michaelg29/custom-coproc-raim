
#include "systemc.h"

#include <string>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef SYSTEM_H
#define SYSTEM_H

// =============================
// ===== HIGH LEVEL MACROS =====
// =============================

// logging functions
#define LOG(a) std::cout << sc_time_stamp() << " - " << a << std::endl;
#define LOGF(a, ...) std::cout << sc_time_stamp() << " - "; printf(a, __VA_ARGS__); printf("\n")

// ===========================
// ===== DELAY FUNCTIONS =====
// ===========================

// clocking parameters
#define FREQ 1 // 1GHz = 1 op/ns
#define CC_NS (1 / FREQ)

// wait for next CC
#define POSEDGE_CPU() wait(1, SC_NS)
#define YIELD() wait(0, SC_NS)
// wait for next CC, then yield priority with another wait statement
#define POSEDGE() wait(1, SC_NS); wait(0, SC_NS)

// ============================
// ===== HELPER FUNCTIONS =====
// ============================

typedef struct {
    uint32_t addr;
    uint32_t size;
    uint32_t mem_cursor;
} mem_cursor_t;

bool parse_cmd_line(int argc, char **argv);
uint32_t read_input_files(mem_cursor_t *cursors, uint32_t max_n_cursor, uint32_t *out, uint32_t max_mem_size);

#endif // SYSTEM_H
