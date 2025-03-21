#pragma once

#include "common.h"

#define MAX_PROCS 8
#define PROC_UNUSED 0
#define PROC_RUNNABLE 1

struct process {
    int pid;
    int state;
    vaddr_t sp; // virtual stack pointer
    uint8_t stack[8192]; // 8KiB virtual stack
};