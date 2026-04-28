# Bunny Framework — Red (Visión General)

Documento general de cómo funciona la red en Bunny Framework y cómo se relaciona con el motor de procesos.

Última actualización: 21 de abril de 2026.

**← [Volver al README](README.md)**

## Objetivo de la red en Bunny

La capa de red de Bunny tiene dos responsabilidades principales:

1. Descubrir dispositivos en la LAN.
2. Mantener un canal bidireccional para comando/evento entre ESP32 y motor de procesos.

Bunny separa estas fases en dos transportes:

1. UDP Broadcast para descubrimiento.
2. WebSocket para sesión activa de intercambio.

## Flujo general

1. El ESP32 arranca, carga configuración y capacidades.
2. Intenta conexión WiFi (STA).
3. Levanta servidor WebSocket en `webhook.port` + `webhook.path`.
4. Emite anuncios UDP periódicos con identidad y endpoint de conexión.
5. El motor de procesos detecta el anuncio UDP y abre conexión WebSocket.
6. A partir del handshake, el canal queda listo para comandos, eventos y lecturas.

## Etapas FLOW en monitor

El runtime imprime un flujo de estado para diagnóstico:

1. `FLOW[0/4]`: esperando red + motor.
2. `FLOW[1/4]`: WiFi conectado (IP asignada).
3. `FLOW[2/4]`: anuncios UDP detectados/salientes.
4. `FLOW[3/4]`: esperando handshake WebSocket del motor.
5. `FLOW[4/4]`: conexión WebSocket establecida.

Estas etapas son útiles para saber en qué punto está bloqueado el enlace.

## Descubrimiento UDP

### Qué publica

El ESP32 transmite un JSON de descubrimiento con:

- `bunny`: marca del protocolo.
- `id`, `name`, `version`: identidad del dispositivo.
- `ip`: IP local del ESP32.
- `webhook_port`, `webhook_path`: endpoint WebSocket.

### Dónde se configura

En [config/device.json](../config/device.json):

- `discovery.enabled`
- `discovery.udp_port`
- `discovery.broadcast_interval_ms`

### Qué hace el motor

El motor abre un socket UDP en `discovery.udp_port`, parsea anuncios y construye la URL WebSocket:

`ws://<ip>:<webhook_port><webhook_path>`

## Canal WebSocket

### Rol de cada parte

- ESP32: servidor WebSocket (HTTP upgrade).
- Motor de procesos: cliente WebSocket.

### Propósito

Transportar mensajes de operación:

- comando -> ejecución en dispositivo
- evento -> notificación hacia motor
- lecturas/estado -> sincronización y consulta

## Configuración de red

La configuración de WiFi y transporte vive en [config/device.json](../config/device.json):

- `network.wifi.ssid`
- `network.wifi.password`
- `network.wifi.auth_type`
- `network.wifi.max_retries`
- `network.wifi.timeout_ms`
- `discovery.*`
- `webhook.*`

## Qué existe hoy y qué está pendiente

### Implementado

- Conexión WiFi STA con timeout y reintentos.
- Broadcast UDP periódico de descubrimiento.
- Servidor WebSocket con handshake y recepción de frames.
- Logs de FLOW para observabilidad en monitor.

### En progreso

- Envío automático del manifiesto completo de capacidades al motor justo después del handshake WebSocket.
- Protocolo de mensajes final para despacho de comandos y respuestas de runtime.

## Recomendaciones de operación

1. Verificar alimentación estable del ESP32 para evitar brownout durante inicio de radio WiFi.
2. Confirmar que motor y ESP32 estén en la misma LAN/subred.
3. Probar primero UDP discovery y luego handshake WebSocket.
4. Usar el monitor del ESP32 como fuente principal de estado (`FLOW[x/4]`).

## Archivos relevantes

- [components/bunny/network/wifi.c](../components/bunny/network/wifi.c)
- [components/bunny/network/discovery.c](../components/bunny/network/discovery.c)
- [components/bunny/network/network.c](../components/bunny/network/network.c)
- [components/bunny/config/config.c](../components/bunny/config/config.c)
- [components/bunny/bunny_sdk.cpp](../components/bunny/bunny_sdk.cpp)
