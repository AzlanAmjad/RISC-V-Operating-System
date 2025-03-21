#pragma once

// macro so __FILE__ and __LINE__ are defined properly
#define PANIC(fmt, ...)                                                                                \
    do                                                                                                 \
    {                                                                                                  \
        printf("PANIC: %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__ /* compiler extension */); \
        while (1)                                                                                      \
        {                                                                                              \
        }                                                                                              \
    } while (0)

struct sbiret
{
    long error;
    long value;
};