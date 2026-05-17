# 📚 Bunny Libraries Catalog

Lista de librerías/componentes pre-hechos para extender Bunny Framework.

---

## 🎨 Display

### ST7789 2.8" TFT Display
**Descripción:** Pantalla TFT 240x320 con ST7789 driver

**Instalación:**
```bash
cp -r libraries/display_st7789 components/display
```

**Capacidades (Commands):**
- `fillScreen(color)` - Llenar pantalla con color
- `drawText(text, x, y)` - Dibujar texto en posición
- `setBrightness(level)` - Controlar brillo backlight (0-255)
- `clearScreen()` - Limpiar pantalla a negro

**Estados (States):**
- `displayStatus` - Estado: "ON" o "OFF"
- `brightness` - Nivel brillo actual (0-255)
- `lastText` - Último texto mostrado

**Configuración:**
Editar `components/display/display_config.h` antes de compilar:

```cpp
#define DISPLAY_PIN_MOSI    23    // Tu pin MOSI
#define DISPLAY_PIN_SCLK    18    // Tu pin SCLK
#define DISPLAY_PIN_CS      5     // Tu pin CS
#define DISPLAY_PIN_DC      2     // Tu pin DC
#define DISPLAY_PIN_RST     4     // Tu pin RST
#define DISPLAY_PIN_MISO    19    // Tu pin MISO
#define DISPLAY_PIN_BL      32    // Tu pin Backlight
```

**Uso en main:**
```c
#include "display/display.h"

void app_main(void) {
    bunny_begin();
    bunny::display::register_display();
    bunny_load_modules();
    bunny_loop();
}
```

**Ejemplo de comando (JSON):**
```json
{
  "type": "command",
  "command": "fillScreen",
  "params": {
    "color": "blue"
  }
}
```

**Colores disponibles:** black, white, red, green, blue, yellow, cyan, magenta

---

## 🔧 Sensores

*(Próximamente)*

---

## ⚡ Actuadores

*(Próximamente)*

---

## 📖 Cómo contribuir

Para agregar una nueva librería:

1. Crear carpeta en `libraries/nombre_libreria/`
2. Agregar `README.md` con instrucciones
3. Incluir `display_config.h` o equivalente para configuración
4. Actualizar este `INDEX.md`

---

**Versión:** 1.0.0
