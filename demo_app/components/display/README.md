# 🎨 ST7789 2.8" TFT Display Library

Módulo listo para usar para controlar una pantalla ST7789 2.8" en Bunny Framework.

---

## 📋 Características

**Comandos (Commands):**
- `fillScreen(color)` - Llenar pantalla con color
- `drawText(text, x, y)` - Dibujar texto
- `setBrightness(level)` - Control de brillo (0-255)
- `clearScreen()` - Limpiar pantalla

**Estados (States):**
- `displayStatus` - Estado ON/OFF
- `brightness` - Nivel de brillo actual
- `lastText` - Último texto dibujado

---

## 🔧 Instalación

### 1. Copiar carpeta
```bash
cp -r libraries/display_st7789 components/display
```

### 2. Configurar pines (IMPORTANTE)

Editar `components/display/display_config.h` con tus pines GPIO:

```cpp
#define DISPLAY_PIN_MOSI    23      // Tu pin MOSI
#define DISPLAY_PIN_SCLK    18      // Tu pin SCLK
#define DISPLAY_PIN_CS      5       // Tu pin CS
#define DISPLAY_PIN_DC      2       // Tu pin DC
#define DISPLAY_PIN_RST     4       // Tu pin RST
#define DISPLAY_PIN_MISO    19      // Tu pin MISO
#define DISPLAY_PIN_BL      32      // Tu pin Backlight
```

---

## 🔧 Instalación

### 1. Copiar librería
```bash
cp -r libraries/display_st7789 components/display
```

### 2. Configurar pines (IMPORTANTE)

Editar `components/display/display_config.h`:

```cpp
#define DISPLAY_PIN_MOSI    23      // Tu pin MOSI
#define DISPLAY_PIN_SCLK    18      // Tu pin SCLK
#define DISPLAY_PIN_CS      5       // Tu pin CS
#define DISPLAY_PIN_DC      2       // Tu pin DC
#define DISPLAY_PIN_RST     4       // Tu pin RST
#define DISPLAY_PIN_MISO    19      // Tu pin MISO
#define DISPLAY_PIN_BL      32      // Tu pin Backlight
```

### 3. Ver ejemplo en main

Dos archivos de ejemplo con las capacidades:

**Comandos:** `main/commands/display_command.cpp`
```cpp
void register_display_commands() {
    Bunny.command("fillScreen")
        .param("color", Type::STRING)
        .build(bunny::display::cmd_fill_screen);
    // ... más comandos
}
```

**Estados:** `main/states/display_state.cpp`
```cpp
void register_display_states() {
    Bunny.state("displayStatus", Type::STRING)
        .build(bunny::display::get_display_status, ...);
    // ... más estados
}
```

**Orquestación en main:** `main/bunny_framework.c`
```c
#include "display/display.h"
#include "commands/display_command.h"
#include "states/display_state.h"

void app_main(void) {
    bunny_begin();
    
    bunny::display::init_hardware();
    register_temperature_sensor();
    register_display_commands();
    register_display_states();
    
    bunny_load_modules();
    bunny_loop();
}
```

### 4. Compilar

```bash
bunny flash
```

---

## 💡 Uso

### Desde Motor de Procesos (JSON)

**Llenar pantalla de azul:**
```json
{
  "type": "command",
  "command": "fillScreen",
  "params": {
    "color": "blue"
  }
}
```

**Dibujar texto:**
```json
{
  "type": "command",
  "command": "drawText",
  "params": {
    "text": "Temperatura: 25°C",
    "x": 10,
    "y": 50
  }
}
```

**Cambiar brillo:**
```json
{
  "type": "command",
  "command": "setBrightness",
  "params": {
    "level": 128
  }
}
```

**Limpiar pantalla:**
```json
{
  "type": "command",
  "command": "clearScreen"
}
```

### Consultar Estados

```json
{
  "type": "state_get",
  "state": "brightness"
}
```

---

## 🎨 Colores disponibles

- `black` - Negro
- `white` - Blanco
- `red` - Rojo
- `green` - Verde
- `blue` - Azul
- `yellow` - Amarillo
- `cyan` - Cian
- `magenta` - Magenta

---

## 📌 Notas importantes

1. **Configurar pines ANTES de compilar**
   - Los pines se definen en `display_config.h`
   - Cambiar después requiere recompilación

2. **Backlight**
   - GPIO debe ser capaz de PWM para control gradual
   - Actualmente usa ON/OFF simple (> 128 = ON, <= 128 = OFF)

3. **Estados**
   - Se actualizan automáticamente cuando ejecutas comandos
   - Puedes leerlos desde el motor de procesos

4. **Rendering**
   - `drawText()` actualmente solo registra en log
   - TODO: Implementar rendering real con fuentes

---

## 🔮 TODO

- [ ] Implementar rendering real con font support
- [ ] PWM para brightness gradual
- [ ] Soporte para imágenes BMP/PNG
- [ ] Touch screen support
- [ ] Rotación de pantalla configurable
- [ ] Modo sleep/standby

---

## 📞 Soporte y Documentación

- **EXAMPLES.md** — Ejemplos JSON de todos los comandos
- **TESTING.md** — Guía de pruebas y troubleshooting
- Revisar logs: `bunny monitor`
- Logs empiezan con `[display]`
- Verificar pines en `display_config.h`

---

**Versión:** 1.0.0  
**Compatible con:** Bunny Framework 1.0+  
**Requiere:** ESP-IDF 4.4+
