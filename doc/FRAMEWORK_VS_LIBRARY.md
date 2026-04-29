# ¿Por Qué Bunny es un Framework y No una Librería?

## Definiciones claras

### Librería
Una **librería** es un conjunto de funciones y componentes reutilizables que **tu código **llama cuando lo necesita**. El control flujo sigue siendo responsabilidad del código que la usa.

**Ejemplo:** Una librería HTTP proporciona funciones como `http_get()`, `http_post()`, pero tu código decide cuándo llamarlas y cómo integrarlas.

### Framework
Un **framework** proporciona una **estructura base, flujo de control y patrones predefinidos** que **tu código sigue obligatoriamente**. El framework controla el flujo; tu código se encaja en los puntos de extensión predefinidos.

**Ejemplo:** Django (Python) define cómo funcionan modelos, vistas, controladores. Tu código se adapta a esa estructura; no lo opuesto.

---

## Por qué Bunny es un Framework

### 1. **Control del flujo de ejecución**

Bunny **dicta el flujo principal de la aplicación**:

```c
// Tu código NO decide el flujo principal
// El framework lo hace:

void app_main() {
  bunny_begin();              // 1. Framework inicializa
  bunny_load_modules();       // 2. Framework carga módulos
  while(1) {
    bunny_loop();             // 3. Framework controla el loop
  }
}
```

**Comparar con una librería:**
```c
// Si fuera librería, tu código controlaría el flujo:
while(1) {
  if(network_connected()) {
    send_data();
    receive_commands();
  }
  update_sensors();
  // Todo bajo TU control
}
```

Bunny **no te deja opción**: debes llamar `bunny_begin()`, `bunny_load_modules()` y `bunny_loop()` en ese orden. Es prescriptivo, no flexible.

### 2. **Estructura mandatoria de directorios y módulos**

El framework **exige una estructura específica**:

```
main/
├── sensors/      ← Todos los sensores van aquí
├── commands/     ← Todos los comandos van aquí
├── events/       ← Todos los eventos van aquí
└── states/       ← Todos los estados van aquí
```

**Una librería no lo haría.** Una librería podría ser usada en cualquier estructura de carpetas. Bunny define dónde va cada cosa.

### 3. **Puntos de extensión predefinidos**

Bunny define **cómo extender la funcionalidad** a través de patrones obligatorios (builders):

```cpp
// El framework dicta QUÉ debes hacer (no tú decides)
Bunny.sensor("temperature")
     .description("...")
     .returns(NUMBER)
     .build([]() {
       return read_temperature();
     });
```

**Una librería ofrecería:**
```cpp
// "Aquí hay funciones, úsalas como necesites"
register_sensor_manually("temperature", &read_temperature);
```

Bunny **fuerza* un patrón fluent builder específico. No puedes hacerlo de otra forma.

### 4. **Infraestructura integral prescriptiva**

Bunny proporciona y controla:

- **Discovery UDP** — Cómo el dispositivo se anuncia
- **Protocolo de comunicación (BCP)** — Cómo se comunica con motor de procesos
- **WebSocket** — Conexión persistente
- **Serialización JSON** — Formato obligatorio
- **Validación de tipos** — Sistema de tipos fijo
- **Registro de capacidades** — Registry central y presencible
- **Ciclo de ejecución** — Cómo se despachan comandos y sensores

Una librería ofrecería **componentes** que TÚ integras. Bunny **integra automáticamente** si sigues su estructura.

### 5. **Separación de responsabilidades no negociable**

Bunny **impone arquitectura**:

```
┌─────────────────────────────────────────────┐
│ Tu código: SOLO capacidades de hardware     │
├─────────────────────────────────────────────┤
│ Framework: Comunicación, discovery, routing │
├─────────────────────────────────────────────┤
│ Motor de procesos (externo): Toda lógica    │
└─────────────────────────────────────────────┘
```

**No puedes mezclar responsabilidades.** El framework lo impide by design.

Una librería sería: "Aquí tienes funciones; úsalas como quieras."
Bunny es: "Tu código entra aquí; lógica va allá; punto."

### 6. **Metadata obligatoria**

Bunny **exige que documentes cada capacidad**:

```cpp
Bunny.sensor("temperature")
  .description("...")        // Obligatorio
  .returns(NUMBER)           // Obligatorio
  .tag("environment")        // Recomendado
  .build([](){ ... });       // Required
```

**Una librería no lo pediría.** Bunny lo requiere porque es parte de su contrato y arquitectura.

### 7. **Filosofía capabilities-first**

Bunny **define cómo pensar el código**:

- **No escribas`if/else` de lógica de negocio** en ESP32.
- **No tomes decisiones** que corresponden al motor.
- **Declara capacidades**, no implementes workflows.

Esto no es opcional. Es la filosofía del framework. Una librería no tendría filosofía; solo componentes.

---

## Comparación visual

| Aspecto | Librería | Framework |
|---------|----------|-----------|
| **Control de flujo** | Tu código lo controla | El framework lo controla |
| **Estructura** | Flexible, cualquier carpeta | Estructura prescriptiva |
| **Puntos de extensión** | Flexible, múltiples formas | Patrones únicos (builders) |
| **Integración** | Explícita, bajo tu control | Automática, sigue estructura |
| **Responsabilidades** | Tú las defines | El framework las impone |
| **Ejemplo** | `#include <http.h>` y usas libremente | `bunny_begin()` → estructura obligatoria |

---

## Conclusión

**Bunny es un framework porque:**

1. **Dicta el flujo principal** de la aplicación (no al revés).
2. **Impone estructura** de directorios y módulos.
3. **Define patrones únicos** de extensión (builders).
4. **Proporciona infraestructura integral** (discovery, WebSocket, protocolo, registry).
5. **Separa responsabilidades** de forma no negociable.
6. **Exige metadata** como parte del contrato.
7. **Define una filosofía** (capabilities-first, sin lógica en firmware).

**Si fuera una librería:**
- Podrías ignorar directorios predefinidos.
- Podrías integrar componentes manualmente.
- Podrías mezclar lógica de negocio con hardware.
- Podrías registrar capacidades de múltiples formas.
- **No habría restricción arquitectónica.**

Pero Bunny **sí las hay**. Eso lo hace un framework.

---

## Ventajas de que Bunny sea un Framework

1. **Predecibilidad:** Todo código Bunny tiene estructura similar.
2. **Seguridad arquitectónica:** El framework impide anti-patrones.
3. **Menos errores:** La estructura reduce decisiones equivocadas.
4. **LLM-friendly:** La estructura clara facilita generación y análisis automático.
5. **Escalabilidad:** Nuevos módulos se integran sin fricción.
6. **Mantenibilidad:** Todo devloper sabe dónde buscar cada cosa.
---

## Arquitectura y Patrones de Diseño

### Arquitectura General: Hexagonal (Ports & Adapters) + Event-Driven

Bunny implementa una **arquitectura hexagonal** (puertos y adaptadores) combinada con **event-driven**:

```
┌─────────────────────────────────────────────────────────────┐
│                      MOTOR DE PROCESOS                      │
│                      (Externo, decide)                       │
└────────────────────────┬────────────────────────────────────┘
                         │ JSON over WebSocket (BCP)
┌────────────────────────▼────────────────────────────────────┐
│          BUNNY FRAMEWORK (ESP32 + Firmware)                 │
│  ┌────────────────────────────────────────────────────────┐ │
│  │ NÚCLEO (Hardware-Agnostic)                             │ │
│  │ - Registry, Builders, Metadata, Protocol               │ │
│  └────────────────────────────────────────────────────────┘ │
│         ▲                    ▲                    ▲           │
│  ┌──────┴─────┐      ┌──────┴─────┐      ┌────┴───────┐   │
│  │  SENSORES  │      │  COMANDOS  │      │   EVENTOS  │   │
│  └────┬─────┬┘      └──────┬─────┘      └────┬──┬────┘   │
│  ┌────▼─┐ ┌─▼────┐   ┌────▼───┐      ┌──────▼──▼─────┐  │
│  │ GPIO │ │ I2C  │   │Actuador│      │ Estado Local  │  │
│  └──────┘ └──────┘   └────────┘      └───────────────┘  │
└─────────────────────────────────────────────────────────┘
```

**¿Por qué hexagonal?** Puertos (entrada/salida), Adaptadores (hardware específico), Núcleo (agnóstico a hardware).

### Patrones de Diseño Identificados

#### 1. **Builder Fluent Pattern** (patrón dominante)

**Qué resuelve:** declarar capacidades complejas de forma legible y encadenada, evitando constructores largos y frágiles.

**Dónde se ve en Bunny:** en `SensorBuilder`, `CommandBuilder`, `EventBuilder` y `StateBuilder`, cada método devuelve `*this` para seguir encadenando.

**Ejemplo real (sensor):**

```cpp
Bunny.sensor("temperature")
   .description("Ambient temperature in degrees Celsius")
   .returns(NUMBER)
   .tag("environment")
   .tag("climate")
   .example("Read: 23.5")
   .build([]() -> double {
     return read_temperature_hw();
   });
```

**Valor práctico:**
- Hace visible el contrato de la capacidad en un solo bloque.
- Reduce errores de inicialización parcial.
- Obliga una fase de cierre (`build()`/`execute()`) donde se registra oficialmente.

#### 2. **Registry Singleton Pattern**

**Qué resuelve:** mantener un punto único de verdad para todas las capacidades cargadas.

**Dónde se ve en Bunny:** `Registry::instance()` devuelve una instancia estática y centralizada.

**Ejemplo real:**

```cpp
Registry& Registry::instance() {
  static Registry s_instance;
  return s_instance;
}

bool Registry::register_capability(ICapability* cap) {
  if (!cap || _count >= MAX_CAPABILITIES) return false;
  _caps[_count++] = cap;
  return true;
}
```

**Valor práctico:**
- Todas las altas pasan por el mismo lugar.
- `serialize_capabilities()` produce un manifiesto consistente.
- `find(name, kind)` permite despacho determinista de comandos/eventos.

#### 3. **Abstract Factory + Polymorphism**

**Qué resuelve:** tratar sensores, comandos, eventos y estados bajo una interfaz común.

**Dónde se ve en Bunny:** `ICapability` define el contrato; cada tipo concreta su comportamiento (`serialize`, `kind`, etc.).

**Ejemplo real:**

```cpp
class ICapability {
public:
  virtual ~ICapability() = default;
  virtual CapabilityKind  kind()     const = 0;
  virtual const char*     name()     const = 0;
  virtual const Metadata& metadata() const = 0;
  virtual size_t serialize(char* buf, size_t len) const = 0;
};
```

**Valor práctico:**
- El registry almacena punteros a `ICapability` sin conocer detalles internos.
- El sistema puede iterar y serializar todas las capacidades de forma uniforme.
- Agregar un nuevo tipo de capacidad no rompe el pipeline existente.

#### 4. **Factory Method en los Builders**

**Qué resuelve:** encapsular la creación de objetos concretos y su registro automático.

**Dónde se ve en Bunny:** `build()` y `execute()` crean la capability concreta y la registran.

**Ejemplo real:**

```cpp
SensorCapability* SensorBuilder::build(SensorReadFn read_fn) {
  auto* cap = new SensorCapability(_name, _meta, std::move(read_fn));
  Registry::instance().register_capability(cap);
  return cap;
}
```

**Valor práctico:**
- El usuario del SDK no necesita hacer `new` ni tocar el registry manualmente.
- Se evita duplicar lógica de creación/registro en cada módulo.

#### 5. **Strategy Pattern** (hooks por lambda)

**Qué resuelve:** desacoplar comportamiento variable (leer, ejecutar, emitir, set/get state) del core del framework.

**Dónde se ve en Bunny:** cada capability recibe funciones (`SensorReadFn`, `CommandExecuteFn`, etc.) y las ejecuta en runtime.

**Ejemplo real (command):**

```cpp
Bunny.command("setFanState")
   .description("Turn the fan relay ON or OFF")
   .param("state", STRING, "Target state: ON or OFF")
   .execute([](const bunny::Params& p) {
     const char* state = p.get_string("state");
     set_fan_hw(strcmp(state, "ON") == 0);
   });
```

**Valor práctico:**
- El framework no sabe cómo leer tu sensor ni cómo escribir tu GPIO.
- El hardware cambia; el núcleo de Bunny no.
- Las estrategias son reemplazables para mocks/tests.

#### 6. **Observer Pattern** (orientado a eventos)

**Qué resuelve:** publicar sucesos de dispositivo sin acoplar productor y consumidor.

**Dónde se ve en Bunny:** evento declarado en firmware + emisión (`Bunny.emit`) + consumo en motor por WebSocket/BCP.

**Ejemplo real (declaración de evento):**

```cpp
Bunny.event("motion_detected")
   .description("Triggered when the PIR sensor detects movement")
   .tag("sensor")
   .tag("security")
   .build([]() {
     blink_indicator_hw();
   });
```

**Valor práctico:**
- El firmware emite hechos, no decide procesos de negocio.
- El motor reacciona de forma asíncrona y orquesta flujos complejos.

#### 7. **Template Method Pattern**

**Qué resuelve:** forzar un ciclo de vida estable, repetible y seguro.

**Dónde se ve en Bunny:** secuencia fija de arranque y operación (`begin -> load_modules -> loop`).

**Ejemplo real (`app_main`):**

```c
void app_main(void)
{
  bunny_begin();

  register_temperature_sensor();
  register_fan_command();
  register_motion_event();
  register_fan_state();

  bunny_load_modules();
  bunny_loop();
}
```

**Valor práctico:**
- Evita arrancar red/protocolo antes de configuración.
- Estandariza el boot de cualquier dispositivo Bunny.
- Reduce clases de bugs por orden incorrecto.

#### 8. **Declarative Programming Model**

**Qué resuelve:** expresar capacidades como contrato semántico, no como lógica procedural dispersa.

**Dónde se ve en Bunny:** bloques de declaración con metadata (`description`, `params`, `returns`, `tags`, `example`).

**Ejemplo conceptual:**

```cpp
// Declarativo: describe la capacidad
Bunny.sensor("temperature")
  .description("Ambient temperature in degrees Celsius")
  .returns(NUMBER)
  .tag("environment")
  .build(read_temperature);

// Imperativo tradicional: crear objeto, setear campos, registrarlo manualmente
```

**Valor práctico:**
- Contrato más claro para humanos, tooling y LLMs.
- Facilita serialización e introspección automática.
- Menor fricción para mantener doc y código alineados.

---

### Separación de Responsabilidades (La Clave)

| Componente | Responsabilidad | Hardware | Lógica |
|-----------|-----------------|----------|---------|
| **Firmware** | Sensores/actuadores | ✅ SÍ | ❌ NO |
| **Bunny** | Comunicación/protocolo | ❌ NO | ❌ NO |
| **Motor** | Reglas/flujos/decisiones | ❌ NO | ✅ SÍ |

Esto no es sugerencia; es arquitectura **inmutable**.

---

### Flujo Event-Driven (No Polling)

```
Sensor → JSON → WebSocket → Motor → Decide → Comando → JSON → Actuador
```

Reactividad asíncrona, no bloqueante, economía de batería.

---

### Metadata Obligatoria = Contrato Ejecutable

```cpp
Bunny.sensor("temperature")
  .description("Room temperature in Celsius")
  .returns(NUMBER)
  .tag("environment")
  .affects("thermostat")
  .example(23.5)
  .build([](){ return read_temp(); });
```

Motor entiende sin inspeccionar código. Validación automática. Generación segura.

---

### Escalabilidad por Arquitectura

Bunny escala por **arquitectura limpia**, no por "más código":

- Límites estáticos: 32 capabilities, 8 parámetros (suficiente para IoT)
- Decisiones complejas van al motor (con recursos)
- Resultado: Firmware simple + Motor inteligente

**Comparación:**

```cpp
// CON LIBRERÍA MONOLÍTICA: crece todo en firmware
if(command == "on") { ... }
else if(command == "off") { ... }
else if(command == "schedule") { ... }
// Firmware = spaghetti code

// CON BUNNY: firmware minimalista
Bunny.command("on").execute([](const Params& p) {
  digitalWrite(LED, HIGH);  // Una línea
});
// Lógica va al motor
```

---

## Justificación: Por Qué Esta Arquitectura

### Para ESP32

1. RAM limitada (520 KB): Arquitectura modular que cabe
2. Sin OS: Event-driven evita threads complejos
3. Hardware heterogéneo: Puertos/adaptadores para GPIO, I2C, SPI
4. Comunicación remota: Motor es computadora principal

### Para Desarrollo

1. Firmware team: Solo hardware, sigue plantilla
2. Backend team: Solo lógica, consume vía JSON
3. DevOps: Integración sin tocar código

### Para IA/LLMs

1. Metadata: LLM entiende sin código
2. Patrones predecibles: Code generation segura
3. Contrato JSON: Comandos válidos asegurados

---

## Conclusión: Un Framework Bien Diseñado

Bunny es un **sistema arquitectónico coherente** que:

- ✅ Implementa patrones probados (Builder, Factory, Strategy, Observer, Template Method)
- ✅ Separa responsabilidades de forma inmutable (Hexagonal)
- ✅ Facilita comunicación asíncrona (Event-driven)
- ✅ Exige contrato ejecutable (Metadata)
- ✅ Escala por arquitectura, no por complejidad
- ✅ Habilita colaboración (Teams, LLMs, tooling)
- ✅ Imposibilita anti-patrones

**Eso es lo que hace un buen framework.**
