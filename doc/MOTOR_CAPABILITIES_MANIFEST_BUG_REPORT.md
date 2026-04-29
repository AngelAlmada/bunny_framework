# Reporte de Bug: Cierre de sesion del motor al recibir `capabilities_manifest`

Documento dirigido al desarrollador del motor de procesos para diagnosticar el cierre de la sesión WebSocket después del handshake y durante el intercambio del manifiesto de capacidades.

Fecha: 28 de abril de 2026
Estado: Abierto
Prioridad: Alta

---

## 1. Resumen ejecutivo

El framework Bunny ya implementa correctamente:

- handshake de aplicación (`handshake_init` -> `handshake_ack`),
- heartbeat (`heartbeat`/`heartbeat_ping` -> `heartbeat_ack`/`heartbeat_pong`),
- exposición automática de capacidades registradas mediante `capabilities_manifest`,
- refresh explícito del manifiesto mediante `capabilities_request`.

El bug actual observado es:

- el motor cierra o resetea la sesión WebSocket cuando Bunny envía `capabilities_manifest`.

Síntoma visible del lado ESP32:

- `error in send : 104`
- `Failed to send WS payload`
- `Failed to send WS header`
- `error in recv : 104`

Interpretación técnica:

- `104` es `ECONNRESET`, es decir, el peer cerró o reseteó la conexión.
- El framework intenta enviar el manifiesto y detecta que el socket ya fue invalidado del lado motor.

---

## 2. Logs observados

```text
I (42434) bunny_network: BCP handshake accepted (engine_id=engine-main-01 protocol_version=0.1.0)
I (42434) bunny_network: Capabilities manifest sent (788 bytes)
W (42484) httpd_txrx: httpd_sock_err: error in send : 104
W (42484) httpd_ws: httpd_ws_send_frame_async: Failed to send WS payload
W (42484) bunny_network: Resetting active session: post-handshake message handling failed

I (43634) bunny_network: BCP handshake accepted (engine_id=engine-main-01 protocol_version=0.1.0)
I (43634) bunny_network: Capabilities manifest sent (788 bytes)
I (43684) bunny_network: Heartbeat acknowledged (engine=engine-main-01)
W (43684) httpd_txrx: httpd_sock_err: error in send : 104
W (43684) httpd_ws: httpd_ws_send_frame_async: Failed to send WS header
W (43684) bunny_network: Resetting active session: post-handshake message handling failed

I (44864) bunny_network: BCP handshake accepted (engine_id=engine-main-01 protocol_version=0.1.0)
I (44864) bunny_network: Capabilities manifest sent (788 bytes)
I (44914) bunny_network: Heartbeat acknowledged (engine=engine-main-01)
I (44924) bunny_network: Capabilities manifest sent (788 bytes)
W (44924) httpd_txrx: httpd_sock_err: error in recv : 104
W (44924) httpd_txrx: httpd_sock_err: error in recv : 104
W (44924) httpd_ws: httpd_ws_recv_frame_internal: WS frame is not properly masked.
W (44934) bunny_network: Resetting active session: websocket read length failed
```

---

## 3. Qué indica este patrón

### 3.1 Qué sí está funcionando

- El motor logra abrir la conexión WebSocket.
- El motor envía `handshake_init`.
- Bunny responde `handshake_ack`.
- Bunny envía el manifiesto de capacidades.
- En algunos ciclos también hay heartbeat exitoso.

### 3.2 Qué no está funcionando

El motor no mantiene la sesión estable cuando aparece `capabilities_manifest` en el canal.

### 3.3 Qué significa `error 104`

`error in send : 104` o `error in recv : 104` significa que el peer (el motor) cerró o reseteó la conexión.

En otras palabras:

- Bunny no está iniciando ese cierre.
- Bunny descubre el cierre cuando intenta enviar o recibir.

---

## 4. Hipótesis principal

La hipótesis más fuerte es que el motor no está manejando correctamente el mensaje `capabilities_manifest`.

Posibles causas concretas:

1. El motor no reconoce `type = capabilities_manifest` y cierra la sesión por mensaje inesperado.
2. El parser del motor falla al procesar el objeto `capabilities`.
3. La state machine del motor espera solo `handshake_ack` y no tolera un mensaje adicional automático justo después.
4. El motor envía `capabilities_request` pero falla al parsear la respuesta.
5. El motor trata mensajes espontáneos del servidor como error de protocolo y cierra el socket.

---

## 5. Contrato actual que Bunny implementa

### 5.1 Envío automático tras handshake

Después de `handshake_ack`, Bunny envía automáticamente:

```json
{
  "type": "capabilities_manifest",
  "status": "ok",
  "device_id": "esp32-001",
  "protocol_version": "0.1.0",
  "capabilities": {
    "sensors": [],
    "commands": [],
    "events": [],
    "states": []
  }
}
```

### 5.2 Solicitud explícita desde motor

El motor puede solicitar refresh enviando:

```json
{
  "type": "capabilities_request"
}
```

Y Bunny responde nuevamente con `capabilities_manifest`.

---

## 6. Qué debe revisar el desarrollador del motor

### 6.1 Parser de mensajes post-handshake

Confirmar que el motor soporte explícitamente estos tipos:

- `handshake_ack`
- `handshake_error`
- `heartbeat_ack`
- `heartbeat_pong`
- `capabilities_manifest`

Pregunta crítica:

- ¿El motor tiene una rama explícita para `capabilities_manifest` o cae en "unknown message type"?

### 6.2 State machine del canal

Revisar si el motor asume este flujo erróneo:

- `handshake_ack`
- luego solo heartbeats o respuestas request/response

Cuando en realidad Bunny ahora también puede enviar espontáneamente:

- `capabilities_manifest`

Pregunta crítica:

- ¿La state machine del motor tolera mensajes server-push inmediatamente después del handshake?

### 6.3 Manejo del manifiesto

Confirmar que el parser soporte la estructura:

- `type`
- `status`
- `device_id`
- `protocol_version`
- `capabilities`

Y dentro de `capabilities`:

- `sensors`
- `commands`
- `events`
- `states`

Pregunta crítica:

- ¿El parser espera solo arrays planos o falla con el objeto agrupado por categoría?

### 6.4 Cierre de socket

Instrumentar el motor para responder estas preguntas:

- ¿Quién llama `socket.close()`?
- ¿Qué condición lo dispara?
- ¿Se cierra al parsear mensaje desconocido?
- ¿Se cierra al fallar el mapping del manifiesto a estructuras internas?

---

## 7. Checklist de depuración para el motor

- [ ] Loggear payload completo de cada mensaje recibido desde Bunny.
- [ ] Confirmar recepción de `capabilities_manifest` antes del cierre.
- [ ] Loggear tipo de mensaje detectado por parser.
- [ ] Loggear excepción o rama de error si el parser falla.
- [ ] Loggear condición exacta que dispara `close()` o `terminate()` del socket.
- [ ] Confirmar si se envía `capabilities_request` desde el motor.
- [ ] Confirmar si el cierre ocurre tras el envío automático o tras el refresh solicitado.
- [ ] Validar que el manifiesto se acepta como mensaje legítimo del protocolo.

---

## 8. Señales recomendadas en logs del motor

Agregar estas trazas al cliente del motor:

- `received message type=capabilities_manifest`
- `capabilities manifest parse ok`
- `capabilities manifest parse failed: <reason>`
- `closing socket due to unexpected message type`
- `closing socket due to parser exception`
- `sending capabilities_request`
- `received capabilities_manifest after request`

---

## 9. Conclusión

Con la evidencia actual:

- Bunny sí está enviando el manifiesto.
- Bunny no está iniciando el cierre.
- El peer (motor) está reseteando la conexión cuando llega o se procesa `capabilities_manifest`.

La siguiente acción correcta del lado motor es instrumentar el parser y la state machine para verificar cómo se maneja ese mensaje y por qué la sesión termina en `ECONNRESET`.

---

## 10. Referencias

- Exposición de capacidades: [CAPABILITIES_EXPOSURE_GUIDE.md](CAPABILITIES_EXPOSURE_GUIDE.md)
- Especificación BCP: [BCP_SPECIFICATION.md](BCP_SPECIFICATION.md)
- Guía WebSocket: [PROCESS_ENGINE_WEBSOCKET_GUIDE.md](PROCESS_ENGINE_WEBSOCKET_GUIDE.md)
- Handshake: [HANDSHAKE_IMPLEMENTATION_GUIDE.md](HANDSHAKE_IMPLEMENTATION_GUIDE.md)
- Heartbeat: [HEARTBEAT_IMPLEMENTATION_GUIDE.md](HEARTBEAT_IMPLEMENTATION_GUIDE.md)
- Implementación firmware: [../components/bunny/network/network.c](../components/bunny/network/network.c)
