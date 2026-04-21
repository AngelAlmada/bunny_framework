# Glosario de Términos

Documento de referencia rápida para desarrolladores que se integran al proyecto Bunny.

**← [Volver al README](README.md)**

## Índice

1. [Arquitectura y conceptos base](#arquitectura-y-conceptos-base)
2. [Capacidades del SDK](#capacidades-del-sdk)
3. [Diseño de API y código](#diseño-de-api-y-código)
4. [Comunicación y datos](#comunicación-y-datos)
5. [Operación en ESP32](#operación-en-esp32)

---

## Arquitectura y conceptos base

**Bunny Framework**
Conjunto de componentes para ESP32 que permite declarar capacidades de hardware (sensores, comandos, eventos y estados) para que un motor de procesos externo decida la lógica.

**Motor de procesos**
Sistema externo que define reglas, flujos, condiciones y transiciones. No es un backend web tradicional; su rol principal es orquestar procesos y enviar instrucciones declarativas al dispositivo.

**Lógica de negocio**
Conjunto de reglas de decisión (por ejemplo, condiciones y flujos). En Bunny, debe vivir en el motor de procesos, no en el firmware del ESP32.

**Arquitectura declarativa**
Estilo donde se describe qué puede hacer el dispositivo (capacidades), en lugar de programar en el dispositivo todas las decisiones de cuándo y por qué ejecutarlas.

**Separación de responsabilidades (edge vs motor)**
Principio por el cual el ESP32 ejecuta acciones de hardware y el motor de procesos toma decisiones de alto nivel.

**Contrato**
Acuerdo formal sobre estructura de datos y comportamientos esperados. En Bunny, metadata + tipos + nombres de capacidades forman el contrato con el motor de procesos.

---

## Capacidades del SDK

**Capability (Capacidad)**
Unidad declarativa que representa algo que el dispositivo puede exponer o ejecutar.

**Sensor**
Capacidad que produce datos (lectura). Ejemplo: temperatura, humedad, voltaje.

**Command**
Capacidad que ejecuta una acción de hardware al recibir parámetros. Ejemplo: encender ventilador.

**Event**
Capacidad que notifica que algo ocurrió. Ejemplo: movimiento detectado.

**State**
Capacidad que mantiene un valor interno legible/escribible por el motor de procesos. Ejemplo: fanState.

**ICapability**
Interfaz base común para todas las capacidades. Define métodos como tipo (`kind`), nombre (`name`), metadata y serialización.

**CapabilityKind**
Enum que clasifica las capacidades: `SENSOR`, `COMMAND`, `EVENT`, `STATE`.

**Registry**
Registro central donde se almacenan todas las capacidades declaradas para búsqueda y serialización del manifiesto.

---

## Diseño de API y código

**Fluent API**
Patrón de diseño que permite encadenar métodos para construir definiciones legibles. Ejemplo: `Bunny.sensor(...).description(...).returns(...).build(...)`.

**Builder**
Objeto que construye una capacidad paso a paso antes de registrarla. En Bunny: `SensorBuilder`, `CommandBuilder`, `EventBuilder`, `StateBuilder`.

**Hook**
Función callback conectada a una capacidad para ejecutar comportamiento de hardware.

**Read hook**
Callback que lee hardware para un sensor (`SensorReadFn`).

**Execute hook**
Callback que ejecuta una acción de hardware para un comando (`CommandExecuteFn`).

**Emit hook**
Callback opcional al emitir eventos (`EventEmitFn`) para efectos locales (por ejemplo, blink LED).

**Getter/Setter de estado**
Callbacks para leer y escribir un estado (`StateGetFn`, `StateSetFn`).

**Singleton**
Patrón de diseño que garantiza una sola instancia global (por ejemplo, `Registry::instance()`).

**Type safety (tipado fuerte)**
Uso explícito de tipos para reducir errores de integración entre firmware y motor de procesos.

---

## Comunicación y datos

**Metadata semántica**
Información descriptiva de una capacidad: `description`, `params`, `returns`, `tags`, `affects`, `example`.

**ParamDef**
Definición tipada de un parámetro de entrada (`name`, `type`, `description`, `required`).

**Type system**
Sistema de tipos del SDK: `NUMBER`, `STRING`, `BOOLEAN`, `OBJECT`, `ARRAY`, `VOID`.

**JSON**
Formato de intercambio de datos usado para capacidades, comandos, eventos y respuestas.

**DSL (Domain-Specific Language)**
Lenguaje específico del dominio. En Bunny, el JSON actúa como DSL para describir capacidades y procesos.

**Serialización**
Proceso de convertir estructuras en memoria a JSON para enviarlas al motor de procesos.

**Deserialización**
Proceso de convertir JSON entrante a estructuras de datos internas (por ejemplo, parámetros de comando).

**Manifest de capacidades**
JSON que lista las capacidades registradas del dispositivo (`sensors`, `commands`, `events`, `states`).

**Validación estructurada**
Verificación de que mensajes y parámetros cumplen el contrato de tipos y metadata.

---

## Operación en ESP32

**Runtime**
Ciclo de ejecución del firmware que atiende comunicación, despacha requests y ejecuta hooks de capacidades.

**Edge device**
Dispositivo en el borde (ESP32) cercano al hardware físico.

**Discovery UDP**
Mecanismo de descubrimiento en red local para anunciar identidad y datos de conexión del dispositivo.

**Webhook**
Endpoint HTTP expuesto por el dispositivo para recibir instrucciones o notificaciones desde el motor de procesos.

**ESP-IDF**
Framework oficial de Espressif para desarrollo en ESP32.

**Firmware**
Software embebido que corre en el microcontrolador.

**Mock runtime**
Modo simulado para probar comportamiento sin hardware real.

---

Si detectas términos ambiguos o nuevos en el código, agrega su definición aquí para mantener un lenguaje común en todo el equipo.
