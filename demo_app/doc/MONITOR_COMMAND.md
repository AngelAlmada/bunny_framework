# Bunny Monitor Command

El comando `bunny monitor` (o `bunny m`) permite ver el output serial del ESP32 en tiempo real.

## Uso Básico

### Monitor Simple
```bash
bunny monitor
```

Automáticamente:
- Detecta el puerto serial
- Conecta a la velocidad correcta (115200 baud)
- Muestra todo lo que el ESP32 envía

### Monitor con Puerto Personalizado
```bash
# Windows CMD
set BUNNY_PORT=COM4 && bunny monitor

# Windows PowerShell
$env:BUNNY_PORT='COM4'; bunny monitor

# Linux/macOS
BUNNY_PORT=/dev/ttyUSB0 ./bunny monitor
```

### Alias Corto
```bash
bunny m          # Equivalente a bunny monitor
```

---

## 🎯 Ejemplo Práctico

### Flujo Completo
```powershell
# 1. Flashear tu ESP32
bunny flash

# 2. Ver el output
bunny monitor

# (Verás algo como:)
# ets Jun  8 2016 00:22:57 rst:0x10 (RTCWDT_SFT), boot mode:(0,7)
# configsip: 0, SPIWP:0xee
# clk_drv:0x00,q_drv:0x00,d_drv:0x00...
# ...
```

### Salir del Monitor
```
Presiona: Ctrl + ]
```

En algunos sistemas:
```
Presiona: Ctrl + C
```

---

## 🔍 Cómo Funciona

El comando `bunny monitor`:
1. Detecta el puerto serial automáticamente
2. Se conecta a 115200 baud (puedes cambiar con `BUNNY_BAUD`)
3. Muestra todo lo que el ESP32 transmite
4. Útil para debugging, ver logs, depurar errores

---

## 📊 Combinaciones Útiles

### Flashear y Ver Monitor (en una línea)
```powershell
# Windows PowerShell
bunny flash; bunny monitor

# Linux/macOS
./bunny flash && ./bunny monitor
```

### Con Puerto Específico
```powershell
# Windows
$env:BUNNY_PORT='COM4'; bunny monitor

# Linux/macOS
BUNNY_PORT=/dev/ttyUSB0 ./bunny monitor
```

### Con Baudrate Personalizado
```powershell
# Windows
$env:BUNNY_BAUD='230400'; bunny monitor

# Linux/macOS
BUNNY_BAUD=230400 ./bunny monitor
```

---

## 🛠️ Troubleshooting

### "Port not found"
```
[bunny] Connecting to COM3 at 115200 baud...
Error: Port COM3 not found
```

**Solución:**
```powershell
# Encuentra el puerto correcto
Get-WmiObject Win32_SerialPort | Select-Object Name, Description

# Usa el puerto correcto
$env:BUNNY_PORT='COM4'
bunny monitor
```

### "Permission denied"
```
Error: Cannot open port /dev/ttyUSB0: Permission denied
```

**Solución (Linux):**
```bash
# Agrega tu usuario al grupo dialout
sudo usermod -a -G dialout $USER

# Luego cierra y abre sesión nuevamente
./bunny monitor
```

### "Device not responding"
```
Conected to /dev/ttyUSB0 but no data...
```

**Solución:**
1. Verifica que el ESP32 está conectado
2. Verifica el puerto es correcto
3. Asegúrate que usaste `bunny flash` antes
4. Intenta reiniciar el ESP32

---

## 📝 Comandos Relacionados

```bash
# Flashear
bunny flash

# Flashear y ver monitor
bunny flash; bunny monitor    # PowerShell
bunny flash && bunny monitor  # Linux/macOS

# Limpiar y flashear
bunny flash clean

# Ver ayuda
bunny --help

# Instalar entorno
bunny install
```

---

## 🎁 Características

✅ **Automático** — Detecta puerto automáticamente
✅ **Simple** — Un comando, sin configuración
✅ **Rápido** — Conexión inmediata
✅ **Compatible** — Windows, Linux, macOS
✅ **Variables** — Soporta BUNNY_PORT y BUNNY_BAUD

---

## 💡 Uso Práctico

### Para Debugging
```bash
# Flashea y monitorea
bunny flash
bunny monitor

# Verás los logs de tu aplicación ESP32
# Útil para encontrar errores
```

### Para Desarrollo Iterativo
```bash
# Loop: modificar → flashear → ver resultado
1. Edita tu código
2. bunny flash
3. bunny monitor
4. Verifica salida
5. Repite
```

### Para Testing
```bash
# Verifica que tu ESP32 está funcionando
bunny monitor

# Debería mostrar output del sistema o tu aplicación
```

---

## 🔌 Por Plataforma

### Windows (PowerShell)
```powershell
# Básico
bunny monitor

# Con puerto específico
$env:BUNNY_PORT='COM4'; bunny monitor

# Alias corto
bunny m
```

### Windows (CMD)
```cmd
# Básico (si bunny está en PATH)
bunny monitor

# Con puerto específico
set BUNNY_PORT=COM4 && bunny monitor

# Sin PATH (desde directorio)
python bunny monitor
```

### Linux/macOS
```bash
# Básico
./bunny monitor

# Con puerto específico
BUNNY_PORT=/dev/ttyUSB0 ./bunny monitor

# Si bunny está en PATH
bunny monitor
```

---

## 📚 Ver También

- [INSTALL_COMMAND.md](./INSTALL_COMMAND.md) — Comando install
- [README.md](../README.md) — Información general
- ESP-IDF Documentation — https://docs.espressif.com/
