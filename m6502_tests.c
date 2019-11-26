#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "m6502.h"

static m6502 cpu;

// memory callbacks
#define MEMORY_SIZE 0x10000
static uint8_t* memory;

static uint8_t rb(void* userdata, uint16_t addr) {
    return memory[addr];
}

static void wb(void* userdata, uint16_t addr, uint8_t val) {
    memory[addr] = val;
}

static int load_file_into_memory(const char* filename, uint16_t addr) {
    FILE* f = fopen(filename, "rb");
    if (f == NULL) {
        fprintf(stderr, "error: can't open file '%s'.\n", filename);
        return 1;
    }

    // file size check:
    fseek(f, 0, SEEK_END);
    unsigned long file_size = ftell(f);
    rewind(f);

    if (file_size + addr > MEMORY_SIZE) {
        fprintf(stderr, "error: file %s can't fit in memory.\n", filename);
        return 1;
    }

    // copying the bytes in the memory:
    size_t result = fread(&memory[addr], sizeof(uint8_t), file_size, f);
    if (result != file_size) {
        fprintf(stderr, "error: while reading file '%s'\n", filename);
        return 1;
    }

    fclose(f);
    return 0;
}

static int test_allsuitea(unsigned long expected_cyc) {
    printf("AllSuiteA: ");

    memset(memory, 0, MEMORY_SIZE);
    load_file_into_memory("programs/AllSuiteA.bin", 0x4000);
    m6502_init(&cpu);
    cpu.read_byte = &rb;
    cpu.write_byte = &wb;
    m6502_gen_res(&cpu);

    int nb_instructions_executed = 0;
    while (true) {
        m6502_step(&cpu);

        nb_instructions_executed += 1;

        if (cpu.pc == 0x45C0) {
            if (rb(&cpu, 0x0210) == 0xFF) {
                printf("PASS");
            }
            else {
                printf("FAIL");
            }
            break;
        }
    }

    long long diff = expected_cyc - cpu.cyc;
    printf(" (%d instructions executed on %lu cycles, "
        " expected=%lu, diff=%lld)\n",
        nb_instructions_executed, cpu.cyc,
        expected_cyc, diff);

    return cpu.cyc != expected_cyc;
}

static int test_6502_functional_test(unsigned long expected_cyc) {
    printf("6502_functional_test: ");

    memset(memory, 0, MEMORY_SIZE);
    if (load_file_into_memory("programs/6502_65C02_functional_tests/bin_files/6502_functional_test.bin", 0) != 0) {
        return 1;
    }
    m6502_init(&cpu);
    cpu.read_byte = &rb;
    cpu.write_byte = &wb;
    cpu.pc = 0x400;

    int nb_instructions_executed = 0;
    uint16_t previous_pc = 0;
    while (true) {
        m6502_step(&cpu);

        nb_instructions_executed += 1;

        // if the program is trapped somewhere, print and exit
        if (previous_pc == cpu.pc) {
            if (cpu.pc == 0x3469) {
                printf("PASS");
                break;
            }
            printf("FAIL (trapped at 0x%04X)", cpu.pc);
            break;
        }
        previous_pc = cpu.pc;
    }

    long long diff = expected_cyc - cpu.cyc;
    printf(" (%d instructions executed on %lu cycles, "
        " expected=%lu, diff=%lld)\n",
        nb_instructions_executed, cpu.cyc,
        expected_cyc, diff);

    return cpu.cyc != expected_cyc;
}

static int test_6502_decimal_test(unsigned long expected_cyc) {
    printf("6502_decimal_test: ");

    memset(memory, 0, MEMORY_SIZE);
    if (load_file_into_memory("programs/6502_decimal_test.bin", 0x200) != 0) {
        return 1;
    }
    m6502_init(&cpu);
    cpu.read_byte = &rb;
    cpu.write_byte = &wb;
    cpu.pc = 0x200;

    int nb_instructions_executed = 0;
    while (true) {
        m6502_step(&cpu);

        nb_instructions_executed += 1;

        if (cpu.pc == 0x024b) {
            printf("%s", cpu.a == 0 ? "PASS" : "FAIL");
            break;
        }
    }

    long long diff = expected_cyc - cpu.cyc;
    printf(" (%d instructions executed on %lu cycles, "
        " expected=%lu, diff=%lld)\n",
        nb_instructions_executed, cpu.cyc,
        expected_cyc, diff);

    return cpu.cyc != expected_cyc;
}

static int test_timingtest(unsigned long expected_cyc) {
    printf("timingtest: ");

    memset(memory, 0, MEMORY_SIZE);
    if (load_file_into_memory("programs/timingtest/timingtest-1.bin", 0x1000) != 0) {
        return 1;
    }
    m6502_init(&cpu);
    cpu.read_byte = &rb;
    cpu.write_byte = &wb;
    cpu.pc = 0x1000;

    int nb_instructions_executed = 0;
    while (true) {
        m6502_step(&cpu);
        nb_instructions_executed += 1;

        if (cpu.pc == 0x1269) {
            printf("%s", cpu.cyc == 1141 ? "PASS" : "FAIL");
            break;
        }
    }

    long long diff = expected_cyc - cpu.cyc;
    printf(" (%d instructions executed on %lu cycles, "
        " expected=%lu, diff=%lld)\n",
        nb_instructions_executed, cpu.cyc,
        expected_cyc, diff);

    return cpu.cyc != expected_cyc;
}

static int test_65C02_extended_opcodes_test(unsigned long expected_cyc) {
    printf("65C02_extended_opcodes_test: ");

    memset(memory, 0, MEMORY_SIZE);
    if (load_file_into_memory("programs/6502_65C02_functional_tests/bin_files/65C02_extended_opcodes_test.bin", 0) != 0) {
        return 1;
    }
    m6502_init(&cpu);
    cpu.read_byte = &rb;
    cpu.write_byte = &wb;
    cpu.pc = 0x400;
    cpu.m65c02_mode = 1;

    int nb_instructions_executed = 0;
    uint16_t previous_pc = 0;
    while (true) {
        m6502_step(&cpu);

        nb_instructions_executed += 1;

        // if the program is trapped somewhere, print and exit
        if (previous_pc == cpu.pc) {
            if (cpu.pc == 0x24F1) {
                printf("PASS");
                break;
            }
            printf("FAIL (trapped at 0x%04X)", cpu.pc);
            break;
        }
        previous_pc = cpu.pc;
    }

    long long diff = expected_cyc - cpu.cyc;
    printf(" (%d instructions executed on %lu cycles, "
        " expected=%lu, diff=%lld)\n",
        nb_instructions_executed, cpu.cyc,
        expected_cyc, diff);

    return cpu.cyc != expected_cyc;
}

static int test_6502_interrupt_test(unsigned long expected_cyc) {
    printf("6502_interrupt_test: ");

    memset(memory, 0, MEMORY_SIZE);
    load_file_into_memory("programs/6502_interrupt_test.bin", 0);
    m6502_init(&cpu);
    cpu.read_byte = &rb;
    cpu.write_byte = &wb;
    cpu.pc = 0x400; // actually start at 0x3f5?

    int nb_instructions_executed = 0;
    uint16_t previous_pc = 0;
    while (true) {
        // m6502_debug_output(&cpu);
        m6502_step(&cpu);

        nb_instructions_executed += 1;

        // if the program is trapped somewhere, print and exit
        if (previous_pc == cpu.pc) {
            if (cpu.pc == 0x06f5) {
                printf("PASS");
                break;
            }
            printf("FAIL (trapped at 0x%04X)", cpu.pc);
            break;
        }
        previous_pc = cpu.pc;
    }

    long long diff = expected_cyc - cpu.cyc;
    printf(" (%d instructions executed on %lu cycles, "
        " expected=%lu, diff=%lld)\n",
        nb_instructions_executed, cpu.cyc,
        expected_cyc, diff);

    return cpu.cyc != expected_cyc;
}

// The following test has been assembled with this configuration:
// cputype = 1         ; 0 = 6502, 1 = 65C02, 2 = 65C816
// vld_bcd = 0         ; 0 = allow invalid bcd, 1 = valid bcd only
// chk_a   = 1         ; check accumulator
// chk_n   = 1         ; check sign (negative) flag
// chk_v   = 0         ; check overflow flag
// chk_z   = 1         ; check zero flag
// chk_c   = 1         ; check carry flag
static int test_65C02_decimal_test(unsigned long expected_cyc) {
    printf("65C02_decimal_test: ");

    memset(memory, 0, MEMORY_SIZE);
    if (load_file_into_memory("programs/65C02_decimal_test.bin", 0x200) != 0) {
        return 1;
    }
    m6502_init(&cpu);
    cpu.read_byte = &rb;
    cpu.write_byte = &wb;
    cpu.pc = 0x200;
    cpu.m65c02_mode = 1;

    int nb_instructions_executed = 0;
    while (true) {
        m6502_step(&cpu);

        nb_instructions_executed += 1;

        if (cpu.pc == 0x024b) {
            printf("%s", cpu.a == 0 ? "PASS" : "FAIL");
            break;
        }
    }

    long long diff = expected_cyc - cpu.cyc;
    printf(" (%d instructions executed on %lu cycles, "
        " expected=%lu, diff=%lld)\n",
        nb_instructions_executed, cpu.cyc,
        expected_cyc, diff);

    return cpu.cyc != expected_cyc;
}

// static int test_65c02_timingtest(unsigned long expected_cyc) {
//     printf("timingtest: ");

//     memset(memory, 0, MEMORY_SIZE);
//     load_file_into_memory("programs/65c02timing/65c02timing.bin", 0x6000);
//     m6502_init(&cpu);
//     cpu.read_byte = &rb;
//     cpu.write_byte = &wb;
//     cpu.pc = 0x6000;
//     cpu.m65c02_mode = 1;

//     int nb_instructions_executed = 0;
//     uint16_t previous_pc = 0;
//     while (true) {
//         // m6502_debug_output(&cpu);
//         m6502_step(&cpu);
//         nb_instructions_executed += 1;

//         // if the program is trapped somewhere, print and exit
//         if (previous_pc == cpu.pc) {
//             printf("%s", cpu.y == 1 ? "PASS" : "FAIL");
//             break;
//         }
//         previous_pc = cpu.pc;
//     }

//     long long diff = expected_cyc - cpu.cyc;
//     printf(" (%d instructions executed on %lu cycles,"
//         " expected=%lu, diff=%lld)\n",
//         nb_instructions_executed, cpu.cyc,
//         expected_cyc, diff);
// }

int main(void) {
    memory = malloc(MEMORY_SIZE);

    int r = 0;
    r += test_allsuitea(1946LU);
    r += test_6502_functional_test(96241367LU); // same cycle count on fake6502
    r += test_6502_decimal_test(46089505LU);
    r += test_timingtest(1141LU);
    r += test_65C02_extended_opcodes_test(66886142LU);
    // r += test_6502_interrupt_test(0LU);
    // r += test_65C02_decimal_test(0LU);
    // r += test_65c02_timingtest(0LU);

    free(memory);

    return r != 0;
}
