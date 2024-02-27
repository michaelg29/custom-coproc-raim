
#include "memory.h"

#include "system.h"

#include "systemc.h"

/** Read formatted binary file with data and text sections. */
uint32_t read_input_file(mem_cursor_t *cursors, uint32_t max_n_cursor, uint32_t *out, uint32_t max_mem_size) {
    // open ASM file
    FILE *fp = fopen(get_asm_file_name(), "rb");
    if (!fp) return 0;

    uint32_t out_cursor = 0;

    int i;
    for (i = 0; i < max_n_cursor; i++) {
        // read eight bytes for address and size
        if (!fread((void*)(cursors + i), sizeof(uint32_t), 2, fp)) break;
        uint32_t size = cursors[i].size;

        // determine if contiguous with previous block
        if (i > 0 && (cursors[i-1].addr + (cursors[i-1].size << 2) == cursors[i].addr)) {
            // add to previous block
            i--;
            cursors[i].size += size;
        }
        else {
            // set cursor in newly allocated entry
            cursors[i].mem_cursor = out_cursor;
        }

        if (!fread((void*)(out + out_cursor), sizeof(uint32_t), size, fp)) break;

        out_cursor += size;
    }

    fclose(fp);

    return i;
}

memory::memory(sc_module_name name, uint32_t max_cursors, uint32_t max_mem_words): sc_module(name) {
    if (max_cursors > 0 && max_mem_words > 0) {
        _max_cursors = max_cursors;
        _cursors = new mem_cursor_t[max_cursors];
        _max_mem_words = max_mem_words;
        _mem = new uint32_t[max_mem_words];

        _n_cursors = read_input_file(_cursors, max_cursors, _mem, max_mem_words);
        _mem_words = 0;
        for (int i = 0; i < _n_cursors; ++i) {
            _mem_words += _cursors[i].size;
        }

        LOGF("[%s]: %d cursors and %d words", this->name(), _n_cursors, _mem_words);
    }
    else {
        _n_cursors = 0;
        _max_cursors = 0;
        _cursors = nullptr;
        _mem_words = 0;
        _max_mem_words = 0;
        _mem = nullptr;
    }
}

memory::~memory() {
    if (_max_cursors) delete _cursors;
    if (_max_mem_words) delete _mem;
}

bool memory::read(uint32_t addr, uint32_t& data) {
    // find memory entry containing the address
    // use data as the counter to save memory
    for (data = 0; data < _n_cursors; ++data) {
        if (_cursors[data].addr <= addr &&
            _cursors[data].addr + (_cursors[data].size << 2) > addr) {
            break;
        }
    }

    if (data == _n_cursors) {
        // iterated through list without finding the address
        data = 0;
    }
    else {
        // calculate the address in the memory array
        addr = _cursors[data].mem_cursor + ((addr - _cursors[data].addr) >> 2);
        data = _mem[addr];
    }

    return true;
}

bool memory::write(uint32_t addr, uint32_t data) {
    // find memory entry containing the address
    int i;
    for (i = 0; i < _n_cursors; ++i) {
        if (_cursors[i].addr <= addr &&
            _cursors[i].addr + (_cursors[i].size << 2) > addr) {
            break;
        }
    }

    if (i == _n_cursors) {
        // iterated through list without finding the address

        if (_n_cursors < _max_cursors) {
            // find spot for new cursor in order
            for (i = 0; i < _n_cursors; ++i) {
                if (_cursors[i].addr > addr) {
                    break;
                }
            }

            // shift list elements
            for (int j = _n_cursors; j > i; --j) {
                _cursors[j] = _cursors[j-1];
            }

            // insert into cursor list
            _cursors[i].addr = addr;
            _cursors[i].size = 1;
            _cursors[i].mem_cursor = _mem_words;

            // insert into memory
            _mem[_mem_words] = data;
            _mem_words++;

            return true;
        }
        else {
            return false;
        }
    }

    // calculate the address in the memory array
    addr = _cursors[i].mem_cursor + ((addr - _cursors[i].addr) >> 2);
    _mem[addr] = data;

    return true;
}
