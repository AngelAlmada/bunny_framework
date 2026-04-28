# Bunny Framework (ESP32)

Framework declarativo para ESP32 orientado a **capacidades de hardware** (sensores, comandos, eventos y estados), donde la **lógica de negocio vive en motor de procesos**.

## 📚 Índice de documentación

- [BCP - Especificación del Bunny Communication Protocol](#bcp---especificación-del-bunny-communication-protocol)
- [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md) — **Guía técnica completa para desarrolladores** (Sistema de tipos, Metadata, Builders, Registry, ejemplos completos)
- [INCLUDES_GUIDE.md](INCLUDES_GUIDE.md) — **Qué incluir en cada archivo y por qué** (bunny_sdk.h vs bunny.h, árbol de includes, tabla de referencia rápida)
- [PROCESS_ENGINE_CONNECTION_GUIDE.md](PROCESS_ENGINE_CONNECTION_GUIDE.md) — **Cómo debe operar el motor de procesos** (discovery UDP, conexión WebSocket y validación de conexión)
- [PROCESS_ENGINE_WEBSOCKET_GUIDE.md](PROCESS_ENGINE_WEBSOCKET_GUIDE.md) — **Cómo debe manejar el motor la sesión WebSocket** (handshake, framing, health-check, reconexión y errores comunes)
- [GLOSARIO.md](GLOSARIO.md) — **Definición de términos clave** (Fluent API, DSL, hooks, runtime, Registry, etc.)
- [BUNNY_PROGRAMMING_PHILOSOPHY.md](BUNNY_PROGRAMMING_PHILOSOPHY.md) — **Cómo programar en Bunny** (filosofía capabilities-first y separación lógica vs firmware)
- [NETWORK_OVERVIEW.md](NETWORK_OVERVIEW.md) — **Cómo funciona la red de Bunny (visión general)** (WiFi, discovery UDP, WebSocket, FLOW y operación)
- [Documentación de tesis](#documentación-de-tesis)
- [Ejemplos de Capacidades](#ejemplos-de-capacidades-ejemplo-módulos)
- [Estructura del Proyecto](#estructura-del-proyecto)
- [Archivos Clave del SDK](#archivos-clave-del-sdk)
- [Configuración del Dispositivo](#configuración-del-dispositivo)

## BCP - Especificación del Bunny Communication Protocol

- [BCP_SPECIFICATION.md](BCP_SPECIFICATION.md) — **Especificación oficial de protocolo** (discovery, handshake, heartbeat, ejecución de capacidades, eventos, validación JSON, estado de implementación y brechas pendientes)

## Documentación de tesis

- [DOCUMENTACION_TESIS_PRINCIPAL.md](DOCUMENTACION_TESIS_PRINCIPAL.md) — Versión condensada para el cuerpo principal de tesis (arquitectura, flujo y evidencia de implementación)
- [DOCUMENTACION_TESIS_ANEXOS.md](DOCUMENTACION_TESIS_ANEXOS.md) — Versión extendida para anexos (detalles técnicos, decisiones y fragmentos adicionales de código)

## Objetivo

Bunny separa completamente responsabilidades:

- **ESP32 + Bunny**: define capacidades y ejecuta acciones de hardware.
- **Motor de procesos**: decide reglas, flujos, condiciones y procesos.
- **JSON (DSL)**: contrato de comunicación entre ambos.

> Regla principal: el firmware NO decide lógica de negocio.

## Principios de diseño

- Arquitectura declarativa (capabilities-first).
- Tipado fuerte en parámetros y retornos.
- Metadata semántica para consumo humano y LLM.
- Runtime liviano para edge (ESP32-friendly).
- Separación estricta entre ejecución de hardware y lógica de procesos.

## Tipos de capacidades

1. **Sensor**: produce datos.
2. **Command**: ejecuta acciones de hardware.
3. **Event**: notifica sucesos.
4. **State**: mantiene estado interno tipado.

## Ejemplos de Capacidades (Módulos de ejemplo)

El proyecto incluye 4 módulos de ejemplo que muestran cómo declarar cada tipo de capacidad:

| Tipo | Archivo | Código |
|------|---------|--------|
| **Sensor** | [main/sensors/temperature_sensor.cpp](main/sensors/temperature_sensor.cpp) | Lee temperatura (hardware mock) |
| **Command** | [main/commands/fan_command.cpp](main/commands/fan_command.cpp) | Enciende/apaga ventilador |
| **Event** | [main/events/motion_event.cpp](main/events/motion_event.cpp) | Notifica movimiento detectado |
| **State** | [main/states/fan_state.cpp](main/states/fan_state.cpp) | Mantiene estado del ventilador |

Para crear tus propios módulos, <u>sigue los patrones de estos ejemplos</u> y consulta [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md).

## Estructura del proyecto

```text
bunny_framework/
├── components/
│   └── bunny/
│       ├── include/
│       ├── types/
│       ├── metadata/
│       ├── core/
│       ├── capabilities/
│       ├── builder/
│       ├── registry/
│       ├── protocol/
│       ├── runtime/
│       ├── network/
│       ├── config/
│       └── utils/
├── config/
│   └── device.json
├── main/
│   ├── bunny_framework.c
│   ├── sensors/
│   ├── commands/
│   ├── events/
│   └── states/
├── CMakeLists.txt
└── sdkconfig
```

## Proposito de cada carpeta

### `components/bunny/`
Codigo del framework en si (SDK + runtime base).

- `include/`: API C compatible para `app_main` (`bunny_begin`, `bunny_load_modules`, `bunny_loop`).
- `types/`: sistema de tipos (`NUMBER`, `STRING`, `BOOLEAN`, `OBJECT`, `ARRAY`).
- `metadata/`: contrato semantico de capacidades (`description`, `params`, `returns`, `tags`, `affects`, `example`).
- `core/`: contratos base (`ICapability`, kind de capacidad).
- `capabilities/`: implementaciones de `SensorCapability`, `CommandCapability`, `EventCapability`, `StateCapability`.
- `builder/`: fluent API para declarar capacidades de forma ergonomica.
- `registry/`: registro central de capacidades y serializacion a JSON.
- `protocol/`: parseo/serializacion de mensajes JSON (motor de procesos <-> dispositivo).
- `runtime/`: ciclo de ejecucion y despacho de hooks (sin logica de negocio).
- `network/`: transporte (discovery UDP y conexion con motor de procesos/webhook).
- `config/`: carga y acceso de configuracion del dispositivo (`device.json`).
- `utils/`: helpers ligeros compartidos (ej. construccion JSON sin dependencias).

### `config/`
Configuracion declarativa editable por el usuario del dispositivo.

- `device.json`: identidad del dispositivo, discovery UDP y webhook.

### `main/`
Punto de entrada del firmware y modulos de ejemplo definidos por el usuario.

- `bunny_framework.c`: inicializa Bunny y registra capacidades.
- `sensors/`: declaraciones de sensores.
- `commands/`: declaraciones de comandos.
- `events/`: declaraciones de eventos.
- `states/`: declaraciones de estados.

## Archivos clave del SDK

Para trabajar con Bunny, estos son los archivos más importantes:

### API Pública (incluir en tus módulos)

- [components/bunny/bunny_sdk.h](components/bunny/bunny_sdk.h) — **Interfaz principal del SDK** (`Bunny` global, builders, lifecycle)
- [components/bunny/include/bunny.h](components/bunny/include/bunny.h) — **Interfaz C compatible** (app_main)

### Core

- [components/bunny/types/bunny_types.h](components/bunny/types/bunny_types.h) — Sistema de tipos (NUMBER, STRING, BOOLEAN, OBJECT, ARRAY)
- [components/bunny/metadata/metadata.h](components/bunny/metadata/metadata.h) — Estructura de metadata semántica
- [components/bunny/core/capability.h](components/bunny/core/capability.h) — Interfaz base ICapability

### Builders (Fluent API)

- [components/bunny/builder/sensor_builder.h](components/bunny/builder/sensor_builder.h)
- [components/bunny/builder/command_builder.h](components/bunny/builder/command_builder.h)
- [components/bunny/builder/event_builder.h](components/bunny/builder/event_builder.h)
- [components/bunny/builder/state_builder.h](components/bunny/builder/state_builder.h)

### Capacidades

- [components/bunny/capabilities/sensor_capability.h](components/bunny/capabilities/sensor_capability.h)
- [components/bunny/capabilities/command_capability.h](components/bunny/capabilities/command_capability.h) — Incluye `Params` container
- [components/bunny/capabilities/event_capability.h](components/bunny/capabilities/event_capability.h)
- [components/bunny/capabilities/state_capability.h](components/bunny/capabilities/state_capability.h)

### Infraestructura

- [components/bunny/registry/registry.h](components/bunny/registry/registry.h) — Registro central y serialización JSON
- [components/bunny/config/config.h](components/bunny/config/config.h) — Carga de configuración del dispositivo
- [components/bunny/network/discovery.h](components/bunny/network/discovery.h) — Discovery UDP
- [NETWORK_OVERVIEW.md](NETWORK_OVERVIEW.md) — Visión general del flujo de red (WiFi + UDP + WebSocket)
- [components/bunny/utils/json_builder.h](components/bunny/utils/json_builder.h) — Helper JSON sin dependencias

## Flujo de arranque esperado

1. `app_main` llama `bunny_begin()`.
2. Se carga configuracion del dispositivo.
3. Se inicializa discovery de red.
4. Se registran capacidades declaradas en `main/`.
5. `bunny_load_modules()` deja listo el runtime.
6. `bunny_loop()` mantiene el ciclo de comunicacion/ejecucion.

## Metadata obligatoria (muy importante)

Cada capacidad debe definir, idealmente siempre:

- `description`
- `params` (cuando aplique)
- `returns`
- `tags`
- `affects` (opcional)
- `example` (opcional, recomendado)

Esto permite:

- Menos errores de uso desde motor de procesos/LLM.
- Validacion estructurada de invocaciones.
- Generacion de procesos y tooling automatico.

## Ejemplo rapido de uso (SDK)

```cpp
#include "bunny_sdk.h"

void register_temperature_sensor() {
  Bunny.sensor("temperature")
       .description("Ambient temperature in Celsius")
       .returns(NUMBER)
       .tag("environment")
       .build([]() -> double {
         return 23.5; // lectura de hardware
       });
}

void register_fan_command() {
  Bunny.command("setFanState")
       .description("Turn fan ON or OFF")
       .param("state", STRING, "ON or OFF")
       .affects("fanState")
       .execute([](const bunny::Params& p) {
         const char* state = p.get_string("state");
         (void)state; // accion hardware aqui
       });
}
```

## Reglas importantes a tener en cuenta

- No meter reglas de negocio en ESP32 (`if/else` de negocio, workflows, condiciones de proceso).
- No evaluar strings como codigo.
- No acoplar decisiones del motor de procesos en firmware.
- Si una capacidad no esta bien documentada en metadata, su uso por motor de procesos/LLM sera fragil.

## Build (ESP-IDF)

```bash
idf.py build
```

Para flashear:

```bash
idf.py -p <PORT> flash monitor
```

## Configuración del dispositivo

Ver [config/device.json](config/device.json) para configurar:

- **Identidad del dispositivo**: `id`, `name`, `description`, `version`
- **Discovery UDP**: puerto y intervalo de broadcast
- **Webhook**: puerto y path para conexión con motor de procesos

## Estado actual

- SDK declarativo base implementado.
- Registro y serializacion de capacidades listos.
- Ejemplos de `sensor`, `command`, `event`, `state` en `main/`.
- Discovery UDP y protocolo/runtime aun con partes marcadas como `TODO` para completar transporte y despacho final.
