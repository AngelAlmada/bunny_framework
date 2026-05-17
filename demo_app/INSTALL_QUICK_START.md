# Bunny Install - Guía Rápida

Instala automáticamente todo lo necesario para desarrollar con Bunny Framework en **1 comando**.

## En 30 Segundos

```bash
# 1. Navega a bunny_framework
cd C:\proyects\bunny_framework    # Windows
cd /ruta/a/bunny_framework        # Linux/macOS

# 2. Ejecuta install (una sola vez)
bunny install

# 3. Reinicia tu terminal/cmd

# 4. ¡Listo! Ya puedes usar
bunny flash
```

## Lo que Instala

```
✅ Python 3.7+           (Verifica que existe)
✅ ESP-IDF 5.0           (Framework oficial Espressif)
✅ Variables de entorno  (Agrega bunny a PATH)
```

## Comandos

```bash
# Instalación completa
bunny install
bunny i         # Alias corto

# Omitir Python (si ya tienes)
bunny install --skip-python

# Omitir ESP-IDF (si ya lo tienes)
bunny install --skip-espidf

# Omitir setup de variables
bunny install --skip-env

# Combinar opciones
bunny install --skip-python --skip-env
```

## Por Plataforma

### Windows
```cmd
bunny install
REM Espera a que termine
REM Cierra y abre nueva CMD
bunny flash  ✅
```

### Linux/macOS
```bash
./bunny install
# Espera a que termine
# (reinicia terminal si es necesario)
bunny flash  ✅
```

## Requisitos

- **Conexión a internet** (para descargar ESP-IDF)
- **Git instalado** (para clonar ESP-IDF)
- **~2GB espacio libre** en disco

## Si Algo Falla

### "Git not found"
```bash
# Instala Git desde https://git-scm.com/
# Reinicia terminal
bunny install
```

### "Python not found"
```bash
# Instala Python desde https://www.python.org/
# Reinicia terminal
bunny install
```

### "No reconoce bunny después de install"
```bash
# Reinicia completamente tu terminal/cmd
# En Linux/macOS también puedes:
source ~/.bashrc    # o ~/.zshrc
```

## Después de Install

```bash
# Flashear tu ESP32
bunny flash

# Limpiar build y flashear
bunny flash clean

# Ver ayuda
bunny --help

# Especificar puerto
BUNNY_PORT=COM4 bunny flash        # Windows CMD
$env:BUNNY_PORT='COM4'; bunny flash # Windows PS
BUNNY_PORT=/dev/ttyUSB0 bunny flash # Linux/macOS
```

## Más Información

→ Lee [doc/INSTALL_COMMAND.md](../doc/INSTALL_COMMAND.md) para detalles completos
→ Lee [README.md](../README.md) para información del proyecto
