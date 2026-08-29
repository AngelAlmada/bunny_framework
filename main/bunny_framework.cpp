#include "bunny.h"
#include "display.h"

extern "C" void app_main(void)
{
    bunny_begin();

    /* Initialize display hardware */
    bunny::display::init_hardware();

    /* Load and initialize all auto-registered capabilities */
    bunny_load_modules();

    /* Enter main runtime loop and network handlers */
    bunny_loop();
}

