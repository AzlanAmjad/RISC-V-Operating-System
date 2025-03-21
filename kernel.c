typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef uint32_t size_t;

extern char __bss[], __bss_end[], __stack_top[];

void *memset(void *buf, char value, size_t space) {
    uint8_t *p = (uint8_t *) buf;
    // set memory to value until space is filled
    while (space--) {
        *p++ = value;
    }
    return buf;
}

void main(void) {
    // set bss to 0 just in case bootloader didn't do it
    memset(__bss, 0, (size_t) __bss_end - (size_t) __bss);

    for (;;);
}

__attribute__((section(".text.boot")))
__attribute__((naked))
void _start(void) { // placed at address 0x80200000 (OpenSBI jumps here on boot)
    __asm__ __volatile__(
        // set address of the stack pointer
        "mv sp, %[stack_top]\n"
        // jump to main function
        "j main\n"
        :
        // pass in the stack top address
        : [stack_top] "r" (__stack_top)
    );
}
