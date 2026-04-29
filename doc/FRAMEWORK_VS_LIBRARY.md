# Por qué Bunny es un Framework y no una Librería

## Tesis central

Bunny se comporta como un framework porque define el ciclo de vida de la aplicación, establece puntos de extensión concretos y organiza la solución alrededor de un contrato de capacidades.

Dicho simple:
- Con una librería, tu aplicación llama funciones cuando quiere.
- Con Bunny, tu aplicación entra en una estructura ya definida y aporta piezas (sensores, comandos, eventos, estados) en lugares específicos.

Una analogía rápida:
- Librería: caja de herramientas.
- Framework: línea de ensamblaje con estaciones definidas.

No significa que Bunny controle todo de forma rígida en cada detalle, pero sí marca el esqueleto principal del sistema y cómo se integra el código de dominio de hardware.

---

## Evidencias prácticas

### 1. Inversion of Control (IoC)

El criterio más fuerte para distinguir framework vs librería es la Inversion of Control:

- Librería: tu código decide cuándo y cómo ejecutar cada pieza.
- Framework: el framework decide el flujo general y tu código entra en hooks o puntos de extensión.

En Bunny, el flujo típico es:

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

Aquí el firmware de usuario registra capacidades, pero el framework controla el arranque, red, ciclo continuo y despacho. Esa es la inversión de control en la práctica.

### 2. Extensión guiada por contrato

Bunny no te pide "usar funciones sueltas" sino declarar capacidades con metadata:

```cpp
Bunny.command("setFanState")
     .description("Turn the fan relay ON or OFF")
     .param("state", STRING, "Target state: ON or OFF")
     .affects("fanState")
     .execute([](const bunny::Params& p) {
       const char* state = p.get_string("state");
       set_fan_hw(strcmp(state, "ON") == 0);
     });
```

El valor no es solo sintáctico: ese bloque define un contrato entendible por runtime, motor de procesos y tooling.

### 3. Infraestructura integrada

Bunny aporta piezas transversales que normalmente en una librería tendrías que cablear manualmente:

- Registro central de capacidades
- Serialización de manifiesto
- Comunicación con motor
- Flujo de runtime para lectura, ejecución y emisión

Esto reduce integración ad hoc y favorece consistencia entre dispositivos.

### 4. Separación de responsabilidades

Bunny orienta a separar:
- Firmware: interacción con hardware
- Motor externo: decisiones de negocio
- Framework: contrato, transporte y orquestación técnica

Esta separación no es absoluta en términos teóricos (siempre puedes romperla en código), pero sí es el camino principal que el diseño promueve y protege.

---

## Arquitectura (visión clara antes de patrones)

### Resumen arquitectónico

Bunny combina una estructura tipo puertos/adaptadores con comunicación orientada a eventos.

- Núcleo: capacidades, metadata, registro, serialización.
- Adaptadores: sensores/actuadores concretos de ESP32.
- Integración externa: motor de procesos vía protocolo de red.

```text
Motor de procesos
      |
      | JSON / WebSocket / protocolo
      v
Bunny runtime + Registry + Metadata
      |
      +--> Sensores (lectura)
      +--> Comandos (ejecución)
      +--> Eventos (emisión)
      +--> Estados (acceso)
      |
      v
Hardware ESP32
```

### Qué aporta esta arquitectura

- Mantiene el firmware pequeño y enfocado.
- Hace más predecible la integración con backend/motor.
- Facilita documentar capacidades de forma consistente.
- Permite crecer agregando módulos sin reescribir el núcleo.

---

## Patrones de diseño (qué aplica y qué no sobreatribuir)

Esta sección evita "poner etiqueta por poner etiqueta". En cada caso: problema, aplicación en Bunny y matiz.

### 1. Fluent Builder

Problema que resuelve:
- Declarar objetos con varios campos opcionales sin constructores enormes.

Cómo lo aplica Bunny:
- SensorBuilder, CommandBuilder, EventBuilder y StateBuilder encadenan metadata y terminan en build/execute.

```cpp
Bunny.sensor("temperature")
     .description("Ambient temperature in degrees Celsius")
     .returns(NUMBER)
     .tag("environment")
     .build([]() -> double { return read_temperature_hw(); });
```

Diferencia frente a "simple setters":
- El flujo de construcción queda guiado y legible en un solo bloque.

### 2. Registry + Singleton

Problema que resuelve:
- Tener una fuente única para consultar y serializar capacidades registradas.

Cómo lo aplica Bunny:
- Registry central con instancia única compartida.

```cpp
Registry& Registry::instance() {
  static Registry s_instance;
  return s_instance;
}
```

```cpp
bool Registry::register_capability(ICapability* cap) {
  if (!cap || _count >= MAX_CAPABILITIES) return false;
  _caps[_count++] = cap;
  return true;
}
```

Aclaración importante:
- "Registry" y "Singleton" no son lo mismo.
- Registry es el rol (catálogo central).
- Singleton es una forma de ciclo de vida/acceso (instancia única).

### 3. Factory Method (sí aplica)

Problema que resuelve:
- Encapsular creación de objetos concretos para no repetir new + wiring en cada módulo.

Cómo lo aplica Bunny:
- build/execute crean capacidades concretas y las registran.

```cpp
SensorCapability* SensorBuilder::build(SensorReadFn read_fn) {
  auto* cap = new SensorCapability(_name, _meta, std::move(read_fn));
  Registry::instance().register_capability(cap);
  return cap;
}
```

Diferencia frente a Abstract Factory:
- Aquí hablamos de métodos de creación concretos por builder.
- No hay una familia completa de fábricas intercambiables en runtime.

### 4. Polimorfismo por interfaz (sí aplica)

Problema que resuelve:
- Tratar capacidades heterogéneas con una API común.

Cómo lo aplica Bunny:
- ICapability define contrato común para kind, name, metadata y serialize.

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

Por qué es útil:
- El registry guarda punteros homogéneos y procesa todos los tipos sin if gigantes por clase concreta.

### 5. Strategy (hooks de comportamiento)

Problema que resuelve:
- Permitir que la lógica variable de hardware cambie sin tocar el núcleo.

Cómo lo aplica Bunny:
- Lambdas o functors inyectados como read/execute/emit/get/set.

```cpp
Bunny.command("setFanState")
     .param("state", STRING, "Target state: ON or OFF")
     .execute([](const bunny::Params& p) {
       const char* state = p.get_string("state");
       set_fan_hw(strcmp(state, "ON") == 0);
     });
```

Diferencia frente a callbacks sueltos:
- Aquí los hooks viven dentro de una capability formal con metadata y ciclo de vida de framework.

### 6. Template Method (aplica parcialmente)

Problema que resuelve:
- Mantener una secuencia estable de inicialización y ejecución.

Cómo lo aplica Bunny:
- Orden esperado begin -> registro de módulos -> load_modules -> loop.

Matiz para no sobreatribuir:
- No es el Template Method clásico en jerarquía OO con virtual hooks de una subclase.
- Es más bien una plantilla de ciclo de vida a nivel de framework/API.

Aun así, el efecto práctico es el mismo: ordenar y estandarizar el boot/runtime.

### 7. Observer / Event-driven (aplica a nivel de sistema)

Problema que resuelve:
- Notificar eventos sin acoplar fuerte al consumidor.

Cómo lo aplica Bunny:
- Firmware emite eventos; motor externo los consume y decide acciones.

```cpp
Bunny.event("motion_detected")
     .description("Triggered when the PIR sensor detects movement")
     .build([]() {
       blink_indicator_hw();
     });
```

Matiz:
- No hay un bus Observer clásico completo en memoria local con lista explícita de observers de objetos C++.
- El patrón se ve más claramente en la interacción distribuida dispositivo <-> motor.

### 8. Abstract Factory (mejor tratarlo con cuidado)

Evaluación:
- En el estado actual del código, es discutible llamarlo Abstract Factory de forma estricta.
- Sí existe polimorfismo y creación de tipos concretos, pero no se ve una familia de fábricas abstractas intercambiables como pieza central.

Recomendación documental:
- Priorizar la etiqueta Factory Method + Polimorfismo.
- Mencionar Abstract Factory solo como posible evolución de diseño, no como descripción principal actual.

---

## Cierre

Bunny es framework no porque "tenga muchos archivos", sino porque organiza la solución alrededor de:

1. Inversion of Control
2. Ciclo de vida y puntos de extensión claros
3. Contrato de capacidades con metadata
4. Infraestructura compartida para registro, serialización e integración

Si quieres evaluar cualquier nueva feature, esta regla ayuda:
- Si solo agrega utilidades sueltas, se parece a librería.
- Si se integra al ciclo de vida, al contrato y al runtime común, fortalece el carácter de framework.
