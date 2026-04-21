# Glosario de Términos

Documento de referencia rápida para desarrolladores que se integran al proyecto Bunny.

**← [Volver al README](README.md)**

## Índice de términos (enlace directo)

- [Arquitectura declarativa](#term-arquitectura-declarativa)
- [Bunny Framework](#term-bunny-framework)
- [Builder](#term-builder)
- [Capability (Capacidad)](#term-capability)
- [CapabilityKind](#term-capabilitykind)
- [Command](#term-command)
- [Contrato](#term-contrato)
- [Deserialización](#term-deserializacion)
- [Discovery UDP](#term-discovery-udp)
- [DSL (Domain-Specific Language)](#term-dsl)
- [Edge device](#term-edge-device)
- [Emit hook](#term-emit-hook)
- [ESP-IDF](#term-esp-idf)
- [Event](#term-event)
- [Execute hook](#term-execute-hook)
- [Firmware](#term-firmware)
- [Fluent API](#term-fluent-api)
- [Getter/Setter de estado](#term-getter-setter-estado)
- [Hook](#term-hook)
- [ICapability](#term-icapability)
- [JSON](#term-json)
- [LLM (Large Language Model)](#term-llm)
- [Lógica de negocio](#term-logica-negocio)
- [Manifest de capacidades](#term-manifest-capacidades)
- [Metadata semántica](#term-metadata-semantica)
- [Mock runtime](#term-mock-runtime)
- [Motor de procesos](#term-motor-procesos)
- [ParamDef](#term-paramdef)
- [Read hook](#term-read-hook)
- [Registry](#term-registry)
- [Runtime](#term-runtime)
- [Sensor](#term-sensor)
- [Separación de responsabilidades (edge vs motor)](#term-separacion-responsabilidades)
- [Serialización](#term-serializacion)
- [Singleton](#term-singleton)
- [State](#term-state)
- [Type safety (tipado fuerte)](#term-type-safety)
- [Type system](#term-type-system)
- [Validación estructurada](#term-validacion-estructurada)
- [Webhook](#term-webhook)

---

<a id="term-arquitectura-declarativa"></a>
### Arquitectura declarativa
Estilo donde se describe qué puede hacer el dispositivo (capacidades), en lugar de programar en el dispositivo todas las decisiones de cuándo y por qué ejecutarlas.

<a id="term-bunny-framework"></a>
### Bunny Framework
Conjunto de componentes para ESP32 que permite declarar capacidades de hardware (sensores, comandos, eventos y estados) para que un motor de procesos externo decida la lógica.

<a id="term-builder"></a>
### Builder
Objeto que construye una capacidad paso a paso antes de registrarla. En Bunny: `SensorBuilder`, `CommandBuilder`, `EventBuilder`, `StateBuilder`.

<a id="term-capability"></a>
### Capability (Capacidad)
Unidad declarativa que representa algo que el dispositivo puede exponer o ejecutar.

<a id="term-capabilitykind"></a>
### CapabilityKind
Enum que clasifica las capacidades: `SENSOR`, `COMMAND`, `EVENT`, `STATE`.

<a id="term-command"></a>
### Command
Capacidad que ejecuta una acción de hardware al recibir parámetros. Ejemplo: encender ventilador.

<a id="term-contrato"></a>
### Contrato
Acuerdo formal sobre estructura de datos y comportamientos esperados. En Bunny, metadata + tipos + nombres de capacidades forman el contrato con el motor de procesos.

<a id="term-deserializacion"></a>
### Deserialización
Proceso de convertir JSON entrante a estructuras de datos internas (por ejemplo, parámetros de comando).

<a id="term-discovery-udp"></a>
### Discovery UDP
Mecanismo de descubrimiento en red local para anunciar identidad y datos de conexión del dispositivo.

<a id="term-dsl"></a>
### DSL (Domain-Specific Language)
Lenguaje específico del dominio. En Bunny, el JSON actúa como DSL para describir capacidades y procesos.

<a id="term-edge-device"></a>
### Edge device
Dispositivo en el borde (ESP32) cercano al hardware físico.

<a id="term-emit-hook"></a>
### Emit hook
Callback opcional al emitir eventos (`EventEmitFn`) para efectos locales (por ejemplo, blink LED).

<a id="term-esp-idf"></a>
### ESP-IDF
Framework oficial de Espressif para desarrollo en ESP32.

<a id="term-event"></a>
### Event
Capacidad que notifica que algo ocurrió. Ejemplo: movimiento detectado.

<a id="term-execute-hook"></a>
### Execute hook
Callback que ejecuta una acción de hardware para un comando (`CommandExecuteFn`).

<a id="term-firmware"></a>
### Firmware
Software embebido que corre en el microcontrolador.

<a id="term-fluent-api"></a>
### Fluent API
Patrón de diseño que permite encadenar métodos para construir definiciones legibles. Ejemplo: `Bunny.sensor(...).description(...).returns(...).build(...)`.

<a id="term-getter-setter-estado"></a>
### Getter/Setter de estado
Callbacks para leer y escribir un estado (`StateGetFn`, `StateSetFn`).

<a id="term-hook"></a>
### Hook
Función callback conectada a una capacidad para ejecutar comportamiento de hardware.

<a id="term-icapability"></a>
### ICapability
Interfaz base común para todas las capacidades. Define métodos como tipo (`kind`), nombre (`name`), metadata y serialización.

<a id="term-json"></a>
### JSON
Formato de intercambio de datos usado para capacidades, comandos, eventos y respuestas.

<a id="term-llm"></a>
### LLM (Large Language Model)
Modelo de lenguaje de gran escala que puede interpretar metadata y contratos para sugerir acciones, generar procesos o asistir en integración, siempre que las capacidades estén bien documentadas.

<a id="term-logica-negocio"></a>
### Lógica de negocio
Conjunto de reglas de decisión (por ejemplo, condiciones y flujos). En Bunny, debe vivir en el motor de procesos, no en el firmware del ESP32.

<a id="term-manifest-capacidades"></a>
### Manifest de capacidades
JSON que lista las capacidades registradas del dispositivo (`sensors`, `commands`, `events`, `states`).

<a id="term-metadata-semantica"></a>
### Metadata semántica
Información descriptiva de una capacidad: `description`, `params`, `returns`, `tags`, `affects`, `example`.

<a id="term-mock-runtime"></a>
### Mock runtime
Modo simulado para probar comportamiento sin hardware real.

<a id="term-motor-procesos"></a>
### Motor de procesos
Sistema externo que define reglas, flujos, condiciones y transiciones. No es un backend web tradicional; su rol principal es orquestar procesos y enviar instrucciones declarativas al dispositivo.

<a id="term-paramdef"></a>
### ParamDef
Definición tipada de un parámetro de entrada (`name`, `type`, `description`, `required`).

<a id="term-read-hook"></a>
### Read hook
Callback que lee hardware para un sensor (`SensorReadFn`).

<a id="term-registry"></a>
### Registry
Registro central donde se almacenan todas las capacidades declaradas para búsqueda y serialización del manifiesto.

<a id="term-runtime"></a>
### Runtime
Ciclo de ejecución del firmware que atiende comunicación, despacha requests y ejecuta hooks de capacidades.

<a id="term-sensor"></a>
### Sensor
Capacidad que produce datos (lectura). Ejemplo: temperatura, humedad, voltaje.

<a id="term-separacion-responsabilidades"></a>
### Separación de responsabilidades (edge vs motor)
Principio por el cual el ESP32 ejecuta acciones de hardware y el motor de procesos toma decisiones de alto nivel.

<a id="term-serializacion"></a>
### Serialización
Proceso de convertir estructuras en memoria a JSON para enviarlas al motor de procesos.

<a id="term-singleton"></a>
### Singleton
Patrón de diseño que garantiza una sola instancia global (por ejemplo, `Registry::instance()`).

<a id="term-state"></a>
### State
Capacidad que mantiene un valor interno legible/escribible por el motor de procesos. Ejemplo: fanState.

<a id="term-type-safety"></a>
### Type safety (tipado fuerte)
Uso explícito de tipos para reducir errores de integración entre firmware y motor de procesos.

<a id="term-type-system"></a>
### Type system
Sistema de tipos del SDK: `NUMBER`, `STRING`, `BOOLEAN`, `OBJECT`, `ARRAY`, `VOID`.

<a id="term-validacion-estructurada"></a>
### Validación estructurada
Verificación de que mensajes y parámetros cumplen el contrato de tipos y metadata.

<a id="term-webhook"></a>
### Webhook
Endpoint HTTP expuesto por el dispositivo para recibir instrucciones o notificaciones desde el motor de procesos.

---

Si detectas términos ambiguos o nuevos en el código, agrega su definición aquí para mantener un lenguaje común en todo el equipo.
