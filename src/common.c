#include "common.h"

void putchar(char ch);

/* DEBUGGING OPERATIONS */
void printf(const char *fmt, ...)
{
    /*
        In this implementation of printf we implement three format specifiers
        %d (decimal), %x (hexadecimal), %s (string)
    */
    // get list of args
    va_list args;
    va_start(args, fmt);

    while (*fmt)
    {
        if (*fmt == '%') // insert argument
        {
            fmt++; // skip %
            switch (*fmt)
            {
            case '\0':
            {
                // string ends print %
                putchar('%');
                goto end;
            }
            case '%':
            {
                // double %
                putchar('%');
                break;
            }
            case 's':
            {
                // NULL terminated string
                const char *string = va_arg(args, const char *);
                while (*string) // while pointing to something other than NUll terminator
                {
                    putchar(*string);
                    string++;
                }
                break;
            }
            case 'd':
            {
                // decimal number
                int value = va_arg(args, int);
                unsigned mag = value;
                // print '-' if negative number, invert mag
                if (value < 0)
                {
                    putchar('-');
                    mag = -mag;
                }
                /*
                    example getting largest divisor to get mag ot single digit:
                    1. mag = 100, div = 1, mag / div = 100
                    2. mag = 100, div = 10, mag / div = 10
                    3. mag = 100, div = 100, mag / div = 1, stop here
                */
                unsigned divisor = 1;
                while (mag / divisor > 9)
                {
                    divisor *= 10;
                }
                // reduce divisor as we print, so we print left to right
                while (divisor > 0)
                {
                    putchar('0' + mag / divisor);
                    mag %= divisor;
                    divisor /= 10;
                }
                break;
            }
            case 'x':
            {
                // hexadecimal
                unsigned value = va_arg(args, unsigned);
                /*
                    example:
                    1. value = 100, i = 7, nibble = 01100100 >> 7 * 4 & 1111 = 0
                    2. value = 100, i = 6, nibble = 01100100 >> 6 * 4 & 1111 = 0
                    3. value = 100, i = 5, nibble = 01100100 >> 5 * 4 & 1111 = 0
                    4. value = 100, i = 4, nibble = 01100100 >> 4 * 4 & 1111 = 0
                    5. value = 100, i = 3, nibble = 01100100 >> 3 * 4 & 1111 = 0
                    6. value = 100, i = 2, nibble = 01100100 >> 2 * 4 & 1111 = 0
                    7. value = 100, i = 1, nibble = 01100100 >> 1 * 4 & 1111 = 6
                    8. value = 100, i = 0, nibble = 01100100 >> 0 * 4 & 1111 = 4
                    result in hex: 0x00000064 = 32-bit value 4 * 8 = 32-bit
                */
                putchar('0');
                putchar('x');
                for (int i = 7; i >= 0; i--)
                {
                    unsigned nibble = (value >> (i * 4)) & 0xf;
                    putchar("0123456789abcdef"[nibble]);
                }
            }
            }
        }
        else
        {
            putchar(*fmt);
        }

        fmt++;
    }

end:
    va_end(args);
}

/* MEMORY OPERATIONS */
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

void *memcpy(void *dst, const void *src, size_t space)
{
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;
    while (space--)
    {
        *d++ = *s++;
    }
    return dst;
}

char *strcpy(char *dst, const char *src)
{
    char *d = dst;
    // assume src is a NULL terminated string
    while (*src)
    {
        *d++ = *src++;
    }
    // add NULL terminator
    *d = '\0';
    return dst;
}

int strcmp(const char *s1, const char *s2)
{
    /*
        s1 == s2, return 0
        s1 > s2, return positive
        s1 < s2, return negative
    */
    while (*s1 && *s2)
    {
        if (*s1 != *s2)
        {
            break;
        }
        s1++;
        s2++;
    }

    return *(unsigned char *)s1 - *(unsigned char *)s2;
}
