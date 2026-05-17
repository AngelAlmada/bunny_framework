# Reporte de Bug: Persistencia de Sesion WebSocket entre Motor y Bunny

Documento dirigido al desarrollador del motor de procesos para analizar el bug actual de persistencia de sesión WebSocket observado durante handshake + heartbeat exitosos seguidos de desconexión cíclica.

Fecha: 28 de abril de 2026
Estado: Abierto
Prioridad: Alta

---

## 1. Resumen ejecutivo

El framework Bunny actualmente ya cumple estas condiciones:

- acepta `handshake_init`,
- responde `handshake_ack`,
- acepta heartbeat de aplicación,
- responde `heartbeat_ack` o `heartbeat_pong`,
- reanuda discovery UDP automáticamente cuando la sesión se pierde,
- acepta reconexión del motor sin intervención manual.

Sin embargo, la sesión WebSocket sigue cayéndose de forma cíclica después de uno o más heartbeats exitosos.

Esto desplaza el foco del problema desde el handshake o el heartbeat del framework hacia la gestión de persistencia del socket del lado motor.

---

## 2. Logs analizados

Logs observados:

```text
W (152583) httpd_ws: httpd_ws_get_frame_type: Failed to read header byte (socket FD invalid), closing socket now
W (152583) httpd_txrx: httpd_sock_err: error in recv : 128
W (152583) httpd_ws: httpd_ws_recv_frame_internal: WS frame is not properly masked.
W (152593) bunny_network: Resetting active session: websocket read length failed
E (152603) bunny_network: WebSocket read length failed: ESP_ERR_INVALID_STATE
I (152813) bunny_discovery: FLOW[1/4] UDP announce sent #1 -> 255.255.255.255:5555
I (152813) bunny_discovery: UDP discovery started (real broadcast mode)
I (152813) bunny: FLOW[3/4] UDP discovery resumed after WebSocket disconnect
I (152833) bunny: Heartbeat: wifi=connected announces=2 websocket=waiting
I (152833) bunny_network: BCP handshake accepted (engine_id=engine-main-01 protocol_version=0.1.0)
I (152883) bunny_network: Heartbeat acknowledged (engine=engine-main-01)
I (155813) bunny_discovery: FLOW[1/4] UDP announce sent #2 -> 255.255.255.255:5555
I (157843) bunny: FLOW[4/4] WebSocket connection established (real)
I (157843) bunny: Bunny ready to receive commands and publish events
I (157843) bunny_discovery: UDP discovery stopped
I (157843) bunny: FLOW[4/4] UDP discovery stopped after WebSocket connection
I (157853) bunny: Heartbeat: wifi=connected announces=2 websocket=connected
I (162853) bunny: Heartbeat: wifi=connected announces=2 websocket=connected
I (163033) bunny_network: Heartbeat acknowledged (engine=engine-main-01)
W (167533) httpd_ws: httpd_ws_get_frame_type: Failed to read header byte (socket FD invalid), closing socket now
W (167533) httpd_txrx: httpd_sock_err: error in recv : 128
W (167543) httpd_ws: httpd_ws_recv_frame_internal: WS frame is not properly masked.
W (167543) bunny_network: Resetting active session: websocket read length failed
E (167553) bunny_network: WebSocket read length failed: ESP_ERR_INVALID_STATE
I (167853) bunny_discovery: FLOW[1/4] UDP announce sent #1 -> 255.255.255.255:5555
I (167853) bunny_discovery: UDP discovery started (real broadcast mode)
I (167853) bunny: FLOW[3/4] UDP discovery resumed after WebSocket disconnect
I (167873) bunny: Heartbeat: wifi=connected announces=2 websocket=waiting
I (167883) bunny_network: BCP handshake accepted (engine_id=engine-main-01 protocol_version=0.1.0)
I (167933) bunny_network: Heartbeat acknowledged (engine=engine-main-01)
I (170853) bunny_discovery: FLOW[1/4] UDP announce sent #2 -> 255.255.255.255:5555
I (172873) bunny: FLOW[4/4] WebSocket connection established (real)
I (172873) bunny: Bunny ready to receive commands and publish events
I (172873) bunny_discovery: UDP discovery stopped
I (172873) bunny: FLOW[4/4] UDP discovery stopped after WebSocket connection
I (172883) bunny: Heartbeat: wifi=connected announces=2 websocket=connected
```

---

## 3. Lectura técnica del patrón

### 3.1 Qué sí funciona

La evidencia confirma que el contrato BCP básico está operativo:

- `handshake_init` llega correctamente al ESP32.
- Bunny responde `handshake_ack`.
- El motor logra establecer sesión activa.
- El motor envía heartbeat.
- Bunny responde heartbeat.
- Si la sesión cae, Bunny reanuda UDP discovery y permite reconexión.

### 3.2 Qué falla

La sesión no se mantiene estable a lo largo del tiempo.

Patrón repetido observado:

1. Sesión activa.
2. Heartbeat exitoso.
3. Socket inválido o cerrado.
4. Error interno de lectura WS en ESP32.
5. Reset de sesión.
6. Reanudación de discovery.
7. Reconexión y nuevo handshake.
8. Repetición del ciclo.

### 3.3 Interpretación del orden de errores

Orden exacto observado:

1. `Failed to read header byte (socket FD invalid)`
2. `error in recv : 128`
3. `WS frame is not properly masked`
4. `Resetting active session`
5. `WebSocket read length failed: ESP_ERR_INVALID_STATE`

Interpretación:

- El primer síntoma es que el descriptor de socket ya no es válido al momento de leer.
- El warning de máscara aparece después y no debe asumirse automáticamente como causa raíz principal.
- En este patrón, la evidencia sugiere que el socket ya entró en un estado inválido o cerrado antes de completar la lectura normal del frame.

---

## 4. Hipótesis más probable

La causa más probable está en la gestión de sesión del lado motor, no en el contrato BCP del framework.

Posibles escenarios:

- El motor está cerrando el socket después del heartbeat aunque haya recibido respuesta válida.
- La librería WebSocket del motor mezcla heartbeat de aplicación con ping/pong o close frames de forma conflictiva.
- Existe una lógica de timeout local del motor que dispara cierre prematuro.
- El motor reabre o reemplaza sockets sin cerrar correctamente el anterior.
- Algún frame de control enviado por el cliente queda malformado o termina llegando sobre un socket inválido.

---

## 5. Qué ya no parece ser el problema

Con base en la implementación actual del framework y en los logs observados, esto ya no parece ser la causa principal:

- falta de `handshake_ack`,
- falta de respuesta a `heartbeat`,
- falta de rediscovery tras caída,
- imposibilidad de reconectar automáticamente.

---

## 6. Comportamiento actual del framework (confirmado)

El framework Bunny implementa actualmente:

### 6.1 Handshake

- `handshake_init` como primer mensaje de aplicación.
- `handshake_ack` si el payload es válido.
- `handshake_error` si el payload es inválido.

### 6.2 Heartbeat

Mensajes aceptados por Bunny:

Formato simple:

```json
{
  "type": "heartbeat",
  "timestamp": "2026-04-28T21:36:45.354Z"
}
```

Respuesta:

```json
{
  "type": "heartbeat_ack",
  "status": "ok",
  "timestamp": "2026-04-28T21:36:45.354Z"
}
```

Formato correlacionado:

```json
{
  "type": "heartbeat_ping",
  "correlation_id": "hb-0001",
  "ts": "2026-04-28T21:36:45.354Z"
}
```

Respuesta:

```json
{
  "type": "heartbeat_pong",
  "correlation_id": "hb-0001",
  "ts": "2026-04-28T21:36:45.354Z"
}
```

### 6.3 Recuperación automática

Cuando se pierde WebSocket:

- Bunny resetea sesión activa.
- Bunny reanuda discovery UDP.
- Bunny permite nuevo handshake y nueva sesión.

---

## 7. Qué debe revisar el desarrollador del motor

### 7.1 Lifecycle del socket

Verificar explícitamente:

- cuándo se abre el socket,
- cuándo se marca `connected`,
- cuándo se marca `closing`,
- cuándo se invoca `close`,
- cuándo se destruye o reemplaza la instancia cliente.

Pregunta crítica:

- ¿Existe alguna ruta de código que cierre el socket aunque se haya recibido `heartbeat_ack` o `heartbeat_pong`?

### 7.2 Timeouts internos

Verificar:

- timeout de handshake,
- timeout de heartbeat,
- timeout de ping/pong de librería WebSocket,
- timeout global de inactividad,
- reconexión agresiva que invalida socket previo.

Pregunta crítica:

- ¿Se dispara un timeout local porque el motor no está asociando correctamente la respuesta heartbeat con su request enviado?

### 7.3 Interacción entre heartbeat BCP y ping/pong WebSocket

Revisar si el motor está usando simultáneamente:

- heartbeat BCP JSON,
- ping/pong nativo de la librería WebSocket,
- cierre por watchdog propio.

Pregunta crítica:

- ¿Existe doble mecanismo de liveness compitiendo entre sí y cerrando el socket por criterios diferentes?

### 7.4 Manejo de frames de control

Revisar:

- `close frame`,
- `ping frame`,
- `pong frame`,
- reconexión sobre socket previo no completamente cerrado.

Pregunta crítica:

- ¿Se está enviando algún frame de control inmediatamente antes de la caída que deje al socket en estado inválido?

### 7.5 Concurrencia

Bunny solo permite un motor activo por ESP32.

Revisar:

- que no haya múltiples instancias del cliente para el mismo `device.id`,
- que no exista doble reconexión en paralelo,
- que no se reemplace una sesión mientras otra todavía está viva.

---

## 8. Checklist de depuración para el motor

### Checklist mínimo

- [ ] Loggear cada `on_open`, `on_message`, `on_close`, `on_error`.
- [ ] Loggear cada envío de `handshake_init`.
- [ ] Loggear cada recepción de `handshake_ack`.
- [ ] Loggear cada envío de `heartbeat` o `heartbeat_ping`.
- [ ] Loggear cada recepción de `heartbeat_ack` o `heartbeat_pong`.
- [ ] Loggear exactamente qué condición dispara `socket.close()`.
- [ ] Loggear si existe timeout aunque ya haya llegado respuesta heartbeat.
- [ ] Confirmar que no hay múltiples conexiones al mismo `device.id`.
- [ ] Confirmar que no hay mezcla conflictiva entre heartbeat BCP y ping/pong interno.

### Señales que deberían agregarse a logs del motor

- `heartbeat sent: correlation_id=...`
- `heartbeat response received: correlation_id=...`
- `heartbeat timeout fired: correlation_id=...`
- `socket close requested by watchdog`
- `socket close requested by reconnect manager`
- `socket close requested by ws library`
- `new socket instance created for device.id=...`

---

## 9. Preguntas concretas para resolver el bug

1. ¿El motor recibe `heartbeat_ack`/`heartbeat_pong` y aun así dispara timeout?
2. ¿El motor está correlacionando correctamente las respuestas heartbeat?
3. ¿Se está usando además un `ping_interval`/`ping_timeout` de la librería WebSocket que cierre la sesión por su cuenta?
4. ¿El motor cierra el socket cuando cambia el estado persistido del dispositivo o cuando falla la capa BD?
5. ¿Existe una ruta de reconexión que invalide el socket actual antes de completar cierre ordenado?

---

## 10. Conclusión

Con la evidencia actual:

- el handshake funciona,
- el heartbeat del framework funciona,
- la recuperación automática del framework funciona,
- el bug persistente parece estar en la gestión de ciclo de vida del socket del lado motor.

El siguiente paso correcto no es cambiar el contrato BCP, sino instrumentar mejor el motor para identificar exactamente quién dispara el cierre del socket y bajo qué condición temporal.

---

## 11. Referencias

- Especificación BCP: [BCP_SPECIFICATION.md](BCP_SPECIFICATION.md)
- Guía WebSocket del motor: [PROCESS_ENGINE_WEBSOCKET_GUIDE.md](PROCESS_ENGINE_WEBSOCKET_GUIDE.md)
- Guía de handshake: [HANDSHAKE_IMPLEMENTATION_GUIDE.md](HANDSHAKE_IMPLEMENTATION_GUIDE.md)
- Guía de heartbeat: [HEARTBEAT_IMPLEMENTATION_GUIDE.md](HEARTBEAT_IMPLEMENTATION_GUIDE.md)
- Implementación del servidor: [../components/bunny/network/network.c](../components/bunny/network/network.c)
- Recuperación de discovery: [../components/bunny/bunny_sdk.cpp](../components/bunny/bunny_sdk.cpp)
