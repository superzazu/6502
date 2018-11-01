CC = cc
CFLAGS = -g -Wall -Wextra -O3 -std=c11 -pedantic

.PHONY: clean

default: m6502_tests

m6502_tests: m6502.o m6502_tests.o
	$(CC) $(CFLAGS) -o m6502_tests m6502.o m6502_tests.o

# ehbasic: m6502.o ehbasic_interpreter.o
# 	$(CC) $(CFLAGS) -o ehbasic m6502.o ehbasic_interpreter.o

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f m6502_tests ehbasic *.o
