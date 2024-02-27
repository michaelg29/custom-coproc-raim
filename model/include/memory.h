
#include "system.h"

#include "systemc.h"

#ifndef MEMORY_H
#define MEMORY_H

typedef struct {
    uint32_t addr;
    uint32_t size;
    uint32_t mem_cursor;
} mem_cursor_t;

/** Interface with memory. */
class memory_if : virtual public sc_interface {

    public:

        /** Read from memory address. */
        virtual bool read(uint32_t addr, uint32_t& data) = 0;

        /** Write to memory address. */
        virtual bool write(uint32_t addr, uint32_t data) = 0;

};

/** Concrete memory module. */
class memory : public sc_module, memory_if {

    public:

        /** Constructor. */
        memory(sc_module_name name, uint32_t max_cursors, uint32_t max_mem_words);

        /** Destructor. */
        ~memory();

        /** Read data. */
        bool read(uint32_t addr, uint32_t& data);

        /** Write data. */
        bool write(uint32_t addr, uint32_t data);

    private:

        uint32_t _n_cursors;
        uint32_t _max_cursors;
        mem_cursor_t *_cursors;
        uint32_t _mem_words;
        uint32_t _max_mem_words;
        uint32_t *_mem;

};

#endif // MEMORY_H
