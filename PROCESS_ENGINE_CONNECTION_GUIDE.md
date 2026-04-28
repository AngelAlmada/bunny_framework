º# Motor de Procesos — Descubrimiento, Conexión y Validación

Guía práctica para implementar correctamente el lado del motor de procesos al trabajar con Bunny Framework.

Última actualización: 22 de abril de 2026.

**← [Volver al README](README.md)**

---

## Objetivo

El motor de procesos debe cumplir tres mecánicas:

1. Detectar qué dispositivos Bunny están disponibles en la red LAN.
2. Conectarse al endpoint WebSocket correcto de cada dispositivo.
3. Validar de forma confiable que la conexión quedó realmente activa.

---

## 1) Descubrir dispositivos disponibles (UDP)

Bunny publica anuncios UDP periódicos en broadcast usando:

- destino: `255.255.255.255`
- puerto: `discovery.udp_port` (por defecto `5555`)
- intervalo: `discovery.broadcast_interval_ms` (por defecto `3000`)

Referencia de implementación: [components/bunny/network/discovery.c](components/bunny/network/discovery.c)

### Payload esperado

El anuncio tiene este formato JSON:

```json
{
  "bunny": true,
  "id": "esp32-001",
  "name": "Mi Dispositivo Bunny",
  "version": "0.1.0",
  "ip": "192.168.100.176",
  "webhook_port": 8080,
  "webhook_path": "/bunny"
}
```

### Reglas mínimas que debe aplicar el motor

1. Escuchar UDP en `0.0.0.0:<udp_port>`.
2. Parsear solo paquetes JSON válidos con `bunny == true`.
3. Considerar disponible un dispositivo por su `id`.
4. Guardar/actualizar un registro con:
   - `id`, `name`, `version`
   - `ip`, `webhook_port`, `webhook_path`
   - `last_seen` (timestamp del último anuncio)
5. Marcar offline si no recibe anuncios por una ventana de timeout (ejemplo: `3 x interval`).

---

## 2) Conectarse al dispositivo (WebSocket)

Con los datos del anuncio, el motor construye la URL:

`ws://<ip>:<webhook_port><webhook_path>`

Ejemplo:

`ws://192.168.100.176:8080/bunny`

Referencia de implementación del servidor: [components/bunny/network/network.c](components/bunny/network/network.c)

### Reglas recomendadas de conexión en el motor

1. Solo intentar WebSocket a dispositivos marcados como `available`.
2. Permitir reconexión con backoff si falla.
3. Evitar múltiples conexiones simultáneas para el mismo `id`.
4. Si cambia `ip` para un `id` conocido, cerrar la conexión anterior y reconectar a la nueva URL.

---

## 3) Validar que sí se conectó

No basta con “descubrir por UDP”; UDP solo indica presencia. La conexión real se valida con WebSocket.

### Validación desde el motor (obligatoria)

Considerar estado `connected` solo cuando se cumple:

1. El cliente WebSocket dispara `on_open` / handshake exitoso.
2. El socket reporta estado abierto (`OPEN`).

Luego de eso, marcar el dispositivo como `connected=true`.

### Validación adicional recomendada

1. Enviar un `ping` de WebSocket y esperar `pong`.
2. Si no hay `pong` dentro del timeout, degradar a `disconnected`.

### Validación desde el dispositivo (diagnóstico)

En monitor ESP32, Bunny imprime:

- `WebSocket handshake completed from /bunny`
- `FLOW[4/4] WebSocket connection established (real)`

Esto confirma que el dispositivo también ve la sesión activa.

Referencia: [components/bunny/network/network.c](components/bunny/network/network.c), [components/bunny/bunny_sdk.cpp](components/bunny/bunny_sdk.cpp)

---

## Máquina de estados sugerida (motor)

Estados por dispositivo:

1. `discovered`: llegó UDP válido.
2. `connecting`: intentando abrir WebSocket.
3. `connected`: handshake + socket abierto.
4. `stale`: sin anuncios UDP recientes.
5. `disconnected`: socket cerrado o error.

Transiciones clave:

1. `discovered -> connecting`: al recibir/actualizar anuncio.
2. `connecting -> connected`: `on_open` exitoso.
3. `connected -> disconnected`: `on_close`/error/ping timeout.
4. `connected -> stale`: deja de llegar UDP por timeout.
5. `stale -> connecting`: reaparece UDP o cambia endpoint.

---

## Checklist de cumplimiento para el motor de procesos

1. Escucha UDP en el puerto configurado por Bunny.
2. Filtra por `bunny == true` y deduplica por `id`.
3. Construye URL WebSocket con `ip + webhook_port + webhook_path`.
4. Conecta con reconexión y backoff.
5. Marca conectado solo al completar handshake (`on_open`).
6. Mantiene health-check (`ping/pong` o equivalente).
7. Maneja actualización de IP/endpoint sin duplicar sesiones.
8. Marca offline por timeout de anuncios UDP.

---

## Límites actuales del framework (importante)

En el estado actual del proyecto:

1. El servidor WebSocket ya recibe frames y marca handshake.
2. El despacho final de comandos desde protocolo aún está en progreso.

Por eso, la validación de conexión debe enfocarse hoy en handshake + estabilidad del socket, no en una respuesta de ejecución de comando como condición única de “conectado”.

Referencias:

- [components/bunny/network/network.c](components/bunny/network/network.c)
- [components/bunny/protocol/protocol.c](components/bunny/protocol/protocol.c)
