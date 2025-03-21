#include "kernel.h"
#include "common.h"
#include "process.h"

extern char __bss[], __bss_end[], __stack_top[], __free_ram[], __free_ram_end[];

// creating processes
struct process procs[MAX_PROCS];
struct process *current_proc; // Currently running process
struct process *idle_proc;    // Idle process

// context switching
__attribute__((naked)) void switch_context(uint32_t *prev_sp, uint32_t *new_sp)
{
    __asm__ __volatile__(
        // store callee-saved registers (sx registers)
        "addi sp, sp, -13 * 4\n" // space for 13 4-byte registers
        "sw ra,  0  * 4(sp)\n"
        "sw s0,  1  * 4(sp)\n"
        "sw s1,  2  * 4(sp)\n"
        "sw s2,  3  * 4(sp)\n"
        "sw s3,  4  * 4(sp)\n"
        "sw s4,  5  * 4(sp)\n"
        "sw s5,  6  * 4(sp)\n"
        "sw s6,  7  * 4(sp)\n"
        "sw s7,  8  * 4(sp)\n"
        "sw s8,  9  * 4(sp)\n"
        "sw s9,  10 * 4(sp)\n"
        "sw s10, 11 * 4(sp)\n"
        "sw s11, 12 * 4(sp)\n"

        // context switch (switch stack pointer to new sp)
        "sw sp, (a0)\n"
        "lw sp, (a1)\n"

        // restore callee-saved registers from the new processes stack
        // this resets the context to whatever the new process has set up
        "lw ra,  0  * 4(sp)\n"
        "lw s0,  1  * 4(sp)\n"
        "lw s1,  2  * 4(sp)\n"
        "lw s2,  3  * 4(sp)\n"
        "lw s3,  4  * 4(sp)\n"
        "lw s4,  5  * 4(sp)\n"
        "lw s5,  6  * 4(sp)\n"
        "lw s6,  7  * 4(sp)\n"
        "lw s7,  8  * 4(sp)\n"
        "lw s8,  9  * 4(sp)\n"
        "lw s9,  10 * 4(sp)\n"
        "lw s10, 11 * 4(sp)\n"
        "lw s11, 12 * 4(sp)\n"
        "addi sp, sp, 13 * 4\n" // We've popped 13 4-byte registers from the stack
        "ret\n");
}

void yield(void)
{
    // Search for a runnable process
    struct process *next = idle_proc;
    for (int i = 0; i < MAX_PROCS; i++)
    {
        struct process *proc = &procs[(current_proc->pid + i) % MAX_PROCS];
        if (proc->state == PROC_RUNNABLE && proc->pid > 0)
        {
            next = proc;
            break;
        }
    }

    // If there's no runnable process other than the current one, return and continue processing
    if (next == current_proc)
        return;

    // store the address at the bottom of the proc stack in sscratch
    // this is used by the exception handler if one occurs
    __asm__ __volatile__(
        "csrw sscratch, %[sscratch]\n"
        :
        : [sscratch] "r" ((uint32_t) &next->stack[sizeof(next->stack)])
    );

    // Context switch
    struct process *prev = current_proc;
    current_proc = next;
    switch_context(&prev->sp, &next->sp);
}

struct process *create_process(uint32_t pc)
{
    struct process *proc = NULL;
    int i;
    for (i = 0; i < MAX_PROCS; i++)
    {
        if (procs[i].state == PROC_UNUSED)
        {
            proc = &procs[i];
            break;
        }
    }
    if (!proc)
    {
        PANIC("no free processes");
    }

    // set callee-saved registers on proc stack
    uint32_t *sp = (uint32_t *)&proc->stack[sizeof(proc->stack)];
    *--sp = 0;            // s11
    *--sp = 0;            // s10
    *--sp = 0;            // s9
    *--sp = 0;            // s8
    *--sp = 0;            // s7
    *--sp = 0;            // s6
    *--sp = 0;            // s5
    *--sp = 0;            // s4
    *--sp = 0;            // s3
    *--sp = 0;            // s2
    *--sp = 0;            // s1
    *--sp = 0;            // s0
    *--sp = (uint32_t)pc; // ra, context switch will return to this

    // Initialize fields.
    proc->pid = i + 1;
    proc->state = PROC_RUNNABLE;
    proc->sp = (uint32_t)sp;
    return proc;
}

// page (4KiB) allocator
// TODO: maybe implement an algorithm that allows deallocation?
paddr_t alloc_pages(uint32_t n)
{
    // start new_addr at free ram
    static paddr_t new_addr = (paddr_t)__free_ram;
    paddr_t start_addr = new_addr;
    // increment based off of allocated memory
    new_addr += n * PAGE_SIZE;
    if (new_addr > (paddr_t)__free_ram_end)
    {
        PANIC("out of memory");
    }
    // set allocated memory to 0 to avoid any problems
    memset((void *)start_addr, 0, n * PAGE_SIZE);
    return (paddr_t)start_addr;
}

// handle exception
void handle_trap(struct trap_frame *f)
{
    // read processor set CSR
    uint32_t scause = READ_CSR(scause);
    uint32_t stval = READ_CSR(stval);
    uint32_t user_pc = READ_CSR(sepc);

    PANIC("unexpected trap scause=%x, stval=%x, sepc=%x\n", scause, stval, user_pc);
}

// exception handler
__attribute__((naked))
__attribute__((aligned(4))) void
kernel_entry(void)
{
    __asm__ __volatile__(
        // save general purpose registers on stack
        "csrrw sp, sscratch, sp\n" // Retrieve the kernel stack of the running process from sscratch

        "addi sp, sp, -4 * 31\n" // grow the stack, modify sp

        // insert current registers onto the stack
        // for exception handling function call
        "sw ra,  4 * 0(sp)\n"
        "sw gp,  4 * 1(sp)\n"
        "sw tp,  4 * 2(sp)\n"
        "sw t0,  4 * 3(sp)\n"
        "sw t1,  4 * 4(sp)\n"
        "sw t2,  4 * 5(sp)\n"
        "sw t3,  4 * 6(sp)\n"
        "sw t4,  4 * 7(sp)\n"
        "sw t5,  4 * 8(sp)\n"
        "sw t6,  4 * 9(sp)\n"
        "sw a0,  4 * 10(sp)\n"
        "sw a1,  4 * 11(sp)\n"
        "sw a2,  4 * 12(sp)\n"
        "sw a3,  4 * 13(sp)\n"
        "sw a4,  4 * 14(sp)\n"
        "sw a5,  4 * 15(sp)\n"
        "sw a6,  4 * 16(sp)\n"
        "sw a7,  4 * 17(sp)\n"
        "sw s0,  4 * 18(sp)\n"
        "sw s1,  4 * 19(sp)\n"
        "sw s2,  4 * 20(sp)\n"
        "sw s3,  4 * 21(sp)\n"
        "sw s4,  4 * 22(sp)\n"
        "sw s5,  4 * 23(sp)\n"
        "sw s6,  4 * 24(sp)\n"
        "sw s7,  4 * 25(sp)\n"
        "sw s8,  4 * 26(sp)\n"
        "sw s9,  4 * 27(sp)\n"
        "sw s10, 4 * 28(sp)\n"
        "sw s11, 4 * 29(sp)\n"

        // store original sp at bottom of frame on stack
        "csrr a0, sscratch\n"
        "sw a0, 4 * 30(sp)\n"

        // Reset the kernel stack.
        "addi a0, sp, 4 * 31\n"
        "csrw sscratch, a0\n"

        // store new sp in a0
        "mv a0, sp\n"
        // call trap handler
        "call handle_trap\n"

        // restore saved execution state (sret instruction)
        "lw ra,  4 * 0(sp)\n"
        "lw gp,  4 * 1(sp)\n"
        "lw tp,  4 * 2(sp)\n"
        "lw t0,  4 * 3(sp)\n"
        "lw t1,  4 * 4(sp)\n"
        "lw t2,  4 * 5(sp)\n"
        "lw t3,  4 * 6(sp)\n"
        "lw t4,  4 * 7(sp)\n"
        "lw t5,  4 * 8(sp)\n"
        "lw t6,  4 * 9(sp)\n"
        "lw a0,  4 * 10(sp)\n"
        "lw a1,  4 * 11(sp)\n"
        "lw a2,  4 * 12(sp)\n"
        "lw a3,  4 * 13(sp)\n"
        "lw a4,  4 * 14(sp)\n"
        "lw a5,  4 * 15(sp)\n"
        "lw a6,  4 * 16(sp)\n"
        "lw a7,  4 * 17(sp)\n"
        "lw s0,  4 * 18(sp)\n"
        "lw s1,  4 * 19(sp)\n"
        "lw s2,  4 * 20(sp)\n"
        "lw s3,  4 * 21(sp)\n"
        "lw s4,  4 * 22(sp)\n"
        "lw s5,  4 * 23(sp)\n"
        "lw s6,  4 * 24(sp)\n"
        "lw s7,  4 * 25(sp)\n"
        "lw s8,  4 * 26(sp)\n"
        "lw s9,  4 * 27(sp)\n"
        "lw s10, 4 * 28(sp)\n"
        "lw s11, 4 * 29(sp)\n"
        "lw sp,  4 * 30(sp)\n"
        "sret\n");
}

// function to interact with the SBI
struct sbiret sbi_call(long arg0, long arg1, long arg2, long arg3, long arg4, long arg5, long fid, long eid)
{
    register long a0 __asm__("a0") = arg0;
    register long a1 __asm__("a1") = arg1;
    register long a2 __asm__("a2") = arg2;
    register long a3 __asm__("a3") = arg3;
    register long a4 __asm__("a4") = arg4;
    register long a5 __asm__("a5") = arg5;
    register long a6 __asm__("a6") = fid; // a6 encodes the SBI function ID (FID)
    register long a7 __asm__("a7") = eid; // a7 encodes the SBI extension ID (EID)

    // call to OpenSBI switches CPU from Supervisor/Kernel Mode to Machine Mode
    __asm__ __volatile__("ecall"
                         : "=r"(a0), "=r"(a1)
                         : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
                           "r"(a6), "r"(a7)
                         : "memory");
    return (struct sbiret){.error = a0, .value = a1};
}

void putchar(char ch)
{
    sbi_call(ch, 0, 0, 0, 0, 0, 0, 1 /* OpenSBI Console Putchar function */);
}

void delay(void)
{
    for (int i = 0; i < 30000000; i++)
        __asm__ __volatile__("nop"); // do nothing
}

struct process *proc_a;
struct process *proc_b;

void proc_a_entry(void)
{
    printf("starting process A\n");
    while (1)
    {
        putchar('A');
        yield();
        for (int i = 0; i < 30; i++)
        {
            delay();
        }
    }
}

void proc_b_entry(void)
{
    printf("starting process B\n");
    while (1)
    {
        putchar('B');
        yield();
        for (int i = 0; i < 30; i++)
        {
            delay();
        }
    }
}

void main(void)
{
    // set bss to 0 just in case bootloader didn't do it
    memset(__bss, 0, (size_t)__bss_end - (size_t)__bss);

    // hello world
    printf("\n\nHello %s\n", "Azlan!");
    printf("1 + 2 = %d, %x\n", 1 + 2, 100);

    // set exception handler
    WRITE_CSR(stvec, (uint32_t)kernel_entry);
    // test: trigger an illegal instruction exception
    //__asm__ __volatile__("unimp");
    // PANIC("testing kernel panic");

    // test: memory allocation
    
    paddr_t paddr0 = alloc_pages(2);
    paddr_t paddr1 = alloc_pages(1);
    printf("alloc_pages test: paddr0=%x\n", paddr0);
    printf("alloc_pages test: paddr1=%x\n", paddr1);
    

    // test: process creation and yield
    
    idle_proc = create_process((uint32_t)NULL);
    idle_proc->pid = 0; // idle
    current_proc = idle_proc;

    proc_a = create_process((uint32_t)proc_a_entry);
    proc_b = create_process((uint32_t)proc_b_entry);

    yield();
    
    for (;;);
    PANIC("switched to idle process");
}


/*
COMPLETED SO FAR

0. I got it to boot?
1. Hello World / printf debugging / C standard library
2. Kernel panic and Exception handling
3. Page memory allocation $llvm-nm kernel.elf | grep __free_ram
4. Process / process scheduling

*/
__attribute__((section(".text.boot")))
__attribute__((naked)) void
_start(void)
{ // placed at address 0x80200000 (OpenSBI jumps here on boot)
    __asm__ __volatile__(
        // set address of the stack pointer
        "mv sp, %[stack_top]\n"
        // jump to main function
        "j main\n"
        :
        // pass in the stack top address
        : [stack_top] "r"(__stack_top));
}
