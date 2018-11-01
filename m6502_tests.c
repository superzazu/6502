#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "m6502.h"

// memory callbacks
#define MEMORY_SIZE 0x20000
static u8* memory;

static u8 rb(void* userdata, const u16 addr) {
    return memory[addr];
}

static void wb(void* userdata, const u16 addr, const u8 val) {
    memory[addr] = val;
}

int load_file_into_memory(const char* filename, const u16 addr) {
    FILE *f = fopen(filename, "rb");
    if (f == NULL) {
        fprintf(stderr, "error: can't open file '%s'.\n", filename);
        return 1;
    }

    // obtain file size + size check
    fseek(f, 0, SEEK_END);
    unsigned long file_size = ftell(f);
    rewind(f);

    if (file_size + addr > MEMORY_SIZE) {
        fprintf(stderr, "error: file %s can't fit in memory.\n",
               filename);
        return 1;
    }

    // loading up the file contents in our array
    size_t result = fread(&memory[addr], sizeof(u8), file_size, f);
    if (result != file_size) {
        fprintf(stderr, "error: while reading file '%s'\n", filename);
        return 1;
    }

    fclose(f);
    return 0;
}

int main(int argc, char **argv) {
    m6502 cpu;

    memory = malloc(MEMORY_SIZE);

    int nb_instructions_executed = 0;

    // AllSuiteA test
    printf("TEST 'AllSuiteA': ");

    memset(memory, 0, MEMORY_SIZE);
    load_file_into_memory("programs/AllSuiteA.bin", 0x4000);
    m6502_init(&cpu);
    cpu.read_byte = &rb;
    cpu.write_byte = &wb;
    m6502_gen_res(&cpu);

    nb_instructions_executed = 0;
    while (true) {
        m6502_step(&cpu);

        nb_instructions_executed += 1;

        if (cpu.pc == 0x45C0) {
            if (rb(&cpu, 0x0210) == 0xFF) {
                printf("SUCCESS");
            }
            else {
                printf("FAIL");
            }
            break;
        }
    }
    printf(" (executed %d opcodes)\n", nb_instructions_executed);

    // Klaus' test
    printf("TEST '6502_functional_test': ");

    memset(memory, 0, MEMORY_SIZE);
    load_file_into_memory("programs/6502_65C02_functional_tests/bin_files/6502_functional_test.bin", 0);
    m6502_init(&cpu);
    cpu.read_byte = &rb;
    cpu.write_byte = &wb;
    cpu.pc = 0x400; // pc must be set manually for this test

    nb_instructions_executed = 0;
    u16 previous_pc = 0;
    while (true) {
        m6502_step(&cpu);

        nb_instructions_executed += 1;

        // if the program counter is blocked somewhere, print and exit program
        if (previous_pc == cpu.pc) {
            if (cpu.pc == 0x3469) {
                printf("SUCCESS");
                break;
            }
            printf("FAIL (blocked at 0x%04X)", cpu.pc);
            break;
        }
        previous_pc = cpu.pc;
    }
    printf(" (executed %d opcodes)\n", nb_instructions_executed);

    free(memory);

    return 0;
}
