# Bunny Framework

Framework declarativo para ESP32 orientado a capacidades de hardware (sensores, comandos, eventos y estados), donde la logica de negocio vive en un motor de procesos externo.

## Documentacion principal

La documentacion oficial del proyecto se encuentra en:

- [doc/README.md](doc/README.md)

## Especificacion del protocolo

La especificacion oficial de comunicacion esta en:

- [doc/BCP_SPECIFICATION.md](doc/BCP_SPECIFICATION.md)

## Estructura

- Codigo fuente del framework: [components/bunny](components/bunny)
- App de ejemplo: [main](main)
- Configuracion del dispositivo: [config/device.json](config/device.json)
- Documentacion completa: [doc](doc)
# Bunny Framework (ESP32)

Framework declarativo para ESP32 orientado a **capacidades de hardware** (sensores, comandos, eventos y estados), donde la **lógica de negocio vive en motor de procesos**.

## 📚 Índice de documentación

- [BCP - Especificación del Bunny Communication Protocol](#bcp---especificación-del-bunny-communication-protocol)
- [INSTALL_COMMAND.md](doc/INSTALL_COMMAND.md) — **Comando bunny install** (instalación automática de entorno de desarrollo completo)
- [MONITOR_COMMAND.md](doc/MONITOR_COMMAND.md) — **Comando bunny monitor** (ver output serial del ESP32 en tiempo real)
- [SETUP_ENVIRONMENT.md](doc/SETUP_ENVIRONMENT.md) — **Cómo agregar bunny al PATH** (instalación universal, pasos para cada plataforma, solución de problemas)
- [WINDOWS_SETUP.md](doc/WINDOWS_SETUP.md) — **Guía de instalación y uso en Windows** (requisitos, instalación, comandos CLI, troubleshooting)
- [DEVELOPER_GUIDE.md](doc/DEVELOPER_GUIDE.md) — **Guía técnica completa para desarrolladores** (Sistema de tipos, Metadata, Builders, Registry, ejemplos completos)
- [PROCESS_ENGINE_WEBSOCKET_GUIDE.md](doc/PROCESS_ENGINE_WEBSOCKET_GUIDE.md) — **Cómo debe manejar el motor la sesión WebSocket** (handshake, framing, health-check, reconexión y errores comunes)
- [GLOSARIO.md](doc/GLOSARIO.md) — **Definición de términos clave** (Fluent API, DSL, hooks, runtime, Registry, etc.)
- [FRAMEWORK_VS_LIBRARY.md](doc/FRAMEWORK_VS_LIBRARY.md) — **¿Por qué Bunny es un Framework? Arquitectura y Patrones** (Diferencias framework vs librería, arquitectura hexagonal, patrones de diseño, separación de responsabilidades)
- [BUNNY_PROGRAMMING_PHILOSOPHY.md](doc/BUNNY_PROGRAMMING_PHILOSOPHY.md) — **Cómo programar en Bunny** (filosofía capabilities-first y separación lógica vs firmware)
- [NETWORK_OVERVIEW.md](doc/NETWORK_OVERVIEW.md) — **Cómo funciona la red de Bunny (visión general)** (WiFi, discovery UDP, WebSocket, FLOW y operación)
- [Documentación de tesis](#documentación-de-tesis)
- [Ejemplos de Capacidades](#ejemplos-de-capacidades-módulos-de-ejemplo)
- [Estructura del Proyecto](#estructura-del-proyecto)
- [Archivos Clave del SDK](#archivos-clave-del-sdk)
- [Configuración del Dispositivo](#configuración-del-dispositivo)

## BCP - Especificación del Bunny Communication Protocol

- [BCP_SPECIFICATION.md](doc/BCP_SPECIFICATION.md) — **Especificación oficial de protocolo** (discovery, handshake, heartbeat, ejecución de capacidades, eventos, validación JSON, estado de implementación y brechas pendientes)

## Documentación de tesis

- [DOCUMENTACION_TESIS_PRINCIPAL.md](doc/DOCUMENTACION_TESIS_PRINCIPAL.md) — Versión condensada para el cuerpo principal de tesis (arquitectura, flujo y evidencia de implementación)
- [DOCUMENTACION_TESIS_ANEXOS.md](doc/DOCUMENTACION_TESIS_ANEXOS.md) — Versión extendida para anexos (detalles técnicos, decisiones y fragmentos adicionales de código)
- [DOCUMENTACION_TESIS_DIAPOSITIVA_UNICA.md](doc/DOCUMENTACION_TESIS_DIAPOSITIVA_UNICA.md) — Versión ultra resumida para una sola diapositiva (qué es, para qué sirve y código esencial)

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

## Comandos rápidos (Bunny CLI)

El framework incluye comandos `bunny` para automatizar tareas comunes.

### Instalación Rápida (Recomendado)

Para usuarios nuevos, usa el comando `bunny install` para instalar todo automáticamente:

```bash
cd C:\proyects\bunny_framework    # Windows
cd /ruta/a/bunny_framework        # Linux/macOS

bunny install
```

Esto instala:
- ✅ Python (si es necesario)
- ✅ ESP-IDF (descarga e instala automáticamente)
- ✅ Variables de entorno (bunny disponible en cualquier ubicación)

Ver [doc/INSTALL_COMMAND.md](doc/INSTALL_COMMAND.md) para más detalles.

### Instalación Manual (Alternativa)

Si prefieres hacer el setup por partes:

#### Windows (CMD):
```cmd
cd C:\ruta\a\bunny_framework
setup-bunny-env.bat
```

#### Windows (PowerShell):
```powershell
cd C:\ruta\a\bunny_framework
.\setup-bunny-env.ps1
```

#### Linux/macOS:
```bash
cd /ruta/a/bunny_framework
bash setup-bunny-env.sh
```

#### Universal (Python):
```bash
python setup-bunny-env.py
```

Después de ejecutar el setup, puedes usar `bunny` desde cualquier directorio.

### Uso del comando `bunny`

#### Instalación de entorno (1 vez)
```bash
bunny install
bunny i           # Alias corto
```

#### Flasheo de ESP32
```bash
bunny flash       # Flashear
bunny flash clean # Limpiar y flashear
```

#### Monitor Serial
```bash
bunny monitor     # Ver output del ESP32
bunny m           # Alias corto
```

#### En Linux/macOS:
```bash
./bunny flash
./bunny flash clean
./bunny monitor
BUNNY_PORT=/dev/ttyUSB1 ./bunny monitor
```

#### En Windows (CMD):
```cmd
bunny flash
bunny flash clean
bunny monitor
set BUNNY_PORT=COM4 && bunny monitor
```

#### En Windows (PowerShell):
```powershell
.\bunny.ps1 flash
.\bunny.ps1 flash clean
.\bunny.ps1 monitor
$env:BUNNY_PORT='COM4'; .\bunny.ps1 monitor
```

#### O después de ejecutar setup (desde cualquier ubicación):
```powershell
bunny flash
```

**Nota:** El comando detecta automáticamente el puerto serial. Si tienes múltiples dispositivos, usa `BUNNY_PORT` para especificar cuál usar.

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
