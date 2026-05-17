# 🎨 Display Capabilities - Ejemplos de Uso

Ejemplos prácticos de cómo usar la pantalla ST7789 con Bunny Framework.

---

## 📋 Capacidades Disponibles

### Commands (Acciones)
- `fillScreen(color)` - Llenar pantalla
- `drawText(text, x, y)` - Dibujar texto
- `setBrightness(level)` - Control de brillo
- `clearScreen()` - Limpiar pantalla

### States (Estados)
- `displayStatus` - ON/OFF
- `brightness` - 0-255
- `lastText` - Último texto

---

## 💡 Ejemplos JSON

### Ejemplo 1: Llenar pantalla de azul

```json
{
  "type": "command",
  "command": "fillScreen",
  "params": {
    "color": "blue"
  }
}
```

**Resultado:** Pantalla completamente azul
**Log:** `[display] Fill screen with color: blue (0x001F)`

---

### Ejemplo 2: Mostrar mensaje de bienvenida

```json
{
  "type": "command",
  "command": "drawText",
  "params": {
    "text": "Welcome to Bunny!",
    "x": 10,
    "y": 50
  }
}
```

**Resultado:** Texto en posición (10, 50)
**Log:** `[display] Draw text at (10, 50): 'Welcome to Bunny!'`
**Estado:** `lastText` se actualiza a "Welcome to Bunny!"

---

### Ejemplo 3: Pantalla de estado con temperatura

Escenario: Mostrar lectura de sensor

```json
{
  "type": "command",
  "command": "fillScreen",
  "params": {
    "color": "black"
  }
}
```

```json
{
  "type": "command",
  "command": "drawText",
  "params": {
    "text": "Temperature: 25.5°C",
    "x": 20,
    "y": 100
  }
}
```

**Flujo:** Fondo negro → Texto blanco con lectura

---

### Ejemplo 4: Control de brillo

```json
{
  "type": "command",
  "command": "setBrightness",
  "params": {
    "level": 200
  }
}
```

**Resultado:** Brillo al 200 (de 255)
**Estado:** `brightness` = 200
**Comportamiento:** GPIO backlight activo (PWM > 128)

---

### Ejemplo 5: Secuencia: Encender y mostrar mensaje

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
      "text": "Device Online",
      "x": 50,
      "y": 150
    }
  },
  {
    "type": "command",
    "command": "setBrightness",
    "params": { "level": 255 }
  }
]
```

**Flujo:**
1. Limpia a negro
2. Muestra "Device Online"
3. Brillo máximo

---

### Ejemplo 6: Dashboard con múltiples elementos

Escenario: Mostrar estado del sistema

```json
{
  "type": "command",
  "command": "fillScreen",
  "params": { "color": "navy" }
}
```

```json
{
  "type": "command",
  "command": "drawText",
  "params": {
    "text": "BUNNY SYSTEM",
    "x": 50,
    "y": 20
  }
}
```

```json
{
  "type": "command",
  "command": "drawText",
  "params": {
    "text": "Status: RUNNING",
    "x": 30,
    "y": 100
  }
}
```

```json
{
  "type": "command",
  "command": "drawText",
  "params": {
    "text": "Fan: ON",
    "x": 30,
    "y": 150
  }
}
```

**Resultado:** Panel de estado con múltiples líneas

---

## 🔍 Consultar Estados

### Obtener estado actual de brillo

```json
{
  "type": "state_get",
  "state": "brightness"
}
```

**Respuesta:** `"200"` (o el valor actual)

---

### Obtener estado de la pantalla

```json
{
  "type": "state_get",
  "state": "displayStatus"
}
```

**Respuesta:** `"ON"` o `"OFF"`

---

### Obtener último texto

```json
{
  "type": "state_get",
  "state": "lastText"
}
```

**Respuesta:** `"Welcome to Bunny!"` (o el texto mostrado)

---

## 🎨 Colores Disponibles

| Color | Código | Hex |
|-------|--------|-----|
| black | #000000 | 0x0000 |
| white | #FFFFFF | 0xFFFF |
| red | #FF0000 | 0xF800 |
| green | #00FF00 | 0x07E0 |
| blue | #0000FF | 0x001F |
| yellow | #FFFF00 | 0xFFE0 |
| cyan | #00FFFF | 0x07FF |
| magenta | #FF00FF | 0xF81F |

---

## 📊 Casos de Uso Reales

### Caso 1: Sistema de Alerta

```json
{
  "type": "command",
  "command": "fillScreen",
  "params": { "color": "red" }
}
```

```json
{
  "type": "command",
  "command": "drawText",
  "params": {
    "text": "ALERT!",
    "x": 100,
    "y": 150
  }
}
```

### Caso 2: Modo Bajo Consumo

```json
{
  "type": "command",
  "command": "setBrightness",
  "params": { "level": 50 }
}
```

```json
{
  "type": "command",
  "command": "drawText",
  "params": {
    "text": "Power Save Mode",
    "x": 30,
    "y": 150
  }
}
```

### Caso 3: Menú de Sistema

```json
{
  "type": "command",
  "command": "clearScreen"
}
```

```json
{
  "type": "command",
  "command": "drawText",
  "params": {
    "text": "1. Settings",
    "x": 20,
    "y": 50
  }
}
```

```json
{
  "type": "command",
  "command": "drawText",
  "params": {
    "text": "2. Status",
    "x": 20,
    "y": 100
  }
}
```

```json
{
  "type": "command",
  "command": "drawText",
  "params": {
    "text": "3. Exit",
    "x": 20,
    "y": 150
  }
}
```

---

## 🧪 Testing

### Test 1: Básico

```bash
# Verificar que display se registró
bunny monitor
# Debería ver: "[display] Display module ready"
```

### Test 2: Comandos

```bash
# Enviar comando fillScreen desde motor
# Debería ver: "[display] Fill screen with color: blue"
```

### Test 3: Estados

```bash
# Consultar brightness
# Respuesta: "255"
```

---

## 🔧 Troubleshooting

### Logs no aparecen
- Verificar que `bunny::display::register_display()` esté en main
- Verificar que `bunny monitor` está ejecutando

### Comando no responde
- Verificar sintaxis JSON
- Verificar nombre exacto del comando (case-sensitive)
- Verificar parámetros requeridos

### Display no enciende
- Verificar GPIO en `display_config.h`
- Verificar conexión de pines
- Verificar voltaje (3.3V para ESP32)

---

## 📚 Próximas Mejoras

- [ ] Font rendering real
- [ ] Imágenes BMP/PNG
- [ ] Animaciones
- [ ] Touch screen
- [ ] Rotación configurable

---

**¡Ahora tienes un display funcional con Bunny Framework!** 🎉
