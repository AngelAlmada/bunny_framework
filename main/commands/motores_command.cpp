#include "bunny.h"

BUNNY_COMMAND(motores) {
    Bunny.command("motores")
         .description("Auto-generated motores command")
         .execute([](const bunny::Params& p) {
             // TODO: Implement motores logic
         });
}

