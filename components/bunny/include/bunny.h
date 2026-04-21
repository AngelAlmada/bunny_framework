#pragma once

/**
 * bunny.h — C-compatible public entry points for the Bunny Framework.
 *
 * Use this in .c files (e.g. app_main).
 * For capability registration in C++ modules, include <bunny_sdk.h> instead.
 */

#ifdef __cplusplus
extern "C" {
#endif

void bunny_begin(void);
void bunny_load_modules(void);
void bunny_loop(void);

#ifdef __cplusplus
}
#endif
