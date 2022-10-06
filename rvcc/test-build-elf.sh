#!/bin/bash

RISCV=$HOME/RISCV/bin/riscv64/
RISCV_BIN=$RISCV/bin/
RISCV_ELF=$RISCV/riscv64-unknown-elf/
LLVM_BIN=$HOME/RISCV/bin/llvm/bin/
QEMU_BIN=$HOME/RISCV/bin/qemu/bin/
PATH=$PATH:$RISCV:$RISCV_BIN:$RISCV_ELF:$LLVM_BIN:$QEMU_BIN



if [ "$1" = "common" ]; then
    echo "skip common"
else
    echo "$1"
    #	$(CC) -o- -E -P -C test/$*.c | ./rvcc -o test/$*.s -
    $RISCV/bin/riscv64-unknown-elf-gcc  -E -P -C test/$1.c | ./rvcc -o test/$1.s -
    #	$(CC) -static -o $@ test/$*.s -xc test/common
    $RISCV/bin/riscv64-unknown-elf-gcc -static -o test/$1.elf test/$1.s -xc test/common.c
fi

# $RISCV/bin/riscv64-unknown-elf-gcc -static -o arith.elf test/arith.s -xc test/common.c
# $RISCV/bin/riscv64-unknown-elf-gcc -o- -E -P -C test/arith.c | ./rvcc -o test/arith.s -
