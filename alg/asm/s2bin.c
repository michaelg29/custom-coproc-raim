
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define BUF_SIZE 1024
#define MAX_WORDS_PER_LOG_LINE 4

FILE *fp;
int fsize = 0;

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

/** Parse a 32-bit word in hexadecimal format */
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
        else {
            return false;
        }
    }
    
    *out = val;
    return true;
}

/** Main entry */
int main(int argc, char **argv) {
    if (argc != 3) {
        printf("USAGE: %s <LOG_FILE> <OUT_FILE>\n", argv[0]);
        return 1;
    }
    memset(buf, 0, BUF_SIZE);

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
