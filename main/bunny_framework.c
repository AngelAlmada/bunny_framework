#include "bunny.h"

void app_main(void)
{
    bunny_begin();
    bunny_load_modules();
    bunny_loop();
}
