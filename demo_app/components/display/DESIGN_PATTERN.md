# 📐 Patrón de Diseño - Display Capabilities en Main

Documento que explica cómo se mantiene el patrón de diseño de Bunny con las capacidades del display registradas en `main()`.

---

## 🎯 Patrón Consistente

En Bunny Framework, **todas las capabilities se registran en `main()`**, no en funciones internas de módulos.

### ✅ Correcto (Patrón Bunny)

```c
void app_main(void) {
    bunny_begin();
    
    // Registro central de capacidades
    register_temperature_sensor();      // Función helper
    register_fan_command();             // Función helper
    bunny::display::init_hardware();    // Inicialización
    
    Bunny.state("displayStatus", ...)   // Declaración aquí
    Bunny.command("fillScreen", ...)    // Declaración aquí
    
    bunny_load_modules();
    bunny_loop();
}
```

**Ventaja:** `main()` es el orquestador central de capacidades.

---

### ❌ Incorrecto (Patrón roto)

```c
void register_display() {
    // Registrar internamente
    Bunny.state("displayStatus", ...);  // ❌ Adentro
    Bunny.command("fillScreen", ...);   // ❌ Adentro
}

void app_main(void) {
    bunny_begin();
    register_display();  // Llamada opaca
    bunny_load_modules();
    bunny_loop();
}
```

**Problema:** `main()` no ve qué se registra, pierde control.

---

## 🏗️ Arquitectura Display (Ahora Correcta)

### display.h (Interfaz pública)

```cpp
namespace bunny::display {

// Hardware setup
void init_hardware();

// Handler functions (para usar en main)
const char* get_display_status();
void set_display_status(const char* value);

const char* get_brightness();
void set_brightness(const char* value);

const char* get_last_text();
void set_last_text(const char* value);

void cmd_fill_screen(const Params& params);
void cmd_draw_text(const Params& params);
void cmd_set_brightness(const Params& params);
void cmd_clear_screen(const Params& params);

}
```

**Qué exporta:**
- Funciones de inicialización hardware
- Getters/setters para estados
- Handlers para comandos

**Qué NO exporta:**
- ~~Función `register_display()`~~

---

### display.cpp (Implementación)

```cpp
namespace bunny::display {

// State variables (encapsuladas)
static const char* g_display_status = "ON";
static int g_brightness = 255;

// Handlers públicos (usados por main)
const char* get_display_status() { 
    return g_display_status; 
}

void set_display_status(const char* value) { 
    // implementación
}

void cmd_fill_screen(const Params& params) { 
    // implementación
}

// ... más handlers

}
```

**Qué hace:**
- Define variables de estado internas
- Implementa handlers que main() usa
- NO registra capabilities

---

### main/bunny_framework.c (Orquestación)

```c
#include "display/display.h"

void app_main(void) {
    bunny_begin();

    // Inicializar hardware
    bunny::display::init_hardware();
    
    // Registrar EXPLÍCITAMENTE cada capability
    Bunny.state("displayStatus", Type::STRING)
        .build(bunny::display::get_display_status, 
               bunny::display::set_display_status);
    
    Bunny.command("fillScreen")
        .param("color", Type::STRING)
        .build(bunny::display::cmd_fill_screen);
    
    // ... más capabilities
    
    bunny_load_modules();
    bunny_loop();
}
```

**Qué hace:**
- MAIN es responsable de registrar todas las capabilities
- Main controla qué se registra y cómo
- Patrón consistente con otros módulos

---

## 📊 Comparación: Display vs Temperature Sensor

### Temperature Sensor (Existente)

**sensor.h:**
```cpp
void register_temperature_sensor();
```

**main:**
```c
register_temperature_sensor();  // Función helper
```

### Display (Nuevo - Consistente)

**display.h:**
```cpp
void init_hardware();
const char* get_display_status();
void set_display_status(const char* value);
// ... handlers
```

**main:**
```c
bunny::display::init_hardware();
Bunny.state("displayStatus", ...)  // Registro explícito
Bunny.command("fillScreen", ...)   // Registro explícito
```

**Ventaja:** Display ahora sigue el MISMO patrón, solo sin la función helper `register_display()` que ocultaba los registros.

---

## ✨ Por qué esto es mejor

### 1. **Transparencia**
Main VE exactamente qué se registra:
```c
Bunny.state("displayStatus", ...)  // Aquí está
Bunny.command("fillScreen", ...)   // Aquí está
```

No hay "magia" oculta en funciones helper.

### 2. **Control**
Si user quiere modificar una capability:
```c
// Fácil: cambiar aquí en main
Bunny.command("fillScreen")
    .param("color", Type::STRING)
    .description("Mi descripción")  // ← User puede modificar
    .build(bunny::display::cmd_fill_screen);
```

No necesita editar la librería.

### 3. **Consistencia**
Todos los módulos siguen el MISMO patrón:
- Librería exporta handlers
- Main registra capabilities
- Nada oculto

### 4. **Debugging**
Si algo no funciona:
```c
// Clear: la capability está aquí registrada
// Clear: este handler se llama
// Clear: este estado se actualiza
```

Fácil de rastrear.

---

## 🔄 Evolución del Patrón

### Fase 1: Descubrimiento
"¿Cómo hago una librería de display?"

❌ Opción incorrecta: Ocultar registration en la librería
```cpp
void register_display() {
    Bunny.state("displayStatus", ...);
}
```

### Fase 2: Patrón correcto
"¡Las capabilities deben estar en main!"

✅ Opción correcta: Handlers públicos, registro en main
```cpp
// display.h
const char* get_display_status();
void set_display_status(const char* value);

// main
Bunny.state("displayStatus", ...)
    .build(get_display_status, set_display_status);
```

### Fase 3: Beneficio
Ahora user ve claramente:
- Qué estados hay
- Qué comandos hay
- Cómo se conectan handlers
- Puede customizar sin tocar librería

---

## 💡 Para Contribuidores

Al crear nuevas librerías:

```cpp
// ❌ DON'T
void register_my_module() {
    Bunny.state(...).build(...);
    Bunny.command(...).build(...);
}

// ✅ DO
// Exporta handlers y dejar que main() registre
const char* get_my_state();
void set_my_state(const char* v);
void cmd_my_command(const Params& p);
```

Mantiene consistencia arquitectónica.

---

## 🎯 Resumen

| Aspecto | Incorrecto | Correcto |
|---------|-----------|----------|
| Dónde se registran | Dentro librería | En main() |
| Qué exporta librería | Función `register_*()` | Handlers, init |
| Control por user | Opaco | Transparente |
| Patrón en Bunny | Inconsistente | Consistente |
| Debugging | Difícil | Fácil |

**El display ahora sigue el patrón correcto.** ✅

---

**Esta es la arquitectura que mantiene Bunny Framework limpio y predecible.** 🚀
