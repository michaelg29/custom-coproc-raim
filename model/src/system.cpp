
#include "system.h"
#include "sc_fault_inject.hpp"
#include "sc_trace.hpp"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <string>

// data file names
char *asm_file_name = (char*)"../../alg/asm/raim.bin";

// optional arguments
char *trace_file_name = NULL;
uint32_t simulation_step_size = 10;
sc_time_unit simulation_time_unit = SC_NS;

bool parse_cmd_line(int argc, char **argv) {
    // check usage
    if (argc < 1 || argc > 5) {
    std::cerr << "Usage: " << argv[0] << " [<ASM_BIN_FILE>] [-t<TRACE_FILE>] [-s<FAULT_INJECT_STEP_SIZE> [(m|u|n|p)]]" << std::endl;
        return false;
    }

    int argi = 1;

    if (argc >= 5) {
        // latch file names
        asm_file_name = argv[argi++];
    }

    // enable tracing
    if (argi < argc && argv[argi][1] == 't') {
        sc_tracer::enable();
        trace_file_name = argv[argi++] + 2; // advance past "-t"
    }
    else {
        sc_tracer::disable();
    }

    // read step size
    if (argi < argc && argv[argi][1] == 's') {
        simulation_step_size = std::stoi((const char*)(argv[argi++] + 2)); // advance past "-s"

        // get time unit if not another option
        if (argi < argc && argv[argi][0] != '-') {
            char arg = argv[argi++][0];

            switch (arg) {
            case 'm': simulation_time_unit = SC_MS; break;
            case 'u': simulation_time_unit = SC_US; break;
            case 'n': simulation_time_unit = SC_NS; break;
            case 'p': simulation_time_unit = SC_PS; break;
            };
        }
    }

    // enable fault injector
    if (simulation_step_size > 0) {
        sc_fault_injector::enable();
        sc_fault_injector::init((double)simulation_step_size, simulation_time_unit);
    }
    else {
        sc_fault_injector::disable();
    }

    // initialize tracer with filename and time unit
    if (trace_file_name) {
        sc_tracer::init((const char*)trace_file_name, simulation_time_unit);
    }

    return true;
}

uint32_t read_input_files(mem_cursor_t *cursors, uint32_t max_n_cursor, uint32_t *out, uint32_t max_mem_size) {
    // open ASM file
    FILE *fp = fopen(asm_file_name, "rb");
    if (!fp) return 0;
    
    uint32_t out_cursor = 0;
    
    for (int i = 0; i < max_n_cursor; i++) {
        // read eight bytes for address and size
        if (!fread((void*)(cursors + i), sizeof(uint32_t), 2, fp)) break;
        cursors[i].mem_cursor = out_cursor;
        uint32_t size = cursors[i].size;
        if (!fread((void*)(out + out_cursor), sizeof(uint32_t), size, fp)) break;
        
        printf("Read %d words for addr %08x:", size, cursors[i].addr);
        for (int i = 0; i < size; ++i) {
            printf(" %08x", out[out_cursor + i]);
        }
        printf("\n");
        
        out_cursor += size;
    }
    
    fclose(fp);

    return out_cursor;
}
