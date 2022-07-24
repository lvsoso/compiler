#!/bin/bash

RISCV=$HOME/RISCV/bin/riscv64/
RISCV_BIN=$RISCV/bin/
RISCV_ELF=$RISCV/riscv64-unknown-elf/
LLVM_BIN=$HOME/RISCV/bin/llvm/bin/
QEMU_BIN=$HOME/RISCV/bin/qemu/bin/
PATH=$PATH:$RISCV:$RISCV_BIN:$RISCV_ELF:$LLVM_BIN:$QEMU_BIN

assert(){
    expected="$1"
    input="$2"

    ./rvcc "$input" > tmp.s || exit
    # clang-12 -o tmp tmp.s
    riscv64-unknown-elf-gcc -static -o tmp tmp.s
    
    # ./tmp
    # qemu-riscv64 -L $RISCV/sysroot ./tmp
    spike --isa=rv64gc $RISCV/riscv64-unknown-elf/bin/pk ./tmp


    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
  fi
}

# assert expect got
# [1] 返回指定数
assert 0 0
assert 42 42

# [2] 支持+ -运算符
assert 34 '12-34+56'

echo OK