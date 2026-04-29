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
