#include "m6502.h"

// memory helpers
static u8 m6502_rb(m6502* const c, const u16 addr);
static u16 m6502_rw(m6502* const c, const u16 addr);
static u16 m6502_rw_bug(m6502* const c, const u16 addr);
static void m6502_wb(m6502* const c, const u16 addr, u8 val);

// adressing modes helpers (return addresses)
static u16 IMM(m6502* const c); // immediate
static u8 ZPG(m6502* const c); // zero page
static u8 ZPX(m6502* const c); // zero page + x
static u8 ZPY(m6502* const c); // zero page + y
static u16 ABS(m6502* const c); // absolute
static u16 ABX(m6502* const c); // absolute + x
static u16 ABY(m6502* const c); // absolute + y
static u16 INX(m6502* const c); // indexed indirect x
static u16 INY(m6502* const c); // indirect indexed y
static s8 REL(m6502* const c); // relative

// stack
static void push_byte(m6502* const c, const u8 val);
static void push_word(m6502* const c, const u16 val);
static u8 pull_byte(m6502* const c);
static u16 pull_word(m6502* const c);

// flag helpers
static void set_zn(m6502* const c, const u8 val);
static u8 get_flags(m6502* const c, const bool b_flag);

// opcodes - storage
static void m6502_ldr(m6502* const c, u8* const reg, const u16 addr);

// opcodes - math
// static u8 bcd(const u8 val);
static void m6502_adc(m6502* const c, const u16 addr);
static void m6502_sbc(m6502* const c, const u16 addr);
static u8 m6502_inc(m6502* const c, const u8 val);
static void m6502_inc_addr(m6502* const c, const u16 addr);
static void m6502_inr(m6502* const c, u8* const reg);
static u8 m6502_dec(m6502* const c, const u8 val);
static void m6502_dec_addr(m6502* const c, const u16 addr);
static void m6502_der(m6502* const c, u8* const reg);

// opcodes - bitwise
static void m6502_and(m6502* const c, const u16 addr);
static u8 m6502_asl(m6502* const c, const u8 val);
static void m6502_asl_addr(m6502* const c, const u16 addr);
static void m6502_bit(m6502* const c, const u16 addr);
static void m6502_eor(m6502* const c, const u16 addr);
static u8 m6502_lsr(m6502* const c, const u8 val);
static void m6502_lsr_addr(m6502* const c, const u16 addr);
static void m6502_ora(m6502* const c, const u16 addr);
static u8 m6502_rol(m6502* const c, const u8 val);
static void m6502_rol_addr(m6502* const c, const u16 addr);
static u8 m6502_ror(m6502* const c, const u8 val);
static void m6502_ror_addr(m6502* const c, const u16 addr);

// opcodes - branch
static void m6502_branch(m6502* const c, const s8 addr, const bool condition);

// opcodes - jump
static void m6502_jmp(m6502* const c, const u16 addr);
static void m6502_jsr(m6502* const c, const u16 addr);
static void m6502_rti(m6502* const c);
static void m6502_rts(m6502* const c);

// opcodes - registers
static void m6502_cmp(m6502* const c, const u16 addr, const u8 reg_value);

// opcodes - stack
static void m6502_pha(m6502* const c);
static void m6502_pla(m6502* const c);
static void m6502_php(m6502* const c);
static void m6502_plp(m6502* const c);

// the number of cycles an instruction takes
static const u8 INSTRUCTIONS_CYCLES[] = {
    7, 6, 2, 8, 3, 3, 5, 5, 3, 2, 2, 2, 4, 4, 6, 6,
    2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    6, 6, 2, 8, 3, 3, 5, 5, 4, 2, 2, 2, 4, 4, 6, 6,
    2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    6, 6, 2, 8, 3, 3, 5, 5, 3, 2, 2, 2, 3, 4, 6, 6,
    2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    6, 6, 2, 8, 3, 3, 5, 5, 4, 2, 2, 2, 5, 4, 6, 6,
    2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
    2, 6, 2, 6, 4, 4, 4, 4, 2, 5, 2, 5, 5, 5, 5, 5,
    2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
    2, 5, 2, 5, 4, 4, 4, 4, 2, 4, 2, 4, 4, 4, 4, 4,
    2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
    2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
    2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
};

// the number of additional cycles an instruction takes if a page is crossed
static const u8 INSTRUCTIONS_PAGE_CROSSED_CYCLES[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0,
};

static const u16 STACK_START_ADDR = 0x100;

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
    c->userdata = NULL;
    c->read_byte = NULL;
    c->write_byte = NULL;
}

// executes one instruction stored at the address pointed by
// the program counter
void m6502_step(m6502* const c) {
    const u8 opcode = m6502_rb(c, c->pc++);
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
    case 0x48: m6502_pha(c); break; // PHA
    case 0x68: m6502_pla(c); break; // PLA
    case 0x08: m6502_php(c); break; // PHP
    case 0x28: m6502_plp(c); break; // PLP

    // system
    case 0x00: m6502_gen_brk(c); break; // BRK
    case 0xEA: break; // NOP

    default: fprintf(stderr, "error: invalid opcode $%02X\n", opcode); break;
    }

    c->cyc += INSTRUCTIONS_CYCLES[opcode];

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
        c->sp, c->a, c->x, c->y, get_flags(c, 0), flags, cyc);
}

// memory helpers (the only functions to use read_byte and write_byte
// function pointers)

// reads a byte from memory
u8 m6502_rb(m6502* const c, const u16 addr) {
    return c->read_byte(c->userdata, addr);
}

// reads a word from memory
u16 m6502_rw(m6502* const c, const u16 addr) {
    return (c->read_byte(c->userdata, addr + 1) << 8) |
            c->read_byte(c->userdata, addr);
}

// emulates a 6502 bug where the low byte wrapped without incrementing
// the high byte
u16 m6502_rw_bug(m6502* const c, const u16 addr) {
    const u16 hi_addr = (addr & 0xFF00) | ((addr + 1) & 0xFF);
    return (c->read_byte(c->userdata, hi_addr) << 8) |
            c->read_byte(c->userdata, addr);
}

// writes a byte to memory
void m6502_wb(m6502* const c, const u16 addr, u8 val) {
    c->write_byte(c->userdata, addr, val);
}

// addressing modes helpers
u16 IMM(m6502* const c) { // immediate
    return c->pc++;
}

u8 ZPG(m6502* const c) { // zero page
    return m6502_rb(c, c->pc++);
}

u8 ZPX(m6502* const c) { // zero page + x
    return m6502_rb(c, c->pc++) + c->x;
}

u8 ZPY(m6502* const c) { // zero page + y
    return m6502_rb(c, c->pc++) + c->y;
}

u16 ABS(m6502* const c) { // absolute
    const u16 addr = m6502_rw(c, c->pc);
    c->pc += 2;
    return addr;
}

u16 ABX(m6502* const c) { // absolute + x
    const u16 addr = ABS(c) + c->x;
    c->page_crossed = ((addr - c->x) & 0xFF00) != (addr & 0xFF00);
    return addr;
}

u16 ABY(m6502* const c) { // absolute + y
    const u16 addr = ABS(c) + c->y;
    c->page_crossed = ((addr - c->y) & 0xFF00) != (addr & 0xFF00);
    return addr;
}

u16 INX(m6502* const c) { // indexed indirect x
    return m6502_rw_bug(c, (m6502_rb(c, c->pc++) + c->x) & 0xFF);
}

u16 INY(m6502* const c) { // indirect indexed y
    const u16 addr = m6502_rw_bug(c, m6502_rb(c, c->pc++)) + c->y;
    c->page_crossed = ((addr - c->y) & 0xFF00) != (addr & 0xFF00);
    return addr;
}

s8 REL(m6502* const c) { // relative
    return (s8) m6502_rb(c, c->pc++);
}

// stack

// pushes a byte onto the stack
void push_byte(m6502* const c, const u8 val) {
    const u16 addr = STACK_START_ADDR + c->sp--;
    m6502_wb(c, addr, val);
}

// pushes a word onto the stack
void push_word(m6502* const c, const u16 val) {
    const u16 addr = STACK_START_ADDR + c->sp;
    m6502_wb(c, addr, val >> 8);
    m6502_wb(c, addr - 1, val & 0xFF);
    c->sp -= 2;
}

// pulls a byte from the stack
u8 pull_byte(m6502* const c) {
    const u16 addr = STACK_START_ADDR + ++c->sp;
    return m6502_rb(c, addr);
}

// pulls a word from the stack
u16 pull_word(m6502* const c) {
    return pull_byte(c) | (pull_byte(c) << 8);
}

// flag helpers

// helper to quickly set Z/N flags according to a byte value
void set_zn(m6502* const c, const u8 val) {
    c->zf = val == 0;
    c->nf = val >> 7;
}

// returns flags status in one byte
u8 get_flags(m6502* const c, const bool b_flag) {
    u8 flags = 0;
    flags |= c->nf << 7;
    flags |= c->vf << 6;
    flags |= 1 << 5; // bit 5 is always set
    flags |= b_flag << 4; // clear if interrupt vectoring, set if BRK or PHP
    flags |= c->df << 3;
    flags |= c->idf << 2;
    flags |= c->zf << 1;
    flags |= c->cf << 0;
    return flags;
}

// interrupts

// generates an NMI interrupt
void m6502_gen_nmi(m6502* const c) {
    push_word(c, c->pc + 1);
    push_byte(c, get_flags(c, 0));
    c->idf = 1;
    c->pc = m6502_rw(c, 0xFFFA);
}

// generates a RESET interrupt
void m6502_gen_res(m6502* const c) {
    c->pc = m6502_rw(c, 0xFFFC);
}

// generates an IRQ interrupt
void m6502_gen_irq(m6502* const c) {
    push_word(c, c->pc + 1);
    push_byte(c, get_flags(c, 0));
    c->idf = 1;
    c->pc = m6502_rw(c, 0xFFFE);
}

// generates an BRK interrupt
void m6502_gen_brk(m6502* const c) {
    push_word(c, c->pc + 1);
    push_byte(c, get_flags(c, 1));
    c->idf = 1;
    c->pc = m6502_rw(c, 0xFFFE);
}

// opcodes - storage

// loads a register with a byte
void m6502_ldr(m6502* const c, u8* const reg, const u16 addr) {
    *reg = m6502_rb(c, addr);
    set_zn(c, *reg);
}

// opcodes - math

// returns the "binary coded decimal": treats the eight bits as two
// groups of four bits, for example: 0b01000110 => 0100 0110 => 46
// meaning: 46 (hex) is converted to 46 (decimal)
// u8 bcd(const u8 val) {
//     return 10 * (val >> 4) + (val & 0x0F);
// }

// adds a byte (+ carry flag) to the accumulator
void m6502_adc(m6502* const c, const u16 addr) {
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

        if (((c->a + val + cy) & 0xFF) == 0) {
            c->zf = 1;
        }
        else if (ah & 0x8) {
            c->nf = 1;
        }

        c->vf = ~(c->a ^ val) & (c->a ^ (ah << 4)) & 0x80;

        if (ah > 9) {
            ah += 0x06;
        }
        if (ah > 0xF) {
            c->cf = 1;
        }

        c->a = (ah << 4) | (al & 0xF);
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
void m6502_sbc(m6502* const c, const u16 addr) {
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
        if ((result & 0xFF) == 0) {
            c->zf = 1;
        }
        else if (result & 0x80) {
            c->nf = 1;
        }

        if ((c->a ^ val) & (c->a ^ result) & 0x80) {
            c->vf = 1;
        }
        c->cf = !(result & 0xFF00);
        if (ah >> 7) {
            ah -= 6;
        }

        c->a = (ah << 4) | (al & 0xF);
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
u8 m6502_inc(m6502* const c, const u8 val) {
    const u8 result = val + 1;
    set_zn(c, result);
    return result;
}

// increments a byte in memory
void m6502_inc_addr(m6502* const c, const u16 addr) {
    const u8 val = m6502_rb(c, addr);
    m6502_wb(c, addr, m6502_inc(c, val));
}

// increments a register
void m6502_inr(m6502* const c, u8* const reg) {
    *reg = m6502_inc(c, *reg);
}

// decrements a byte and returns the decremented value
u8 m6502_dec(m6502* const c, const u8 val) {
    const u8 result = val - 1;
    set_zn(c, result);
    return result;
}

// decrements a byte in memory
void m6502_dec_addr(m6502* const c, const u16 addr) {
    const u8 val = m6502_rb(c, addr);
    m6502_wb(c, addr, m6502_dec(c, val));
}

// decrements a register
void m6502_der(m6502* const c, u8* const reg) {
    *reg = m6502_dec(c, *reg);
}

// opcodes - bitwise

// executes a logical AND between the accumulator and a byte in memory
void m6502_and(m6502* const c, const u16 addr) {
    const u8 val = m6502_rb(c, addr);
    c->a &= val;
    set_zn(c, c->a);
}

// shifts left the contents of a byte and returns it
u8 m6502_asl(m6502* const c, const u8 val) {
    const u8 result = val << 1;
    c->cf = val >> 7;
    set_zn(c, result);
    return result;
}

// shifts left the contents of a byte in memory
void m6502_asl_addr(m6502* const c, const u16 addr) {
    const u8 val = m6502_rb(c, addr);
    m6502_wb(c, addr, m6502_asl(c, val));
}

// sets the Z flag as though the value at addr were ANDed with register A
void m6502_bit(m6502* const c, const u16 addr) {
    const u8 val = m6502_rb(c, addr);
    c->vf = (val >> 6) & 1;
    c->zf = (val & c->a) == 0;
    c->nf = val >> 7;
}

// executes an exclusive OR on register A and a byte in memory
void m6502_eor(m6502* const c, const u16 addr) {
    c->a ^= m6502_rb(c, addr);
    set_zn(c, c->a);
}

// shifts right the contents of a byte and returns it
u8 m6502_lsr(m6502* const c, const u8 val) {
    const u8 result = val >> 1;
    c->cf = val & 1;
    set_zn(c, result);
    return result;
}

// shifts right the contents of a byte in memory
void m6502_lsr_addr(m6502* const c, const u16 addr) {
    const u8 val = m6502_rb(c, addr);
    m6502_wb(c, addr, m6502_lsr(c, val));
}

// executes an inclusive OR on register A and a byte in memory
void m6502_ora(m6502* const c, const u16 addr) {
    c->a |= m6502_rb(c, addr);
    set_zn(c, c->a);
}

// rotates left a byte and returns the rotated value
u8 m6502_rol(m6502* const c, const u8 val) {
    u8 result = val << 1;
    result |= c->cf;
    c->cf = val >> 7;
    set_zn(c, result);
    return result;
}

// rotates left a byte in memory
void m6502_rol_addr(m6502* const c, const u16 addr) {
    const u8 val = m6502_rb(c, addr);
    m6502_wb(c, addr, m6502_rol(c, val));
}

// rotates right a byte and returns the rotated value
u8 m6502_ror(m6502* const c, const u8 val) {
    u8 result = val >> 1;
    result |= c->cf << 7;
    c->cf = val & 1;
    set_zn(c, result);
    return result;
}

// rotates right a byte in memory
void m6502_ror_addr(m6502* const c, const u16 addr) {
    const u8 val = m6502_rb(c, addr);
    m6502_wb(c, addr, m6502_ror(c, val));
}

// opcodes - branch

// adds to PC a *signed* byte if condition is true.
void m6502_branch(m6502* const c, const s8 addr, const bool condition) {
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
void m6502_jmp(m6502* const c, const u16 addr) {
    c->pc = addr;
}

// jumps to a subroutine
void m6502_jsr(m6502* const c, const u16 addr) {
    push_word(c, c->pc - 1);
    c->pc = addr;
}

// returns from an interrupt
void m6502_rti(m6502* const c) {
    m6502_plp(c); // pull flags
    c->pc = pull_word(c);
}

// returns from a subroutine
void m6502_rts(m6502* const c) {
    c->pc = pull_word(c) + 1;
}

// opcodes - registers

// compares the value of a register with another byte in memory
void m6502_cmp(m6502* const c, const u16 addr, const u8 reg_value) {
    const u8 val = m6502_rb(c, addr);
    const u8 result = reg_value - val;
    c->cf = reg_value >= val;
    set_zn(c, result);
}

// opcodes - stack

// pushes a copy of register A onto the stack
void m6502_pha(m6502* const c) {
    push_byte(c, c->a);
}

// pulls a byte from stack into the register A
void m6502_pla(m6502* const c) {
    c->a = pull_byte(c);
    set_zn(c, c->a);
}

// pushes a copy of the flags onto the stack
void m6502_php(m6502* const c) {
    push_byte(c, get_flags(c, 1));
}

// pulls a copy of the flags from the stack
void m6502_plp(m6502* const c) {
    const u8 flags = pull_byte(c);
    c->nf = (flags >> 7) & 1;
    c->vf = (flags >> 6) & 1;
    c->df = (flags >> 3) & 1;
    c->idf = (flags >> 2) & 1;
    c->zf = (flags >> 1) & 1;
    c->cf = (flags >> 0) & 1;
}
