#include "motores_command.h"
#include "bunny_sdk.h"

void register_motores_command() {
    Bunny.command("motores")
         .description("Auto-generated motores command")
         .execute([](const bunny::Params& p) {
             // TODO: Implement motores logic
         });
}
