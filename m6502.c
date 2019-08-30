#include "m6502.h"

// the number of cycles an instruction takes
static const u8 CYCLES_6502[] = {
    0, 6, 0, 0, 0, 3, 5, 0, 3, 2, 2, 0, 0, 4, 6, 0,
    2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
    6, 6, 0, 0, 3, 3, 5, 0, 4, 2, 2, 0, 4, 4, 6, 0,
    2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
    6, 6, 0, 0, 0, 3, 5, 0, 3, 2, 2, 0, 3, 4, 6, 0,
    2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
    6, 6, 0, 0, 0, 3, 5, 0, 4, 2, 2, 0, 5, 4, 6, 0,
    2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
    0, 6, 0, 0, 3, 3, 3, 0, 2, 0, 2, 0, 4, 4, 4, 0,
    2, 6, 0, 0, 4, 4, 4, 0, 2, 5, 2, 0, 0, 5, 0, 0,
    2, 6, 2, 0, 3, 3, 3, 0, 2, 2, 2, 0, 4, 4, 4, 0,
    2, 5, 0, 0, 4, 4, 4, 0, 2, 4, 2, 0, 4, 4, 4, 0,
    2, 6, 0, 0, 3, 3, 5, 0, 2, 2, 2, 0, 4, 4, 6, 0,
    2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
    2, 6, 0, 0, 3, 3, 5, 0, 2, 2, 2, 0, 4, 4, 6, 0,
    2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0
};

// from http://www.obelisk.demon.co.uk/65C02/reference.html
static const u8 CYCLES_65C02[] = {
    0, 6, 0, 0, 5, 3, 5, 0, 3, 2, 2, 0, 6, 4, 6, 0,
    2, 5, 5, 0, 5, 4, 6, 0, 2, 4, 2, 0, 6, 4, 7, 0,
    6, 6, 0, 0, 3, 3, 5, 0, 4, 2, 2, 0, 4, 4, 6, 0,
    2, 5, 5, 0, 3, 4, 6, 0, 2, 4, 2, 0, 4, 4, 7, 0,
    6, 6, 0, 0, 0, 3, 5, 0, 3, 2, 2, 0, 3, 4, 6, 0,
    2, 5, 5, 0, 0, 4, 6, 0, 2, 4, 3, 0, 0, 4, 7, 0,
    6, 6, 0, 0, 3, 3, 5, 0, 4, 2, 2, 0, 5, 4, 6, 0,
    2, 5, 5, 0, 4, 4, 6, 0, 2, 4, 4, 0, 6, 4, 7, 0,
    3, 6, 0, 0, 3, 3, 3, 0, 2, 3, 2, 0, 4, 4, 4, 0,
    2, 6, 5, 0, 4, 4, 4, 0, 2, 5, 2, 0, 4, 5, 5, 0,
    2, 6, 2, 0, 3, 3, 3, 0, 2, 2, 2, 0, 4, 4, 4, 0,
    2, 5, 5, 0, 4, 4, 4, 0, 2, 4, 2, 0, 4, 4, 4, 0,
    2, 6, 0, 0, 3, 3, 5, 0, 2, 2, 2, 3, 4, 4, 6, 0,
    2, 5, 5, 0, 0, 4, 6, 0, 2, 4, 3, 3, 0, 4, 7, 0,
    2, 6, 0, 0, 3, 3, 5, 0, 2, 2, 2, 0, 4, 4, 6, 0,
    2, 5, 5, 0, 0, 4, 6, 0, 2, 4, 4, 0, 0, 4, 7, 0
};

// the number of additional cycles an instruction takes if a page is crossed
static const u8 INSTRUCTIONS_PAGE_CROSSED_CYCLES[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1
};

static const u16 STACK_START_ADDR = 0x100;


// memory helpers (the only functions to use read_byte and write_byte
// function pointers)

// reads a byte from memory
static inline u8 m6502_rb(m6502* const c, u16 addr) {
    return c->read_byte(c->userdata, addr);
}

// reads a word from memory
static inline u16 m6502_rw(m6502* const c, u16 addr) {
    return (c->read_byte(c->userdata, addr + 1) << 8) |
            c->read_byte(c->userdata, addr);
}

// emulates a 6502 bug where the low byte wrapped without incrementing
// the high byte
static inline u16 m6502_rw_bug(m6502* const c, u16 addr) {
    // the buggy read word has been fixed in the 65C02
    if (c->m65c02_mode) {
        return m6502_rw(c, addr);
    }

    u16 hi_addr = (addr & 0xFF00) | ((addr + 1) & 0xFF);
    return (c->read_byte(c->userdata, hi_addr) << 8) |
            c->read_byte(c->userdata, addr);
}

// writes a byte to memory
static inline void m6502_wb(m6502* const c, u16 addr, u8 val) {
    c->write_byte(c->userdata, addr, val);
}

// addressing modes helpers
static inline u16 IMM(m6502* const c) { // immediate
    return c->pc++;
}

static inline u8 ZPG(m6502* const c) { // zero page
    return m6502_rb(c, c->pc++);
}

static inline u8 ZPX(m6502* const c) { // zero page + x
    return m6502_rb(c, c->pc++) + c->x;
}

static inline u8 ZPY(m6502* const c) { // zero page + y
    return m6502_rb(c, c->pc++) + c->y;
}

static inline u16 ABS(m6502* const c) { // absolute
    u16 addr = m6502_rw(c, c->pc);
    c->pc += 2;
    return addr;
}

static inline u16 ABX(m6502* const c) { // absolute + x
    u16 addr = ABS(c) + c->x;
    c->page_crossed = ((addr - c->x) & 0xFF00) != (addr & 0xFF00);
    return addr;
}

static inline u16 ABY(m6502* const c) { // absolute + y
    u16 addr = ABS(c) + c->y;
    c->page_crossed = ((addr - c->y) & 0xFF00) != (addr & 0xFF00);
    return addr;
}

static inline u16 INX(m6502* const c) { // indexed indirect x
    return m6502_rw_bug(c, (m6502_rb(c, c->pc++) + c->x) & 0xFF);
}

static inline u16 INY(m6502* const c) { // indirect indexed y
    u16 addr = m6502_rw_bug(c, m6502_rb(c, c->pc++)) + c->y;
    c->page_crossed = ((addr - c->y) & 0xFF00) != (addr & 0xFF00);
    return addr;
}

static inline s8 REL(m6502* const c) { // relative
    return (s8) m6502_rb(c, c->pc++);
}

static inline u16 INZ(m6502* const c) { // indirect zero page (65C02)
    u16 addr = m6502_rw(c, m6502_rb(c, c->pc++));
    return addr;
}

// stack

// pushes a byte onto the stack
static inline void push_byte(m6502* const c, u8 val) {
    u16 addr = STACK_START_ADDR + c->sp--;
    m6502_wb(c, addr, val);
}

// pushes a word onto the stack
static inline void push_word(m6502* const c, u16 val) {
    u16 addr = STACK_START_ADDR + c->sp;
    m6502_wb(c, addr, val >> 8);
    m6502_wb(c, addr - 1, val & 0xFF);
    c->sp -= 2;
}

// pulls a byte from the stack
static inline u8 pull_byte(m6502* const c) {
    u16 addr = STACK_START_ADDR + ++c->sp;
    return m6502_rb(c, addr);
}

// pulls a word from the stack
static inline u16 pull_word(m6502* const c) {
    return pull_byte(c) | (pull_byte(c) << 8);
}

// flag helpers

// helper to quickly set Z/N flags according to a byte value
static inline void set_zn(m6502* const c, u8 val) {
    c->zf = val == 0;
    c->nf = val >> 7;
}

// returns flags status in one byte
static inline u8 get_flags(m6502* const c) {
    u8 flags = 0;
    flags |= c->nf << 7;
    flags |= c->vf << 6;
    flags |= 1 << 5; // bit 5 is always set
    flags |= c->bf << 4; // clear if interrupt vectoring, set if BRK or PHP
    flags |= c->df << 3;
    flags |= c->idf << 2;
    flags |= c->zf << 1;
    flags |= c->cf << 0;
    return flags;
}

static inline void set_flags(m6502* const c, u8 val) {
    c->nf = (val >> 7) & 1;
    c->vf = (val >> 6) & 1;
    c->df = (val >> 3) & 1;
    c->bf = (val >> 4) & 1;
    c->idf = (val >> 2) & 1;
    c->zf = (val >> 1) & 1;
    c->cf = (val >> 0) & 1;
}

// interrupts

static inline void interrupt(m6502* const c, u16 vector) {
    push_word(c, c->pc);
    push_byte(c, get_flags(c));
    c->pc = m6502_rw(c, vector);

    c->idf = 1;
    c->wait = 0;
    c->cyc += 7;
    if (c->m65c02_mode) {
        c->df = 0;
    }
}

// opcodes - storage

// loads a register with a byte
static inline void m6502_ldr(m6502* const c, u8* const reg, u16 addr) {
    *reg = m6502_rb(c, addr);
    set_zn(c, *reg);
}

// opcodes - math

// adds a byte (+ carry flag) to the accumulator
static inline void m6502_adc(m6502* const c, u16 addr) {
    const u8 val = m6502_rb(c, addr);

    if (c->enable_bcd && c->df) {
        // decimal ADC
        const u8 cy = c->cf;

        c->nf = 0;
        c->vf = 0;
        c->zf = 0;
        c->cf = 0;

        u8 al = (c->a & 0xF) + (val & 0xF) + cy;
        if (al > 9) {
            al += 0x06;
        }

        u8 ah = (c->a >> 4) + (val >> 4) + (al > 0xF);

        c->vf = ~(c->a ^ val) & (c->a ^ (ah << 4)) & 0x80;

        if (ah > 9) {
            ah += 0x06;
        }
        if (ah > 0xF) {
            c->cf = 1;
        }

        c->a = (ah << 4) | (al & 0xF);

        if (c->a == 0) {
            c->zf = 1;
        }

        if (ah & 0x8) {
            c->nf = 1;
        }

        // in the 65c02, if the decimal mode is set in ADC/SBC,
        // those operations last one more cycle
        if (c->m65c02_mode) {
            c->cyc += 1;
        }
    }
    else {
        // binary ADC
        const u16 result = c->a + val + c->cf;

        set_zn(c, result & 0xFF);
        c->vf = (~(c->a ^ val) & (c->a ^ result) & 0x80);
        c->cf = result & 0xFF00;

        c->a = result & 0xFF;
    }
}

// substracts a byte (+ *not* carry flag) to the accumulator
static inline void m6502_sbc(m6502* const c, u16 addr) {
    const u8 val = m6502_rb(c, addr);

    if (c->enable_bcd && c->df) {
        // decimal ADC
        const u8 cy = !c->cf;

        c->nf = 0;
        c->vf = 0;
        c->zf = 0;
        c->cf = 0;

        const u16 result = c->a - val - cy;

        u8 al = (c->a & 0xF) - (val & 0xF) - cy;
        if (al >> 7) {
            al -= 0x06;
        }

        u8 ah = (c->a >> 4) - (val >> 4) - (al >> 7);

        if ((c->a ^ val) & (c->a ^ result) & 0x80) {
            c->vf = 1;
        }
        c->cf = !(result & 0xFF00);
        if (ah >> 7) {
            ah -= 6;
        }

        c->a = (ah << 4) | (al & 0xF);

        if (c->a == 0) {
            c->zf = 1;
        }

        if (ah & 0x8) {
            c->nf = 1;
        }

        // in the 65c02, if the decimal mode is set in ADC/SBC,
        // those operations last one more cycle
        if (c->m65c02_mode) {
            c->cyc += 1;
        }
    }
    else {
        // binary ADC
        const u16 result = c->a - val - !c->cf;

        set_zn(c, result & 0xFF);
        c->vf = (c->a ^ val) & (c->a ^ result) & 0x80;
        c->cf = !(result & 0xFF00);

        c->a = result & 0xFF;
    }
}

// increments a byte and returns the incremented value
static inline u8 m6502_inc(m6502* const c, u8 val) {
    u8 result = val + 1;
    set_zn(c, result);
    return result;
}

// increments a byte in memory
static inline void m6502_inc_addr(m6502* const c, u16 addr) {
    u8 val = m6502_rb(c, addr);
    m6502_wb(c, addr, m6502_inc(c, val));
}

// increments a register
static inline void m6502_inr(m6502* const c, u8* const reg) {
    *reg = m6502_inc(c, *reg);
}

// decrements a byte and returns the decremented value
static inline u8 m6502_dec(m6502* const c, u8 val) {
    u8 result = val - 1;
    set_zn(c, result);
    return result;
}

// decrements a byte in memory
static inline void m6502_dec_addr(m6502* const c, u16 addr) {
    u8 val = m6502_rb(c, addr);
    m6502_wb(c, addr, m6502_dec(c, val));
}

// decrements a register
static inline void m6502_der(m6502* const c, u8* const reg) {
    *reg = m6502_dec(c, *reg);
}

// opcodes - bitwise

// executes a logical AND between the accumulator and a byte in memory
static inline void m6502_and(m6502* const c, u16 addr) {
    u8 val = m6502_rb(c, addr);
    c->a &= val;
    set_zn(c, c->a);
}

// shifts left the contents of a byte and returns it
static inline u8 m6502_asl(m6502* const c, u8 val) {
    u8 result = val << 1;
    c->cf = val >> 7;
    set_zn(c, result);
    return result;
}

// shifts left the contents of a byte in memory
static inline void m6502_asl_addr(m6502* const c, u16 addr) {
    u8 val = m6502_rb(c, addr);
    m6502_wb(c, addr, m6502_asl(c, val));
}

// sets the Z flag as though the value at addr were ANDed with register A
static inline void m6502_bit(m6502* const c, u16 addr) {
    u8 val = m6502_rb(c, addr);
    c->vf = (val >> 6) & 1;
    c->zf = (val & c->a) == 0;
    c->nf = val >> 7;
}

// executes an exclusive OR on register A and a byte in memory
static inline void m6502_eor(m6502* const c, u16 addr) {
    c->a ^= m6502_rb(c, addr);
    set_zn(c, c->a);
}

// shifts right the contents of a byte and returns it
static inline u8 m6502_lsr(m6502* const c, u8 val) {
    u8 result = val >> 1;
    c->cf = val & 1;
    set_zn(c, result);
    return result;
}

// shifts right the contents of a byte in memory
static inline void m6502_lsr_addr(m6502* const c, u16 addr) {
    u8 val = m6502_rb(c, addr);
    m6502_wb(c, addr, m6502_lsr(c, val));
}

// executes an inclusive OR on register A and a byte in memory
static inline void m6502_ora(m6502* const c, u16 addr) {
    c->a |= m6502_rb(c, addr);
    set_zn(c, c->a);
}

// rotates left a byte and returns the rotated value
static inline u8 m6502_rol(m6502* const c, u8 val) {
    u8 result = val << 1;
    result |= c->cf;
    c->cf = val >> 7;
    set_zn(c, result);
    return result;
}

// rotates left a byte in memory
static inline void m6502_rol_addr(m6502* const c, u16 addr) {
    u8 val = m6502_rb(c, addr);
    m6502_wb(c, addr, m6502_rol(c, val));
}

// rotates right a byte and returns the rotated value
static inline u8 m6502_ror(m6502* const c, u8 val) {
    u8 result = val >> 1;
    result |= c->cf << 7;
    c->cf = val & 1;
    set_zn(c, result);
    return result;
}

// rotates right a byte in memory
static inline void m6502_ror_addr(m6502* const c, u16 addr) {
    u8 val = m6502_rb(c, addr);
    m6502_wb(c, addr, m6502_ror(c, val));
}

// test and resets bits
static inline void m6502_trb(m6502* const c, u16 addr) {
    u8 val = m6502_rb(c, addr);
    c->zf = (val & c->a) == 0;
    m6502_wb(c, addr, val & ~c->a);
}

// test and set bits
static inline void m6502_tsb(m6502* const c, u16 addr) {
    u8 val = m6502_rb(c, addr);
    c->zf = (val & c->a) == 0;
    m6502_wb(c, addr, val | c->a);
}

// opcodes - branch

// adds to PC a *signed* byte if condition is true.
static inline void m6502_branch(m6502* const c, s8 addr, bool condition) {
    if (condition) {
        if ((c->pc & 0xFF00) != ((c->pc + addr) & 0xFF00)) {
            c->page_crossed = 1;
        }
        c->pc += addr;
        c->cyc += 1; // add one cycle for taking a branch
    }
}

// opcodes - jump

// jumps to an address
static inline void m6502_jmp(m6502* const c, const u16 addr) {
    c->pc = addr;
}

// jumps to a subroutine
static inline void m6502_jsr(m6502* const c, u16 addr) {
    push_word(c, c->pc - 1);
    c->pc = addr;
}

// returns from an interrupt
static inline void m6502_rti(m6502* const c) {
    set_flags(c, pull_byte(c));
    c->pc = pull_word(c);
}

// returns from a subroutine
static inline void m6502_rts(m6502* const c) {
    c->pc = pull_word(c) + 1;
}

// opcodes - registers

// compares the value of a register with another byte in memory
static inline void m6502_cmp(m6502* const c, u16 addr, u8 reg_value) {
    u8 val = m6502_rb(c, addr);
    u8 result = reg_value - val;
    c->cf = reg_value >= val;
    set_zn(c, result);
}

static inline void execute_m65c02_opcode(m6502* const c, u8 opcode) {
    switch (opcode) {
    case 0x80: m6502_branch(c, REL(c), 1); break; // BRA REL

    case 0xDA: push_byte(c, c->x); break; // PHX
    case 0xFA: c->x = pull_byte(c); set_zn(c, c->x); break; // PLX
    case 0x5A: push_byte(c, c->y); break; // PHY
    case 0x7A: c->y = pull_byte(c); set_zn(c, c->y); break; // PLY

    case 0x9C: m6502_wb(c, ABS(c), 0); break; // STZ ABS
    case 0x9E: m6502_wb(c, ABX(c), 0); break; // STZ ABX
    case 0x64: m6502_wb(c, ZPG(c), 0); break; // STZ ZPG
    case 0x74: m6502_wb(c, ZPX(c), 0); break; // STZ ZPX

    case 0x1C: m6502_trb(c, ABS(c)); break; // TRB ABS
    case 0x14: m6502_trb(c, ZPG(c)); break; // TRB ZPG

    case 0x0C: m6502_tsb(c, ABS(c)); break; // TSB ABS
    case 0x04: m6502_tsb(c, ZPG(c)); break; // TSB ZPG

    // BBR
    case 0x0F: case 0x1F: case 0x2F: case 0x3F:
    case 0x4F: case 0x5F: case 0x6F: case 0x7F: {
        const u8 bit_no = opcode >> 4;
        const u8 val = m6502_rb(c, ZPG(c));
        const s8 addr = REL(c);

        m6502_branch(c, addr, ((val >> bit_no) & 1) == 0);
    } break;

    // BBS
    case 0x8F: case 0x9F: case 0xAF: case 0xBF:
    case 0xCF: case 0xDF: case 0xEF: case 0xFF: {
        const u8 bit_no = (opcode >> 4) - 8;
        const u8 val = m6502_rb(c, ZPG(c));
        const s8 addr = REL(c);

        m6502_branch(c, addr, ((val >> bit_no) & 1) == 1);
    } break;

    // RMB
    case 0x07: case 0x17: case 0x27: case 0x37:
    case 0x47: case 0x57: case 0x67: case 0x77: {
        const u8 bit_no = opcode >> 4;
        const u8 addr = ZPG(c);
        u8 val = m6502_rb(c, addr);
        val &= ~(1UL << bit_no);
        m6502_wb(c, addr, val);
    } break;

    // SMB
    case 0x87: case 0x97: case 0xA7: case 0xB7:
    case 0xC7: case 0xD7: case 0xE7: case 0xF7: {
        const u8 bit_no = (opcode >> 4) - 8;
        const u8 addr = ZPG(c);
        u8 val = m6502_rb(c, addr);
        val |= (1 << bit_no);
        m6502_wb(c, addr, val);
    } break;

    case 0xDB: c->stop = 1; break; // STP
    case 0xCB: c->wait = 1; break; // WAI

    case 0x72: m6502_adc(c, INZ(c)); break; // ADC INZ
    case 0x32: m6502_and(c, INZ(c)); break; // AND INZ
    case 0x3C: m6502_bit(c, ABX(c)); break; // BIT ABX
    case 0x34: m6502_bit(c, ZPX(c)); break; // BIT ZPX
    // when the BIT instruction is used with the immediate
    // addressing mode, the n and v flags are unaffected.
    case 0x89: c->zf = (m6502_rb(c, IMM(c)) & c->a) == 0; break; // BIT IMM
    case 0xD2: m6502_cmp(c, INZ(c), c->a); break; // CMP INZ
    case 0x3A: m6502_der(c, &c->a); break; // DEA
    case 0x1A: m6502_inr(c, &c->a); break; // INA
    case 0x52: m6502_eor(c, INZ(c)); break; // EOR INZ
    // JMP absolute indexed indirect
    case 0x7C: m6502_jmp(c, m6502_rw(c, ABS(c) + c->x)); break;
    case 0xB2: m6502_ldr(c, &c->a, INZ(c)); break; // LDA INZ
    case 0x12: m6502_ora(c, INZ(c)); break; // ORA INZ
    case 0xF2: m6502_sbc(c, INZ(c)); break; // SBC INZ
    case 0x92: m6502_wb(c, INZ(c), c->a); break; // STA INZ

    // one-byte NOP
    case 0x03: case 0x13: case 0x23: case 0x33: case 0x43: case 0x53:
    case 0x63: case 0x73: case 0x83: case 0x93: case 0xA3: case 0xB3:
    case 0xC3: case 0xD3: case 0xE3: case 0xF3:
    case 0x0B: case 0x1B: case 0x2B: case 0x3B: case 0x4B: case 0x5B:
    case 0x6B: case 0x7B: case 0x8B: case 0x9B: case 0xAB: case 0xBB:
    case 0xEB: case 0xFB:
        c->cyc += 1;
    break;

    // two-bytes NOP
    case 0x02: case 0x22: case 0x42: case 0x62: case 0x82: case 0xC2:
    case 0xE2:
        c->pc += 1;
        c->cyc += 2;
    break;

    case 0x44:
        c->pc += 1;
        c->cyc += 3;
    break;

    case 0x54: case 0xD4: case 0xF4:
        c->pc += 1;
        c->cyc += 4;
    break;

    // three-bytes NOP
    case 0x5C:
        c->pc += 2;
        c->cyc += 8;
    break;

    case 0xDC: case 0xFC:
        c->pc += 2;
        c->cyc += 4;
    break;

    default:
        // treat invalid opcodes as NOPs
        c->cyc += 2;
        // fprintf(stderr, "error: invalid 65C02 opcode 0x%02X\n", opcode);
    break;
    }
}

// interface

// initialises the emulator with default values
void m6502_init(m6502* const c) {
    c->pc = 0;
    c->a = 0;
    c->x = 0;
    c->y = 0;
    c->sp = 0xFD;
    c->cyc = 0;
    c->cf = 0;
    c->zf = 0;
    c->idf = 0;
    c->df = 0;
    c->bf = 0;
    c->vf = 0;
    c->nf = 0;
    c->page_crossed = 0;
    c->enable_bcd = 1;
    c->m65c02_mode = 0;
    c->stop = 0;
    c->wait = 0;
    c->userdata = NULL;
    c->read_byte = NULL;
    c->write_byte = NULL;
}

// executes one instruction stored at the address pointed by
// the program counter
void m6502_step(m6502* const c) {
    if (c->stop || c->wait) {
        return;
    }

    const u8 opcode = m6502_rb(c, c->pc++);
    if (c->m65c02_mode) {
            c->cyc += CYCLES_65C02[opcode];
    }
    else {
        c->cyc += CYCLES_6502[opcode];
    }
    c->page_crossed = 0;

    switch (opcode) {
    // storage
    case 0xA9: m6502_ldr(c, &c->a, IMM(c)); break; // LDA IMM
    case 0xA5: m6502_ldr(c, &c->a, ZPG(c)); break; // LDA ZPG
    case 0xB5: m6502_ldr(c, &c->a, ZPX(c)); break; // LDA ZPX
    case 0xAD: m6502_ldr(c, &c->a, ABS(c)); break; // LDA ABS
    case 0xBD: m6502_ldr(c, &c->a, ABX(c)); break; // LDA ABX
    case 0xB9: m6502_ldr(c, &c->a, ABY(c)); break; // LDA ABY
    case 0xA1: m6502_ldr(c, &c->a, INX(c)); break; // LDA INX
    case 0xB1: m6502_ldr(c, &c->a, INY(c)); break; // LDA INY

    case 0xA2: m6502_ldr(c, &c->x, IMM(c)); break; // LDX IMM
    case 0xA6: m6502_ldr(c, &c->x, ZPG(c)); break; // LDX ZPG
    case 0xB6: m6502_ldr(c, &c->x, ZPY(c)); break; // LDX ZPY
    case 0xAE: m6502_ldr(c, &c->x, ABS(c)); break; // LDX ABS
    case 0xBE: m6502_ldr(c, &c->x, ABY(c)); break; // LDX ABY

    case 0xA0: m6502_ldr(c, &c->y, IMM(c)); break; // LDY IMM
    case 0xA4: m6502_ldr(c, &c->y, ZPG(c)); break; // LDY ZPG
    case 0xB4: m6502_ldr(c, &c->y, ZPX(c)); break; // LDY ZPX
    case 0xAC: m6502_ldr(c, &c->y, ABS(c)); break; // LDY ABS
    case 0xBC: m6502_ldr(c, &c->y, ABX(c)); break; // LDY ABX

    case 0x85: m6502_wb(c, ZPG(c), c->a); break; // STA ZPG
    case 0x95: m6502_wb(c, ZPX(c), c->a); break; // STA ZPX
    case 0x8D: m6502_wb(c, ABS(c), c->a); break; // STA ABS
    case 0x9D: m6502_wb(c, ABX(c), c->a); break; // STA ABX
    case 0x99: m6502_wb(c, ABY(c), c->a); break; // STA ABY
    case 0x81: m6502_wb(c, INX(c), c->a); break; // STA INX
    case 0x91: m6502_wb(c, INY(c), c->a); break; // STA INY

    case 0x86: m6502_wb(c, ZPG(c), c->x); break; // STX ZPG
    case 0x96: m6502_wb(c, ZPY(c), c->x); break; // STX ZPY
    case 0x8E: m6502_wb(c, ABS(c), c->x); break; // STX ABS

    case 0x84: m6502_wb(c, ZPG(c), c->y); break; // STY ZPG
    case 0x94: m6502_wb(c, ZPX(c), c->y); break; // STY ZPX
    case 0x8C: m6502_wb(c, ABS(c), c->y); break; // STY ABS

    case 0xAA: c->x = c->a; set_zn(c, c->x); break; // TAX
    case 0xA8: c->y = c->a; set_zn(c, c->y); break; // TAY
    case 0xBA: c->x = c->sp; set_zn(c, c->x); break; // TSX
    case 0x8A: c->a = c->x; set_zn(c, c->a); break; // TXA
    case 0x9A: c->sp = c->x; break; // TXS
    case 0x98: c->a = c->y; set_zn(c, c->a); break; // TYA

    // math
    case 0x69: m6502_adc(c, IMM(c)); break; // ADC IMM
    case 0x65: m6502_adc(c, ZPG(c)); break; // ADC ZPG
    case 0x75: m6502_adc(c, ZPX(c)); break; // ADC ZPX
    case 0x6D: m6502_adc(c, ABS(c)); break; // ADC ABS
    case 0x7D: m6502_adc(c, ABX(c)); break; // ADC ABX
    case 0x79: m6502_adc(c, ABY(c)); break; // ADC ABY
    case 0x61: m6502_adc(c, INX(c)); break; // ADC INX
    case 0x71: m6502_adc(c, INY(c)); break; // ADC INY

    case 0xC6: m6502_dec_addr(c, ZPG(c)); break; // DEC ZPG
    case 0xD6: m6502_dec_addr(c, ZPX(c)); break; // DEC ZPX
    case 0xCE: m6502_dec_addr(c, ABS(c)); break; // DEC ABS
    case 0xDE: m6502_dec_addr(c, ABX(c)); break; // DEC ABX
    case 0xCA: m6502_der(c, &c->x); break; // DEX
    case 0x88: m6502_der(c, &c->y); break; // DEY

    case 0xE6: m6502_inc_addr(c, ZPG(c)); break; // INC ZPG
    case 0xF6: m6502_inc_addr(c, ZPX(c)); break; // INC ZPX
    case 0xEE: m6502_inc_addr(c, ABS(c)); break; // INC ABS
    case 0xFE: m6502_inc_addr(c, ABX(c)); break; // INC ABX
    case 0xE8: m6502_inr(c, &c->x); break; // INX
    case 0xC8: m6502_inr(c, &c->y); break; // INY

    case 0xE9: m6502_sbc(c, IMM(c)); break; // SBC IMM
    case 0xE5: m6502_sbc(c, ZPG(c)); break; // SBC ZPG
    case 0xF5: m6502_sbc(c, ZPX(c)); break; // SBC ZPX
    case 0xED: m6502_sbc(c, ABS(c)); break; // SBC ABS
    case 0xFD: m6502_sbc(c, ABX(c)); break; // SBC ABX
    case 0xF9: m6502_sbc(c, ABY(c)); break; // SBC ABY
    case 0xE1: m6502_sbc(c, INX(c)); break; // SBC INX
    case 0xF1: m6502_sbc(c, INY(c)); break; // SBC INY

    // bitwise
    case 0x29: m6502_and(c, IMM(c)); break; // AND IMM
    case 0x25: m6502_and(c, ZPG(c)); break; // AND ZPG
    case 0x35: m6502_and(c, ZPX(c)); break; // AND ZPX
    case 0x2D: m6502_and(c, ABS(c)); break; // AND ABS
    case 0x3D: m6502_and(c, ABX(c)); break; // AND ABX
    case 0x39: m6502_and(c, ABY(c)); break; // AND ABY
    case 0x21: m6502_and(c, INX(c)); break; // AND INX
    case 0x31: m6502_and(c, INY(c)); break; // AND INY

    case 0x0A: c->a = m6502_asl(c, c->a); break; // ASL ACC
    case 0x06: m6502_asl_addr(c, ZPG(c)); break; // ASL ZPG
    case 0x16: m6502_asl_addr(c, ZPX(c)); break; // ASL ZPX
    case 0x0E: m6502_asl_addr(c, ABS(c)); break; // ASL ABS
    case 0x1E: m6502_asl_addr(c, ABX(c)); break; // ASL ABX

    case 0x24: m6502_bit(c, ZPG(c)); break; // BIT ZPG
    case 0x2C: m6502_bit(c, ABS(c)); break; // BIT ABS

    case 0x49: m6502_eor(c, IMM(c)); break; // EOR IMM
    case 0x45: m6502_eor(c, ZPG(c)); break; // EOR ZPG
    case 0x55: m6502_eor(c, ZPX(c)); break; // EOR ZPX
    case 0x4D: m6502_eor(c, ABS(c)); break; // EOR ABS
    case 0x5D: m6502_eor(c, ABX(c)); break; // EOR ABX
    case 0x59: m6502_eor(c, ABY(c)); break; // EOR ABY
    case 0x41: m6502_eor(c, INX(c)); break; // EOR INX
    case 0x51: m6502_eor(c, INY(c)); break; // EOR INY

    case 0x4A: c->a = m6502_lsr(c, c->a); break; // LSR ACC
    case 0x46: m6502_lsr_addr(c, ZPG(c)); break; // LSR ZPG
    case 0x56: m6502_lsr_addr(c, ZPX(c)); break; // LSR ZPX
    case 0x4E: m6502_lsr_addr(c, ABS(c)); break; // LSR ABS
    case 0x5E: m6502_lsr_addr(c, ABX(c)); break; // LSR ABX

    case 0x09: m6502_ora(c, IMM(c)); break; // ORA IMM
    case 0x05: m6502_ora(c, ZPG(c)); break; // ORA ZPG
    case 0x15: m6502_ora(c, ZPX(c)); break; // ORA ZPX
    case 0x0D: m6502_ora(c, ABS(c)); break; // ORA ABS
    case 0x1D: m6502_ora(c, ABX(c)); break; // ORA ABX
    case 0x19: m6502_ora(c, ABY(c)); break; // ORA ABY
    case 0x01: m6502_ora(c, INX(c)); break; // ORA IMM
    case 0x11: m6502_ora(c, INY(c)); break; // ORA IMM

    case 0x2A: c->a = m6502_rol(c, c->a); break; // ROL ACC
    case 0x26: m6502_rol_addr(c, ZPG(c)); break; // ROL ZPG
    case 0x36: m6502_rol_addr(c, ZPX(c)); break; // ROL ZPX
    case 0x2E: m6502_rol_addr(c, ABS(c)); break; // ROL ABS
    case 0x3E: m6502_rol_addr(c, ABX(c)); break; // ROL ABX

    case 0x6A: c->a = m6502_ror(c, c->a); break; // ROR ACC
    case 0x66: m6502_ror_addr(c, ZPG(c)); break; // ROR ZPG
    case 0x76: m6502_ror_addr(c, ZPX(c)); break; // ROR ZPX
    case 0x6E: m6502_ror_addr(c, ABS(c)); break; // ROR ABS
    case 0x7E: m6502_ror_addr(c, ABX(c)); break; // ROR ABX

    // branch
    case 0x90: m6502_branch(c, REL(c), c->cf == 0); break; // BCC REL
    case 0xB0: m6502_branch(c, REL(c), c->cf == 1); break; // BCS REL
    case 0xD0: m6502_branch(c, REL(c), c->zf == 0); break; // BNE REL
    case 0xF0: m6502_branch(c, REL(c), c->zf == 1); break; // BEQ REL
    case 0x10: m6502_branch(c, REL(c), c->nf == 0); break; // BPL REL
    case 0x30: m6502_branch(c, REL(c), c->nf == 1); break; // BMI REL
    case 0x50: m6502_branch(c, REL(c), c->vf == 0); break; // BVC REL
    case 0x70: m6502_branch(c, REL(c), c->vf == 1); break; // BVS REL

    // jump
    case 0x4C: m6502_jmp(c, ABS(c)); break; // JMP
    case 0x6C: m6502_jmp(c, m6502_rw_bug(c, ABS(c))); break; // JMP
    case 0x20: m6502_jsr(c, ABS(c)); break; // JSR
    case 0x40: m6502_rti(c); break; // RTI
    case 0x60: m6502_rts(c); break; // RTS

    // registers
    case 0x38: c->cf = 1; break; // SEC
    case 0x18: c->cf = 0; break; // CLC
    case 0xF8: c->df = 1; break; // SED
    case 0xD8: c->df = 0; break; // CLD
    case 0x78: c->idf = 1; break; // SEI
    case 0x58: c->idf = 0; break; // CLI
    case 0xB8: c->vf = 0; break; // CLV

    case 0xC9: m6502_cmp(c, IMM(c), c->a); break; // CMP IMM
    case 0xC5: m6502_cmp(c, ZPG(c), c->a); break; // CMP ZPG
    case 0xD5: m6502_cmp(c, ZPX(c), c->a); break; // CMP ZPX
    case 0xCD: m6502_cmp(c, ABS(c), c->a); break; // CMP ABS
    case 0xDD: m6502_cmp(c, ABX(c), c->a); break; // CMP ABX
    case 0xD9: m6502_cmp(c, ABY(c), c->a); break; // CMP ABY
    case 0xC1: m6502_cmp(c, INX(c), c->a); break; // CMP INX
    case 0xD1: m6502_cmp(c, INY(c), c->a); break; // CMP INY
    case 0xE0: m6502_cmp(c, IMM(c), c->x); break; // CPX IMM
    case 0xE4: m6502_cmp(c, ZPG(c), c->x); break; // CPX ZPG
    case 0xEC: m6502_cmp(c, ABS(c), c->x); break; // CPX ABS
    case 0xC0: m6502_cmp(c, IMM(c), c->y); break; // CPY IMM
    case 0xC4: m6502_cmp(c, ZPG(c), c->y); break; // CPY ZPG
    case 0xCC: m6502_cmp(c, ABS(c), c->y); break; // CPY ABS

    // stack
    case 0x48: push_byte(c, c->a); break; // PHA
    case 0x68: c->a = pull_byte(c); set_zn(c, c->a); break; // PLA
    case 0x08: c->bf = 1; push_byte(c, get_flags(c)); break; // PHP
    case 0x28: set_flags(c, pull_byte(c)); break; // PLP

    // system
    case 0x00: c->bf = 1; c->pc += 1; interrupt(c, 0xFFFE); break; // BRK
    case 0xEA: break; // NOP

    default:
        if (c->m65c02_mode) {
            execute_m65c02_opcode(c, opcode);
        }
        else {
            // for now, treat all invalid opcodes as NOPs in 6502 mode:
            c->cyc += 2;
            // fprintf(stderr, "error: invalid opcode 0x%02X\n", opcode);
        }
    break;
    }

    // on certain instructions, if a page is crossed; the instruction
    // takes additional cycles to execute:
    if (c->page_crossed) {
        c->cyc += INSTRUCTIONS_PAGE_CROSSED_CYCLES[opcode];
    }
}

// prints to the standard output the current state of the emulation,
// including registers and flags
void m6502_debug_output(m6502* const c) {
    char flags[] = "........";

    if (c->nf) flags[0] = 'n';
    if (c->vf) flags[1] = 'v';
    flags[2] = '1'; // always set
    if (c->bf) flags[3] = 'b';
    if (c->df) flags[4] = 'd';
    if (c->idf) flags[5] = 'i';
    if (c->zf) flags[6] = 'z';
    if (c->cf) flags[7] = 'c';

    printf("PC:%04X (%02X %02X %02X) ",
        c->pc,
        m6502_rb(c, c->pc),
        m6502_rb(c, c->pc + 1),
        m6502_rb(c, c->pc + 2));

    // the following line helps to compare with Nintendulator logs
    const u16 cyc = (c->cyc * 3) % 341;

    printf("SP:%02X A:%02X X:%02X Y:%02X P:%02X (%s) CYC:%d\n",
        c->sp, c->a, c->x, c->y, get_flags(c), flags, cyc);
}

// generates an NMI interrupt
void m6502_gen_nmi(m6502* const c) {
    c->bf = 0;
    interrupt(c, 0xFFFA);
}

// generates a RESET interrupt
void m6502_gen_res(m6502* const c) {
    c->bf = 0;
    interrupt(c, 0xFFFC);
    c->stop = 0;
    c->cyc = 0;
}

// generates an IRQ interrupt
void m6502_gen_irq(m6502* const c) {
    if (c->idf == 0) {
        c->bf = 0;
        interrupt(c, 0xFFFE);
    }
}
