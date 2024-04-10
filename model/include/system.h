
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
#define DELAY_CC(n) wait(n, SC_NS)
#define YIELD() wait(0, SC_NS)
// wait for next CC, then yield priority with another wait statement
#define POSEDGE() wait(1, SC_NS); wait(0, SC_NS)

// ============================
// ===== HELPER FUNCTIONS =====
// ============================

bool parse_cmd_line(int argc, char **argv);
char *get_asm_file_name();

#endif // SYSTEM_H
