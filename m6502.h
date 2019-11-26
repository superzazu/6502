#ifndef M6502_M6502_H_
#define M6502_M6502_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct m6502 {
    uint8_t (*read_byte)(void*, uint16_t); // user function to read from memory
    void (*write_byte)(void*, uint16_t, uint8_t); // same for writing to memory
    void* userdata; // user custom pointer

    unsigned long cyc; // cycle count

    uint16_t pc; // program counter
    uint8_t a, x, y, sp; // register A, X, Y and stack pointer

    // flags: carry, zero, interrupt disable, decimal mode,
    // break command, overflow, negative
    bool cf : 1, zf : 1, idf : 1, df : 1, bf : 1, vf : 1, nf : 1;

    bool page_crossed : 1; // helper flag to keep track of page crossing
    bool enable_bcd : 1; // helper flag to enable/disable BCD
    bool m65c02_mode : 1; // helper flag to enable 65C02 emulation

    bool stop : 1, wait : 1; // flags used with STP/WAI 65C02 instructions
} m6502;

void m6502_init(m6502* const c);
void m6502_step(m6502* const c);
void m6502_debug_output(m6502* const c);

// interrupts
void m6502_gen_nmi(m6502* const c);
void m6502_gen_res(m6502* const c);
void m6502_gen_irq(m6502* const c);

#endif // M6502_M6502_H_
