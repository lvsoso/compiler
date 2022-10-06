#! /bin/bash

RISCV=$HOME/RISCV/bin/riscv64/
RISCV_BIN=$RISCV/bin/
RISCV_ELF=$RISCV/riscv64-unknown-elf/
LLVM_BIN=$HOME/RISCV/bin/llvm/bin/
QEMU_BIN=$HOME/RISCV/bin/qemu/bin/
PATH=$PATH:$RISCV:$RISCV_BIN:$RISCV_ELF:$LLVM_BIN:$QEMU_BIN
