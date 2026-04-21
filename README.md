# Bunny Framework (ESP32)

Framework declarativo para ESP32 orientado a **capacidades de hardware** (sensores, comandos, eventos y estados), donde la **lógica de negocio vive en backend**.

## Objetivo

Bunny separa completamente responsabilidades:

- **ESP32 + Bunny**: define capacidades y ejecuta acciones de hardware.
- **Backend**: decide reglas, flujos, condiciones y procesos.
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
- `protocol/`: parseo/serializacion de mensajes JSON (backend <-> dispositivo).
- `runtime/`: ciclo de ejecucion y despacho de hooks (sin logica de negocio).
- `network/`: transporte (discovery UDP y conexion con backend/webhook).
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

- Menos errores de uso desde backend/LLM.
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
- No acoplar decisiones del backend en firmware.
- Si una capacidad no esta bien documentada en metadata, su uso por backend/LLM sera fragil.

## Build (ESP-IDF)

```bash
idf.py build
```

Para flashear:

```bash
idf.py -p <PORT> flash monitor
```

## Estado actual

- SDK declarativo base implementado.
- Registro y serializacion de capacidades listos.
- Ejemplos de `sensor`, `command`, `event`, `state` en `main/`.
- Discovery UDP y protocolo/runtime aun con partes marcadas como `TODO` para completar transporte y despacho final.
