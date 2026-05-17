#include "platform_mock.h"
#include <stdio.h>

void bunny_mock_platform_init(void)
{
    printf("[MOCK] Platform initialized\n");
}

void bunny_mock_gpio_write(int pin, int level)
{
    printf("[MOCK] Pin %d -> %s\n", pin, level ? "HIGH" : "LOW");
}
