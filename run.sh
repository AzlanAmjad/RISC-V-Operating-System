#!/bin/bash
set -xue

# path to QEMU
QEMU=qemu-system-riscv32
# path to clang and compiler flags
CC=clang
CFLAGS="-std=c11 -O2 -g3 -Wall -Wextra --target=riscv32-unknown-elf -fno-stack-protector -ffreestanding -nostdlib"

# compile kernel as kernel.elf
$CC $CFLAGS -Wl,-Tsrc/kernel.ld -Wl,-Map=kernel.map -o kernel.elf \
    src/kernel.c src/common.c

# run QEMU
$QEMU -machine virt -bios default -nographic -serial mon:stdio --no-reboot \
    -kernel kernel.elf