# 📚 Cómo Usar las Librerías Bunny

Guía rápida para agregar librerías pre-hechas al tu proyecto.

---

## 🎨 Ejemplo: ST7789 Display

### Paso 1: Copiar librería

```bash
# Desde la raíz del proyecto
cp -r libraries/display_st7789 components/display
```

### Paso 2: Configurar pines

Editar `components/display/display_config.h`:

```cpp
#define DISPLAY_PIN_MOSI    23      // Cambia a tu pin MOSI
#define DISPLAY_PIN_SCLK    18      // Cambia a tu pin SCLK
#define DISPLAY_PIN_CS      5       // Cambia a tu pin CS
#define DISPLAY_PIN_DC      2       // Cambia a tu pin DC
#define DISPLAY_PIN_RST     4       // Cambia a tu pin RST
#define DISPLAY_PIN_MISO    19      // Cambia a tu pin MISO
#define DISPLAY_PIN_BL      32      // Cambia a tu pin Backlight
```

### Paso 3: Incluir en main

Archivo: `main/bunny_framework.c`

```c
#include "bunny.h"
#include "display/display.h"                    // ← Agregar esta línea

void app_main(void) {
    bunny_begin();

    /* Register all capabilities */
    register_temperature_sensor();
    register_fan_command();
    bunny::display::register_display();         // ← Agregar esta línea

    bunny_load_modules();
    bunny_loop();
}
```

### Paso 4: Compilar

```bash
bunny flash
```

### Paso 5: Usar

Enviar comando JSON desde motor de procesos:

```json
{
  "type": "command",
  "command": "fillScreen",
  "params": { "color": "blue" }
}
```

---

## ✨ Eso es todo

Solo:
1. ✅ Copiar carpeta
2. ✅ Configurar pines
3. ✅ Incluir header
4. ✅ Registrar en main
5. ✅ Compilar

**Sin escribir código de Bunny capabilities.**

---

## 🔍 Estructura de una librería Bunny

```
libraries/mi_componente/
├── README.md                    ← Instrucciones
├── component_config.h           ← Configuración (pines, etc)
├── component.h                  ← API pública
├── component.cpp                ← Implementación
├── CMakeLists.txt              ← Build config
└── idf_component.yml           ← ESP-IDF metadata
```

**Lo importante:** El usuario **solo copia, configura y usa.**

El resto del código (getters, setters, handlers) está adentro.

---

## 📖 Disponibles

Ver `libraries/INDEX.md` para catálogo completo.

---

**¡Listo! Ahora puedes agregar componentes como si fuera Arduino.** 🚀
