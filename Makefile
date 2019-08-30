bin = m6502_tests
src = $(wildcard *.c)
obj = $(src:.c=.o)

CFLAGS = -g -Wall -Wextra -O2 -std=c99 -pedantic
LDFLAGS =

.PHONY: all clean

all: $(bin)

$(bin): $(obj)
	$(CC) -o $@ $^ $(LDFLAGS)

# ehbasic: m6502.o ehbasic_interpreter.o
# 	$(CC) $(CFLAGS) -o ehbasic m6502.o ehbasic_interpreter.o

clean:
	-rm $(bin) $(obj)
