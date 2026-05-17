#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Bunny Protocol – JSON envelope parsing, dispatch, and response serialization. */
void  bunny_protocol_init(void);

/*
 * Parse and dispatch an incoming COMMAND/REQUEST envelope.
 * Returns a heap-allocated JSON envelope (RESPONSE/ERROR) or NULL when no reply is needed.
 * Caller must free the returned buffer with bunny_protocol_free_message().
 */
char* bunny_protocol_handle_incoming(const char* incoming_json);

/* Release heap memory returned by bunny_protocol_handle_incoming(). */
void  bunny_protocol_free_message(char* message_json);

/* Emit an autonomous EVENT envelope through the active websocket session. */
bool  bunny_protocol_emit_event(const char* event_name, const char* data_json);

#ifdef __cplusplus
}
#endif
