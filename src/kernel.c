typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef uint32_t size_t;

extern char __bss[], __bss_end[], __stack_top[];

__attribute__((section(".text.boot")))
__attribute__((naked))
void _start(void) {}
