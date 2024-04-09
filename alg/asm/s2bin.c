
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "isa.h"

#define BUF_SIZE 1024
#define MAX_WORDS_PER_LOG_LINE 4

// ============================
// ===== Global variables =====
// ============================
FILE *fp;
int fsize = 0;

// ===============================
// ===== Function prototypes =====
// ===============================

/** Get the next byte in the file. */
char getfchar();

/** Parse a 32-bit word in hexadecimal format */
bool parsehex32(uint32_t *out, bool doRead0);

// ======================
// ===== Main entry =====
// ======================
int main(int argc, char **argv) {
    if (argc != 3) {
        printf("USAGE: %s <LOG_FILE> <OUT_FILE>\n", argv[0]);
        return 1;
    }

    /** Placeholder instruction replacements. */
    uint32_t bgezl_tst = (OPCODE_REGIMM << 26) | (0b00000 << 21) | (REGIMM_BGEZL << 16);
    uint32_t bgezl_mask = 0xffffffff << 16;
    printf("%08x test %08x mask\n", bgezl_tst, bgezl_mask);
    uint32_t gen_instr_idx = 0;
    int gen_instr[] = {
        gen_immd_instr(OPCODE_LWC2, REGS_t0, RPU_VR_AL0, -12), // LWC2 $AL0, -12($t0)
        gen_immd_instr(OPCODE_LWC2, REGS_t0, RPU_VR_SA2, -8),
        gen_immd_instr(OPCODE_LWC2, REGS_t0, RPU_VR_SE2, -4),
        gen_immd_instr(OPCODE_LWC2, REGS_t0, RPU_VR_KX, 4),
        gen_immd_instr(OPCODE_LWC2, REGS_t0, RPU_VR_KY, 8),
        gen_immd_instr(OPCODE_LWC2, REGS_t0, RPU_VR_KZ, 12),
        gen_immd_instr(OPCODE_LWC2, REGS_t0, RPU_VR_KR, 16),
        gen_immd_instr(OPCODE_LWC2, REGS_t1, RPU_VR_LX, 0),
        gen_immd_instr(OPCODE_LWC2, REGS_t1, RPU_VR_LY, 4),
        gen_immd_instr(OPCODE_LWC2, REGS_t1, RPU_VR_LZ, 8),
        gen_immd_instr(OPCODE_LWC2, REGS_t1, RPU_VR_C,  12),
        gen_immd_instr(OPCODE_LWC2, REGS_t1, RPU_VR_ST2, 16),
        gen_immd_instr(OPCODE_LWC2, REGS_t1, RPU_VR_SR2, 20),
        gen_immd_instr(OPCODE_LWC2, REGS_t0, RPU_VR_BN, 0),
        gen_reg_instr (OPCODE_COP2, RPU_FMT_NONE, 0, 0, 0, RPU_NEWSV),
        gen_reg_instr (OPCODE_COP2, RPU_FMT_NONE, 0, 0, 0, RPU_CALCU),
        gen_reg_instr (OPCODE_COP2, RPU_FMT_MT, REGS_t0, RPU_VR_IDX, 0, 0),
        gen_reg_instr (OPCODE_COP2, RPU_FMT_NONE, 0, 0, 0, RPU_INITP),
        gen_reg_instr (OPCODE_COP2, RPU_FMT_NONE, 0, 0, 0, RPU_CALCP),
        gen_reg_instr (OPCODE_COP2, RPU_FMT_NONE, 0, 0, 0, RPU_WLS),
        gen_reg_instr (OPCODE_COP2, RPU_FMT_NONE, 0, 0, 0, RPU_NEWSS),
        gen_reg_instr (OPCODE_COP2X, REGS_t0, REGS_t1, 0, RPU_VR_IDX, OPCODE_LWXCZ), // LWXC2 $IDX $t1($t0)
        gen_reg_instr (OPCODE_COP2, RPU_FMT_NONE, 0, 0, 0, RPU_INITP),
        gen_reg_instr (OPCODE_COP2, RPU_FMT_NONE, 0, 0, 0, RPU_CALCP),
        gen_reg_instr (OPCODE_COP2, RPU_FMT_NONE, 0, 0, 0, RPU_WLS),
        gen_reg_instr (OPCODE_COP2, RPU_FMT_NONE, 0, 0, 0, RPU_POSVAR),
        gen_reg_instr (OPCODE_COP2, RPU_FMT_NONE, 0, 0, 0, RPU_BIAS),
        gen_reg_instr (OPCODE_COP2, RPU_FMT_NONE, 0, 0, 0, RPU_CALCSS),
        gen_reg_instr (OPCODE_COP2, RPU_FMT_NONE, 0, 0, 0, RPU_SSVAR),
        gen_reg_instr (OPCODE_COP2, RPU_FMT_NONE, 0, 0, 0, RPU_TSTG),
        gen_immd_instr(OPCODE_COP2, RPU_FMT_BC, RPU_FD, 0), // BFDC2 sv_local_test
        gen_reg_instr (OPCODE_COP2, RPU_FMT_MT, REGS_t3, RPU_VR_I, 0, 0),
        gen_reg_instr (OPCODE_COP2, RPU_FMT_NONE, 0, 0, 0, RPU_TSTL),
        gen_immd_instr(OPCODE_COP2, RPU_FMT_BC, RPU_FL, 0), // BFLC2 faulty_sv_located
        gen_reg_instr (OPCODE_COP2, RPU_FMT_NONE, 0, 0, 0, RPU_NEWSS),
    };
    uint32_t n_gen_instr = sizeof(gen_instr) / sizeof(uint32_t);
    printf("Replacing %d instructions.\n", n_gen_instr);

    // read input
    fp = fopen(argv[1], "r");
    if (!fp) {
        printf("Could not open file %s for reading\n", argv[1]);
        return 1;
    }
    // get file size
    fseek(fp, 0L, SEEK_END);
    fsize = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    // write output
    FILE *fout = fopen(argv[2], "wb");
    if (!fout) {
        printf("Could not open file %s for writing\n", argv[2]);
        fclose(fp);
        return 1;
    }

    // read characters
    uint32_t base_addr;
    uint32_t size;
    uint32_t word;
    uint32_t words[MAX_WORDS_PER_LOG_LINE];
    char c = getfchar();
    bool newline = true;
    bool dataline = false;

    // process a line in each iteration of the loop
    uint32_t line = 0;
    while (c) {
        line++;
        do {
            if (c != '[') break;
            // line with bytes to store

            // get base address
            if (!parsehex32(&base_addr, true)) {
                printf("Could not parse base address on line %d\n", line);
                break;
            }
            if (getfchar() != ']') break;

            // empty memory
            if ((c = getfchar()) == '.') break;

            // read numbers
            for (size = 0; size < MAX_WORDS_PER_LOG_LINE; ++size) {
                while (c == ' ') c = getfchar();
                if (c != '0' || !parsehex32(&words[size], false)) {
                    break;
                }
                c = getfchar();

                // perform replacement
                if (!words[size] && gen_instr_idx < n_gen_instr) {
                    // total replacement of nop
                    words[size] = (uint32_t)gen_instr[gen_instr_idx];
                    gen_instr_idx++;
                }
                else if (((words[size] & bgezl_mask) == bgezl_tst) && gen_instr_idx < n_gen_instr) {
                    // preserve existing offset for branch
                    word = (uint32_t)gen_instr[gen_instr_idx];
                    word &= 0xffff0000;
                    word |= words[size] & 0x0000ffff;
                    words[size] = word;
                    gen_instr_idx++;
                }
            }

            if (size == 0) {
                printf("Could not parse data words on line %d\n", line);
            }
            else {
                printf("Read %d words starting at %08x on line %d:", size, base_addr, line);
                for (int i = 0; i < size; ++i) {
                    printf(" %08x", words[i]);
                }
                printf("\n");

                // write memory to output file
                fwrite(&base_addr, sizeof(base_addr), 1, fout);
                fwrite(&size, sizeof(size), 1, fout);
                fwrite(words, sizeof(words[0]), size, fout);
            }
        } while (false);

        // read until next line
        while (c && !(c == '\n' || (c == '\r' && getfchar() == '\n'))) {
            c = getfchar();
        }
        c = getfchar();
    }

    fclose(fp);
    fclose(fout);

    return 0;
}

/** Get a character from the file. */
int loaded = 0;
int bufcursor = 0;
int fcursor = 0;
char buf[BUF_SIZE+1];
char getfchar() {
    if (bufcursor >= loaded) {
        if (fcursor < fsize) {
            // determine how may bytes to load (max BUF_SIZE)
            loaded = fsize - fcursor;
            if (loaded > BUF_SIZE) loaded = BUF_SIZE;

            // load bytes
            if (!fread(buf, 1, loaded, fp)) {
                printf("Could not read more bytes\n");
                loaded = 0;
            }
        }
        else {
            loaded = 0;
        }

        // update cursors
        fcursor += loaded;
        bufcursor = 0;
        buf[loaded] = 0;
    }

    // return character
    char ret = buf[bufcursor];
    bufcursor++;
    return ret;
}

bool parsehex32(uint32_t *out, bool doRead0) {
    // 0x starter
    if (doRead0 && getfchar() != '0') return false;
    if (getfchar() != 'x') return false;

    // iterate through 8 characters
    uint32_t val = 0;
    for (int i = 0; i < 8; ++i) {
        val <<= 4;
        char c = getfchar();
        if (c >= '0' && c <= '9') {
            val |= (c - '0') & 0xf;
        }
        else if (c >= 'a' && c <= 'f') {
            val |= (c - 'a' + 10) & 0xf;
        }
        else if (c >= 'A' && c <= 'F') {
            val |= (c - 'A' + 10) & 0xf;
        }
        else {
            return false;
        }
    }

    *out = val;
    return true;
}
