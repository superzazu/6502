# 6502

A MOS Technology 6502 emulator written in C99. It was made with readability in mind. You can use it easily in your own projects (see m6502_tests.c for an example) just by including m6502.c and m6502.h.

Note that undocumented instructions are not supported, and cycles are counted at instruction level. You can disable decimal mode by setting `enable_bcd` to false.

The emulator currently passes the following tests:

- [x] AllSuiteA.bin
- [x] 6502_functional_test.bin

To run the tests, run `make && ./m6502_tests` (don't forget to clone the repo with its submodules).

## Resources

- [6502 instruction reference](http://www.obelisk.me.uk/6502/reference.html) and [this one](http://www.6502.org/tutorials/6502opcodes.html)
- [info on addressing modes](http://www.obelisk.me.uk/6502/addressing.html)
- [info on status flags](http://wiki.nesdev.com/w/index.php/Status_flags)
- [info on the overflow flag](http://www.6502.org/tutorials/vflag.html)
- [MAME 6502 implementation](https://github.com/mamedev/mame/blob/master/src/devices/cpu/m6502/m6502.h)
- [this thread](https://codegolf.stackexchange.com/questions/12844/emulate-a-mos-6502-cpu) for test roms and info on EhBasic
- [Klaus' test roms](https://github.com/Klaus2m5/6502_65C02_functional_tests)
