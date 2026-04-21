#include "utils.h"
#include <stdio.h>

void bunny_utils_log(const char *tag, const char *message)
{
    printf("[%s] %s\n", tag, message);
}
