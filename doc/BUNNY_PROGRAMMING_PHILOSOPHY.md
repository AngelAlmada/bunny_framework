# Guía de Programación en Bunny Framework

Este documento define cómo programar correctamente en Bunny Framework.

Objetivo principal: en Bunny se programan capacidades de hardware, no lógica de negocio en firmware.

**← [Volver al README](README.md)**

## Índice

1. [Principio central](#principio-central)
2. [Modelo mental de Bunny](#modelo-mental-de-bunny)
3. [Qué sí va en firmware](#qué-sí-va-en-firmware)
4. [Qué no va en firmware](#qué-no-va-en-firmware)
5. [Reglas de diseño obligatorias](#reglas-de-diseño-obligatorias)
6. [Patrones recomendados](#patrones-recomendados)
7. [Anti-patrones a evitar](#anti-patrones-a-evitar)
8. [Plantilla para crear capacidades](#plantilla-para-crear-capacidades)
9. [Checklist antes de merge](#checklist-antes-de-merge)

---

## Principio central

Bunny separa dos capas:

- **ESP32 (edge)**: expone y ejecuta capacidades.
- **Motor de procesos**: decide reglas, flujos, condiciones y transiciones.

Frase guía:

"El firmware ejecuta, el motor de procesos decide."

Si tienes una duda sobre dónde implementar algo, usa esta pregunta:

"¿Esto es una decisión de negocio o una acción de hardware?"

- Si es decisión: va en el motor de procesos.
- Si es acción: va en el firmware (como hook de capacidad).

---

## Modelo mental de Bunny

Piensa Bunny como una "API de hardware declarativa":

1. Declaras capacidades (`sensor`, `command`, `event`, `state`).
2. Documentas metadata completa (descripción, params, returns, tags, affects, example).
3. El runtime publica el manifest de capacidades.
4. El motor de procesos usa ese contrato para orquestar comportamiento.

El firmware no contiene orquestación compleja.

---

## Qué sí va en firmware

Estas responsabilidades sí pertenecen al proyecto ESP32:

- Lectura física de sensores.
- Escritura en actuadores (GPIO, PWM, I2C, SPI, UART, etc.).
- Emisión de eventos de hardware.
- Almacenamiento/lectura de estado local técnico.
- Validaciones técnicas de seguridad (rangos físicos, límites eléctricos, formato mínimo).
- Descubrimiento de red, transporte y serialización de mensajes.

Ejemplo correcto:

```cpp
Bunny.command("setFanSpeed")
     .description("Set fan speed percentage")
     .param("speed", NUMBER, "0 to 100")
     .execute([](const bunny::Params& p) {
         int speed = (int)p.get_number("speed");
         if (speed < 0) speed = 0;
         if (speed > 100) speed = 100;
         pwm_set(FAN_PIN, speed);
     });
```

---

## Qué no va en firmware

No debes implementar en firmware:

- Reglas de negocio del dominio.
- Flujos de proceso multi-paso.
- Condiciones contextuales de alto nivel (tiempo, roles, políticas, campañas).
- Priorización de procesos o coordinación de escenarios complejos.
- Evaluación dinámica de strings como código.

Ejemplo incorrecto:

```cpp
Bunny.command("controlClimate")
     .execute([](const bunny::Params& p) {
         if (read_temp() > 28 && is_business_hours() && user_has_plan("pro")) {
             activate_fan();
         }
     });
```

Las condiciones anteriores pertenecen al motor de procesos.

---

## Reglas de diseño obligatorias

1. Toda capacidad debe tener metadata semántica suficiente.
2. Todo parámetro debe tener tipo explícito.
3. Toda acción física debe estar encapsulada en hooks.
4. Toda capacidad debe tener nombre estable y descriptivo.
5. Evita acoplar una capacidad a un caso de negocio específico.
6. No uses estado global sin control de concurrencia si hay múltiples tareas.
7. Si una decisión puede cambiar sin tocar hardware, debe estar fuera del firmware.

---

## Patrones recomendados

### 1) Capability-first

Declara primero el contrato, luego la implementación hardware.

```cpp
Bunny.sensor("temperature")
     .description("Ambient temperature in Celsius")
     .returns(NUMBER)
     .tag("environment")
     .build([]() -> double { return read_temperature_hw(); });
```

### 2) Hardware abstraction

Separa función de hardware de la definición Bunny:

```cpp
static double read_temperature_hw() {
    return 23.5;
}

void register_temperature_sensor() {
    Bunny.sensor("temperature")
         .description("Ambient temperature in Celsius")
         .returns(NUMBER)
         .build([]() -> double { return read_temperature_hw(); });
}
```

### 3) Contratos estables

Evita renombrar capacidades o parámetros sin control de versión.

- `setFanState(state)` es mejor que `doAction(x)`.

### 4) Eventos orientados a hechos

Nombra eventos por lo ocurrido, no por la reacción esperada.

- Bien: `motion_detected`
- Mal: `turn_on_alarm`

---

## Anti-patrones a evitar

- "Comando gigante" que mezcla muchas responsabilidades.
- Parámetros ambiguos como `x`, `value`, `data` sin descripción.
- Estados sin propósito claro.
- Metadata incompleta o genérica.
- Dependencias de negocio en código de periféricos.

---

## Plantilla para crear capacidades

### Sensor

```cpp
Bunny.sensor("sensor_name")
     .description("What this sensor measures")
     .returns(NUMBER)
     .tag("domain")
     .example("Read: 12.3")
     .build([]() -> double {
         return read_sensor_hw();
     });
```

### Command

```cpp
Bunny.command("command_name")
     .description("What hardware action this command performs")
     .param("param", STRING, "Meaning of this param")
     .affects("stateName")
     .execute([](const bunny::Params& p) {
         execute_hw(p);
     });
```

### Event

```cpp
Bunny.event("event_name")
     .description("When this event is emitted")
     .tag("domain")
     .build();
```

### State

```cpp
Bunny.state("state_name", STRING)
     .description("What this state represents")
     .build(
         []() -> const char* { return get_state(); },
         [](const char* v) { set_state(v); }
     );
```

---

## Checklist antes de merge

- [ ] ¿La lógica de negocio quedó fuera del firmware?
- [ ] ¿La capacidad tiene metadata completa y útil?
- [ ] ¿Los nombres son claros y estables?
- [ ] ¿Los parámetros están tipados y descritos?
- [ ] ¿El hook hace solo trabajo de hardware?
- [ ] ¿La validación es técnica y no de negocio?
- [ ] ¿La capacidad se serializa correctamente en el manifest?

---

Documentos relacionados:

- [README.md](README.md)
- [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md)
- [GLOSARIO.md](GLOSARIO.md)
