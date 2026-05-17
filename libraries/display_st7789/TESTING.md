# 🧪 Display Testing Guide

Guía para probar las capacidades de la pantalla ST7789.

---

## ✅ Pre-requisitos

1. Librería display copiada a `components/display/`
2. Pines configurados en `display_config.h`
3. Main actualizado con `bunny::display::register_display()`
4. Compilado: `bunny flash`
5. Monitor ejecutando: `bunny monitor`

---

## 🧪 Tests Básicos

### Test 1: Inicialización

**Esperado en logs:**
```
[display] Initializing hardware...
[display] Hardware initialized successfully
[display] MOSI: GPIO23, SCLK: GPIO18, CS: GPIO5, DC: GPIO2, RST: GPIO4, BL: GPIO19
[display] Registering display capabilities...
[display] Display module ready (4 commands, 3 states)
```

**Verificación:** ✅ Si ves estos logs, la librería se cargó correctamente

---

### Test 2: Llenar Pantalla

**Comando JSON:**
```json
{
  "type": "command",
  "command": "fillScreen",
  "params": {
    "color": "blue"
  }
}
```

**Esperado en logs:**
```
[display] Fill screen with color: blue (0x001F)
```

**Hardware:**
- Pantalla debería mostrar azul
- Si no tienes hardware, solo verás el log

---

### Test 3: Dibujar Texto

**Comando JSON:**
```json
{
  "type": "command",
  "command": "drawText",
  "params": {
    "text": "Hello Bunny",
    "x": 10,
    "y": 50
  }
}
```

**Esperado en logs:**
```
[display] Draw text at (10, 50): 'Hello Bunny'
```

**Estado actualizado:**
```json
{
  "type": "state_get",
  "state": "lastText"
}
```

**Respuesta:** `"Hello Bunny"`

---

### Test 4: Control de Brillo

**Comando JSON:**
```json
{
  "type": "command",
  "command": "setBrightness",
  "params": {
    "level": 128
  }
}
```

**Esperado en logs:**
```
[display] Brightness: 128
```

**Estado:**
```json
{
  "type": "state_get",
  "state": "brightness"
}
```

**Respuesta:** `"128"`

---

### Test 5: Limpiar Pantalla

**Comando JSON:**
```json
{
  "type": "command",
  "command": "clearScreen"
}
```

**Esperado en logs:**
```
[display] Clear screen
```

---

## 🎯 Test de Secuencia

Envía estos comandos en orden:

```json
[
  {
    "type": "command",
    "command": "fillScreen",
    "params": { "color": "black" }
  },
  {
    "type": "command",
    "command": "drawText",
    "params": {
      "text": "Bunny Framework",
      "x": 50,
      "y": 100
    }
  },
  {
    "type": "command",
    "command": "setBrightness",
    "params": { "level": 255 }
  }
]
```

**Esperado:**
1. Pantalla negra
2. Texto "Bunny Framework" visible
3. Brillo máximo

---

## 📊 Test de Estados

### Leer todos los estados

```json
[
  {
    "type": "state_get",
    "state": "displayStatus"
  },
  {
    "type": "state_get",
    "state": "brightness"
  },
  {
    "type": "state_get",
    "state": "lastText"
  }
]
```

**Esperado:**
- `displayStatus`: "ON"
- `brightness`: "255" (o último valor)
- `lastText`: texto dibujado

---

### Cambiar estado manualmente

```json
{
  "type": "state_set",
  "state": "displayStatus",
  "value": "OFF"
}
```

**Esperado en logs:**
```
[display] Display status: OFF
```

---

## 🔍 Verificación de Pines

Para verificar que los pines se configuraron correctamente:

**Logs esperados:**
```
[display] MOSI: GPIO23, SCLK: GPIO18, CS: GPIO5, DC: GPIO2, RST: GPIO4, BL: GPIO19
```

**Si los números son diferentes:**
- Editar `components/display/display_config.h`
- Cambiar #define DISPLAY_PIN_* con tus valores
- Recompilar: `bunny flash`

---

## 🎨 Test de Colores

Probar cada color disponible:

```json
{
  "type": "command",
  "command": "fillScreen",
  "params": { "color": "red" }
}
```

Cambiar `"red"` por:
- `"black"` → Negro
- `"white"` → Blanco
- `"green"` → Verde
- `"blue"` → Azul
- `"yellow"` → Amarillo
- `"cyan"` → Cian
- `"magenta"` → Magenta

---

## ⚠️ Errores Comunes

### Error: "fillScreen: missing 'color' parameter"

**Causa:** JSON incorrecto
**Solución:**
```json
{
  "type": "command",
  "command": "fillScreen",
  "params": {
    "color": "blue"    ← Debe tener "params"
  }
}
```

---

### Error: "drawText: missing 'text' parameter"

**Causa:** Falta el parámetro text
**Solución:**
```json
{
  "type": "command",
  "command": "drawText",
  "params": {
    "text": "Tu texto aquí",    ← REQUERIDO
    "x": 10,
    "y": 50
  }
}
```

---

### Pantalla no se enciende

**Checklist:**
- [ ] GPIO BL (backlight) configurado en `display_config.h`
- [ ] Wire conectado correctamente
- [ ] Voltaje correcto (3.3V para ESP32)
- [ ] Pin BL está habilitado en código

---

### Logs no aparecen

**Checklist:**
- [ ] `bunny monitor` está ejecutando
- [ ] `bunny::display::register_display()` está en main
- [ ] Compilación con `bunny flash` completó sin errores

---

## 🏆 Checklist Completo

- [ ] Librería copiada a components/display
- [ ] display_config.h editado con pines correctos
- [ ] main incluye display/display.h
- [ ] main llama a bunny::display::register_display()
- [ ] Compilado sin errores (bunny flash)
- [ ] Monitor ejecutando (bunny monitor)
- [ ] Logs muestran "Display module ready"
- [ ] fillScreen funciona (véase en logs)
- [ ] drawText funciona (véase en logs)
- [ ] setBrightness funciona (véase en logs)
- [ ] Estados se actualizan (state_get)

**Si todo está ✅, ¡tu display está funcional!**

---

## 📈 Prueba de Carga

Una vez funcionando todo, prueba comandos rápidos:

```json
{
  "type": "command",
  "command": "fillScreen",
  "params": { "color": "white" }
}
```

Envía esto 10 veces rápido. Esperado:
- Todos los comandos procesados
- Logs muestran cada uno
- Sin crashes o timeouts

---

## 🚀 Siguiente Paso

Cuando todo funcione:
1. Integra con tu lógica de sensores
2. Muestra datos en pantalla
3. Implementa rendering real (TODO en librería)

---

**¡Ya tienes la pantalla lista para probar!** 🎉
