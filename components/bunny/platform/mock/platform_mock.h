#pragma once
/* Platform abstraction – Mock implementation for development/testing */
void bunny_mock_platform_init(void);
void bunny_mock_gpio_write(int pin, int level);
