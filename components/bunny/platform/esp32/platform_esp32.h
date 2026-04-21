#pragma once
/* Platform abstraction – ESP32 implementation */
void bunny_platform_init(void);
void bunny_platform_gpio_write(int pin, int level);
