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
Enfoque donde el firmware describe capacidades (qué puede hacer) en vez de definir reglas de negocio (cuándo y por qué hacerlo).
En Bunny esto evita acoplar decisiones al ESP32 y facilita que el motor de procesos cambie comportamientos sin reflashear.
Ejemplo: declarar `setFanState` como comando, mientras la condición "si temperatura > 28" vive fuera del firmware.

<a id="term-bunny-framework"></a>
### Bunny Framework
Framework para ESP32 centrado en exponer capacidades de hardware de forma tipada y documentada.
Su objetivo es separar ejecución de hardware (device) y orquestación lógica (motor de procesos).
Incluye SDK, registro de capacidades, serialización y base de comunicación.

<a id="term-builder"></a>
### Builder
Patrón que permite construir una capacidad paso a paso con métodos encadenables.
En Bunny mejora legibilidad y reduce errores de configuración al declarar metadata.
Ejemplo: `Bunny.command("setFan").param("state", STRING).execute(...)`.

<a id="term-capability"></a>
### Capability (Capacidad)
Unidad funcional declarada por el dispositivo.
Cada capacidad tiene identidad (`name`), tipo (`kind`) y metadata semántica.
Puede representar lectura (sensor), acción (command), notificación (event) o memoria de estado (state).

<a id="term-capabilitykind"></a>
### CapabilityKind
Enum interno que clasifica capacidades por comportamiento.
Permite al Registry filtrar y despachar correctamente.
Valores actuales: `SENSOR`, `COMMAND`, `EVENT`, `STATE`.

<a id="term-command"></a>
### Command
Capacidad orientada a ejecutar acciones sobre hardware.
Recibe parámetros tipados, valida estructura y llama un `execute hook`.
Ejemplo: `setFanState(state: STRING)` para activar o desactivar un relé.

<a id="term-contrato"></a>
### Contrato
Acuerdo técnico que define cómo debe invocarse una capacidad y qué respuesta esperar.
En Bunny lo forman nombre de capacidad, tipos, metadata y formato JSON.
Si el contrato está incompleto, la integración con motor de procesos y herramientas se vuelve frágil.

<a id="term-deserializacion"></a>
### Deserialización
Proceso de convertir un mensaje JSON recibido en estructuras internas del firmware.
Es clave para mapear parámetros remotos a `Params` y validarlos por tipo.
Ejemplo: `{ "state": "ON" }` se convierte en claves/valores consumibles por `CommandExecuteFn`.

<a id="term-discovery-udp"></a>
### Discovery UDP
Mecanismo de anuncio en red local para que el motor detecte dispositivos sin configuración manual.
El dispositivo emite su identidad y endpoint de comunicación periódicamente.
Ejemplo: broadcast con `deviceId`, `name`, `ip`, `webhook_port` y `webhook_path`.

<a id="term-dsl"></a>
### DSL (Domain-Specific Language)
Lenguaje diseñado para un dominio concreto.
En Bunny, el JSON se usa como DSL para describir capacidades y mensajes operativos.
Permite que el motor de procesos razone sin depender de código nativo del ESP32.

<a id="term-edge-device"></a>
### Edge device
Dispositivo que ejecuta cerca del mundo físico (sensores/actuadores).
En Bunny, el edge device es el ESP32 que realiza acciones y lecturas reales.
Su rol no es decidir reglas complejas, sino exponer y ejecutar capacidades.

<a id="term-emit-hook"></a>
### Emit hook
Callback opcional ligado a un evento al momento de emitirlo.
Sirve para efectos locales de hardware sin introducir lógica de negocio.
Ejemplo: parpadear un LED al emitir `motion_detected`.

<a id="term-esp-idf"></a>
### ESP-IDF
SDK oficial de Espressif para construir firmware en ESP32.
Provee toolchain, RTOS, drivers, red, build system y utilidades de flash/monitor.
Bunny se integra sobre esta base como componente del proyecto.

<a id="term-event"></a>
### Event
Capacidad para reportar que ocurrió un hecho relevante.
Generalmente se dispara desde interrupciones o tareas de monitoreo.
Ejemplo: `motion_detected` al superar umbral de un PIR.

<a id="term-execute-hook"></a>
### Execute hook
Función callback que implementa la ejecución física de un comando.
Recibe parámetros ya parseados y ejecuta operaciones de hardware.
Ejemplo: escribir GPIO, PWM o I2C en respuesta a `setFanSpeed`.

<a id="term-firmware"></a>
### Firmware
Software embebido compilado y cargado en el microcontrolador.
En Bunny, el firmware contiene capacidades, hooks y transporte; no reglas de negocio complejas.
Se actualiza con build + flash del proyecto ESP-IDF.

<a id="term-fluent-api"></a>
### Fluent API
Estilo de API que encadena métodos para construir definiciones legibles y consistentes.
En Bunny acelera onboarding y hace más clara la intención del desarrollador.
Ejemplo: `Bunny.sensor("temperature").description(...).returns(NUMBER).build(...)`.

<a id="term-getter-setter-estado"></a>
### Getter/Setter de estado
Callbacks para exponer lectura y escritura de un estado interno.
Permiten que el motor de procesos consulte o actualice valores persistentes del dispositivo.
Ejemplo: `fanState` con getter que devuelve "ON/OFF" y setter que actualiza la variable.

<a id="term-hook"></a>
### Hook
Punto de extensión donde se conecta lógica de hardware concreta.
Bunny define hooks por tipo de capacidad (read, execute, emit, getter/setter).
Su función es ejecutar hardware, no implementar reglas de negocio.

<a id="term-icapability"></a>
### ICapability
Interfaz base que estandariza todas las capacidades del framework.
Garantiza que cada capacidad tenga identidad, metadata y serialización consistente.
Permite al Registry tratarlas de forma polimórfica.

<a id="term-json"></a>
### JSON
Formato de texto estructurado usado para mensajería entre device y motor de procesos.
Bunny lo usa para manifest de capacidades, comandos entrantes y eventos salientes.
Su ventaja es interoperabilidad y fácil inspección en logs.

<a id="term-llm"></a>
### LLM (Large Language Model)
Modelo de lenguaje capaz de comprender texto y estructuras semánticas.
En Bunny puede asistir en integración o generación de procesos si la metadata es clara y completa.
Sin metadata precisa, el LLM tiende a inferir mal parámetros o comportamientos.

<a id="term-logica-negocio"></a>
### Lógica de negocio
Conjunto de decisiones del dominio: reglas, condiciones, transiciones y prioridades.
En esta arquitectura debe residir fuera del firmware, dentro del motor de procesos.
Ejemplo: "si temperatura > 30 y horario nocturno, activar ventilación".

<a id="term-manifest-capacidades"></a>
### Manifest de capacidades
Documento JSON que describe todo lo que el dispositivo expone.
Es la base de descubrimiento e integración automática del motor de procesos.
Incluye categorías como `sensors`, `commands`, `events` y `states`.

<a id="term-metadata-semantica"></a>
### Metadata semántica
Descripción estructurada que explica cómo usar correctamente una capacidad.
Debe incluir intención, parámetros, retorno y contexto para validación y automatización.
Campos típicos: `description`, `params`, `returns`, `tags`, `affects`, `example`.

<a id="term-mock-runtime"></a>
### Mock runtime
Entorno de simulación que replica la ejecución del runtime sin depender de placa física.
Permite pruebas tempranas, demos y depuración rápida de contratos y mensajes.
Reduce tiempos de desarrollo cuando el hardware no está disponible.

<a id="term-motor-procesos"></a>
### Motor de procesos
Sistema externo que orquesta procesos, reglas y transiciones del dominio.
No se limita a ser un backend CRUD; su función principal es decidir comportamiento operativo.
Consume capacidades del dispositivo y emite instrucciones declarativas hacia el edge.

<a id="term-paramdef"></a>
### ParamDef
Estructura que describe un parámetro de entrada de forma explícita.
Define nombre, tipo, descripción y obligatoriedad para validación consistente.
Es parte central de la metadata de comandos y eventos.

<a id="term-read-hook"></a>
### Read hook
Callback de lectura asociado a un sensor.
Se ejecuta cuando el runtime o motor solicita el valor actual.
Ejemplo: leer ADC de temperatura y devolver un `double`.

<a id="term-registry"></a>
### Registry
Componente que almacena y organiza todas las capacidades registradas en memoria.
Permite lookup por nombre/tipo y construcción del manifest JSON.
Es pieza clave para despacho de comandos y exposición de catálogo del dispositivo.

<a id="term-runtime"></a>
### Runtime
Ciclo operativo del firmware durante la ejecución continua del dispositivo.
Coordina recepción de mensajes, validación, despacho y ejecución de hooks.
En Bunny debe mantenerse enfocado en operación técnica, no en reglas de negocio.

<a id="term-sensor"></a>
### Sensor
Capacidad de lectura que produce un valor medible del entorno o hardware.
Suele exponer unidades claras y tipo de retorno definido en metadata.
Ejemplos: temperatura, humedad, corriente, batería.

<a id="term-separacion-responsabilidades"></a>
### Separación de responsabilidades (edge vs motor)
Principio arquitectónico que divide responsabilidades por capa.
El edge ejecuta hardware y el motor de procesos decide flujos y reglas del dominio.
Esta separación reduce acoplamiento y mejora escalabilidad y mantenibilidad.

<a id="term-serializacion"></a>
### Serialización
Proceso de transformar estructuras internas a formato JSON transportable.
Se utiliza para publicar capacidades, respuestas y eventos.
Debe preservar contrato y tipos para evitar errores de interpretación remota.

<a id="term-singleton"></a>
### Singleton
Patrón que garantiza una única instancia compartida en toda la aplicación.
En Bunny simplifica acceso global a componentes como el Registry.
Debe usarse con cuidado para evitar dependencias globales difíciles de testear.

<a id="term-state"></a>
### State
Capacidad que almacena un valor interno persistente o semipersistente.
Puede leerse o actualizarse desde el motor de procesos según el contrato definido.
Ejemplo: `fanState`, `operatingMode`, `lastCommand`.

<a id="term-type-safety"></a>
### Type safety (tipado fuerte)
Práctica de definir tipos explícitos para parámetros y retornos.
Disminuye errores en integración, validación y ejecución remota.
En Bunny se apoya en `Type`, `ParamDef` y metadata estructurada.

<a id="term-type-system"></a>
### Type system
Conjunto oficial de tipos permitidos por el SDK para contratos de capacidades.
Uniforma interpretación entre firmware, motor de procesos y herramientas.
Tipos actuales: `NUMBER`, `STRING`, `BOOLEAN`, `OBJECT`, `ARRAY`, `VOID`.

<a id="term-validacion-estructurada"></a>
### Validación estructurada
Proceso de comprobar que un mensaje cumple formato, campos requeridos y tipos esperados.
Evita ejecutar acciones inválidas o incompletas en el dispositivo.
Se basa en metadata semántica y definición de parámetros (`ParamDef`).

<a id="term-webhook"></a>
### Webhook
Punto HTTP del dispositivo para intercambio de mensajes con el motor de procesos.
Puede usarse para recibir comandos, confirmar ejecución o exponer callbacks.
Normalmente se anuncia junto al discovery para conexión automática.

---

Si detectas términos ambiguos o nuevos en el código, agrega su definición aquí para mantener un lenguaje común en todo el equipo.
