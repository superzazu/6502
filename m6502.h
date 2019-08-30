#ifndef MOS_6502_M6502_H_
#define MOS_6502_M6502_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;

typedef struct m6502 {
    u16 pc; // program counter
    u8 a, x, y, sp; // register A, X, Y and stack pointer

    // flags: carry, zero, interrupt disable, decimal mode,
    // break command, overflow, negative
    bool cf, zf, idf, df, bf, vf, nf;

    bool page_crossed; // helper flag to keep track of page crossing
    bool enable_bcd; // helper flag to enable/disable BCD
    bool m65c02_mode; // helper flag to enable 65C02 emulation

    bool stop, wait; // flags used with STP/WAI 65C02 instructions

    unsigned long cyc; // cycles count

    void* userdata; // user custom pointer
    u8 (*read_byte)(void*, u16);
    void (*write_byte)(void*, u16, u8);
} m6502;

void m6502_init(m6502* const c);
void m6502_step(m6502* const c);
void m6502_debug_output(m6502* const c);

// interrupts
void m6502_gen_nmi(m6502* const c);
void m6502_gen_res(m6502* const c);
void m6502_gen_irq(m6502* const c);

#endif // MOS_6502_M6502_H_
