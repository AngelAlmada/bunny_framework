# Motor de Procesos — Guía de Conexión WebSocket

Guía específica para implementar correctamente la conexión WebSocket entre el motor de procesos y un dispositivo Bunny.

Última actualización: 23 de abril de 2026.

**← [Volver al README](README.md)**

---

## Objetivo

El motor de procesos debe usar WebSocket como canal de sesión activa con Bunny para:

1. enviar comandos al dispositivo,
2. recibir eventos del dispositivo,
3. mantener una conexión viva y verificable,
4. detectar desconexiones y reconectar sin corromper estado.

---

## 1. Rol de cada lado

En Bunny, el dispositivo ESP32 actúa como:

- servidor HTTP,
- endpoint WebSocket,
- receptor de frames de texto,
- punto de entrada para mensajes del motor.

El motor de procesos actúa como:

- cliente WebSocket,
- iniciador del handshake,
- emisor de frames JSON,
- responsable de reconexión, health-check y manejo de sesión.

Referencia de implementación del servidor: [components/bunny/network/network.c](../components/bunny/network/network.c)

---

## 2. URL de conexión

El motor no debe hardcodear la URL. Debe construirla usando el anuncio UDP del dispositivo:

`ws://<ip>:<webhook_port><webhook_path>`

Ejemplo:

`ws://192.168.100.176:8080/bunny`

Campos usados:

- `ip`
- `webhook_port`
- `webhook_path`

Estos campos vienen del anuncio UDP emitido por Bunny.

---

## 3. Requisitos obligatorios del cliente WebSocket

El motor debe cumplir estas reglas:

1. Debe usar una librería WebSocket real, no un socket TCP crudo.
2. Debe hacer handshake HTTP Upgrade válido.
3. Debe enviar frames correctamente enmascarados.
4. Debe enviar texto UTF-8 si el payload es JSON.
5. Debe tratar al dispositivo como un servidor WebSocket estándar.

Importante:

Un cliente WebSocket válido enmascara sus frames automáticamente. Si el motor implementa el protocolo “a mano” o envía texto plano por TCP, Bunny puede rechazarlo con errores como:

- `WS frame is not properly masked`
- `WebSocket read length failed: ESP_ERR_INVALID_STATE`

---

## 4. Flujo correcto de conexión

Secuencia esperada:

1. El motor descubre el dispositivo por UDP.
2. Construye la URL WebSocket.
3. Abre la conexión como cliente.
4. Espera confirmación de handshake (`on_open`).
5. Marca el dispositivo como `connected`.
6. Comienza a intercambiar mensajes JSON.
7. Mantiene la sesión con ping/pong o heartbeat equivalente.
8. Si falla, marca `disconnected` y reintenta.

---

## 5. Cuándo considerar que la conexión quedó activa

El motor debe considerar conectada una sesión solo cuando ocurra una de estas condiciones equivalentes:

1. La librería WebSocket dispara `on_open`.
2. El estado del socket pasa a `OPEN`.
3. Se completa el handshake HTTP Upgrade sin error.

No considerar conectado solo porque:

- hubo discovery UDP,
- se pudo abrir TCP al puerto,
- el dispositivo respondió HTTP,
- se resolvió la IP.

UDP descubre. WebSocket conecta.

---

## 6. Validación de conexión

La validación debe hacerse en dos niveles.

### Validación primaria

El motor debe exigir:

1. handshake exitoso,
2. socket abierto,
3. sin error de protocolo.

### Validación de continuidad

Después del `connected`, el motor debe:

1. enviar ping periódico o usar el mecanismo interno de la librería,
2. esperar pong,
3. cerrar y reconectar si no llega respuesta en timeout.

### Validación cruzada con logs del dispositivo

Si el handshake se completó correctamente, Bunny registra:

- `WebSocket handshake completed from /bunny`
- `FLOW[4/4] WebSocket connection established (real)`

Si el motor afirma estar conectado pero Bunny no muestra eso, la conexión no está bien establecida del lado del dispositivo.

---

## 7. Manejo de reconexión

El motor debe implementar reconexión con estas reglas:

1. Un solo socket activo por `device.id`.
2. Si cambia la IP del mismo `device.id`, cerrar sesión anterior y reconectar a la nueva URL.
3. Usar backoff progresivo.
4. No abrir conexiones en paralelo al mismo endpoint para “probar suerte”.
5. Limpiar estado de mensajes pendientes al cerrar la sesión.

Backoff recomendado:

1. 1 segundo
2. 2 segundos
3. 5 segundos
4. 10 segundos
5. 30 segundos máximo

---

## 8. Formato de mensajes

Mientras el protocolo final del runtime sigue en evolución, el motor debe asumir lo siguiente:

1. el transporte es WebSocket,
2. el contenido del frame es JSON texto,
3. los mensajes deben ser serializados como string JSON completo.

Ejemplo de comando:

```json
{
  "type": "command",
  "command": "setFanState",
  "params": {
    "state": "ON"
  }
}
```

El motor debe enviar eso como un frame WebSocket de texto, no como bytes sueltos por TCP ni como POST HTTP.

---

## 9. Errores típicos que debe evitar el motor

### Error: usar HTTP en lugar de WebSocket

Incorrecto:

- hacer `GET /bunny` normal esperando que eso “sea suficiente”,
- hacer `POST /bunny` con JSON,
- usar `curl` o `netcat` para enviar comandos.

Correcto:

- usar una librería cliente WebSocket.

### Error: enviar frames sin máscara

Si ocurre esto, ESP-IDF puede registrar:

- `WS frame is not properly masked`

Causa típica:

- implementación manual incorrecta del protocolo,
- cliente no compatible con RFC 6455,
- datos enviados por socket TCP crudo.

### Error: marcar conectado demasiado pronto

Incorrecto:

- marcar `connected=true` apenas llega discovery UDP,
- marcar `connected=true` al abrir TCP.

Correcto:

- marcar `connected=true` solamente en `on_open` o equivalente.

### Error: no manejar cierre del socket

El motor debe tratar `on_close`, timeout, EOF o error como pérdida real de sesión.

---

## 10. Estado mínimo recomendado por dispositivo

El motor debería mantener, como mínimo:

```json
{
  "id": "esp32-001",
  "name": "Mi Dispositivo Bunny",
  "ip": "192.168.100.176",
  "websocket_url": "ws://192.168.100.176:8080/bunny",
  "available": true,
  "connected": false,
  "last_seen_udp": 1713910000,
  "last_ws_open": null,
  "last_ws_pong": null,
  "last_error": null
}
```

Esto permite separar tres cosas:

1. descubierto en red,
2. conectado por sesión,
3. sano operativamente.

---

## 11. Comportamiento recomendado del loop del motor

1. Escuchar anuncios UDP continuamente.
2. Actualizar `last_seen_udp` para cada dispositivo.
3. Intentar WebSocket si el dispositivo está disponible y no conectado.
4. Al abrir, registrar `last_ws_open`.
5. Al recibir pong o mensaje, actualizar actividad.
6. Al cerrar, marcar `connected=false`.
7. Si el dispositivo deja de anunciarse por timeout, marcar `available=false`.

---

## 12. Ejemplo mínimo en Python

```python
import asyncio
import json
import websockets

async def connect_device(ip: str, port: int, path: str):
    uri = f"ws://{ip}:{port}{path}"
    async with websockets.connect(uri, ping_interval=20, ping_timeout=20) as ws:
        print(f"Connected to {uri}")

        payload = {
            "type": "command",
            "command": "setFanState",
            "params": {"state": "ON"}
        }
        await ws.send(json.dumps(payload))

        async for message in ws:
            print("RX:", message)

asyncio.run(connect_device("192.168.100.176", 8080, "/bunny"))
```

Ese ejemplo sí usa una librería WebSocket real y evita los errores típicos de framing.

---

## 13. Estado actual del framework

Hoy Bunny ya tiene implementado:

1. servidor WebSocket,
2. handshake WebSocket,
3. recepción de frames de texto,
4. logging de conexión en monitor.

Todavía está evolucionando:

1. el dispatcher final de protocolo,
2. el manejo completo de respuestas de runtime,
3. la actualización fina del estado de conexión al desconectar.

Por eso, el motor debe diseñarse para:

1. validar la conexión por handshake y salud del socket,
2. no asumir todavía semántica final de todos los mensajes,
3. tolerar que el transporte ya exista aunque el protocolo de ejecución siga en construcción.

---

## 14. Checklist final para el motor

1. Usar WebSocket cliente real, no TCP raw.
2. Construir la URL desde discovery UDP.
3. Marcar conectado solo al completar handshake.
4. Enviar JSON como frame texto.
5. Mantener ping/pong o health-check equivalente.
6. Manejar cierre, timeout y reconexión.
7. Evitar múltiples sesiones para el mismo dispositivo.
8. Registrar errores de protocolo y framing para diagnóstico.

Referencias útiles:

- [PROCESS_ENGINE_CONNECTION_GUIDE.md](PROCESS_ENGINE_CONNECTION_GUIDE.md)
- [MOTOR_INTEGRATION.md](MOTOR_INTEGRATION.md)
- [NETWORK_OVERVIEW.md](NETWORK_OVERVIEW.md)
- [components/bunny/network/network.c](../components/bunny/network/network.c)
