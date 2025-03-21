#include "kernel.h"
#include "common.h"

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef uint32_t size_t;

extern char __bss[], __bss_end[], __stack_top[];

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

void *memset(void *buf, char value, size_t space)
{
    uint8_t *p = (uint8_t *)buf;
    // set memory to value until space is filled
    while (space--)
    {
        *p++ = value;
    }
    return buf;
}

void main(void)
{
    // set bss to 0 just in case bootloader didn't do it
    memset(__bss, 0, (size_t)__bss_end - (size_t)__bss);

    // hello world
    printf("\n\nHello %s\n", "Azlan!");
    printf("1 + 2 = %d, %x\n", 1 + 2, 100);

    for (;;)
    {
        __asm__ __volatile__("wfi");
    }
}

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
