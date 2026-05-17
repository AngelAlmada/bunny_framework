# Quick Start: Bunny en Windows

## Para usuarios de CMD (Símbolo del sistema)

### Opción 1: Usar el comando `bunny` directamente
```cmd
cd C:\ruta\a\bunny_framework
bunny flash
bunny flash clean
```

Si necesitas especificar el puerto:
```cmd
set BUNNY_PORT=COM4
bunny flash
```

### Opción 2: Usar Python directamente (siempre funciona)
```cmd
cd C:\ruta\a\bunny_framework
python bunny flash
python bunny flash clean
```

---

## Para usuarios de PowerShell

### Opción 1: Usar el script PS1
```powershell
cd C:\ruta\a\bunny_framework
.\bunny.ps1 flash
.\bunny.ps1 flash clean
```

### Opción 2: Usar alias (recomendado)
Agrega esto a tu perfil de PowerShell (ejecuta una sola vez):
```powershell
Add-Alias -Name bunny -Value "$PWD\bunny.ps1"
```

Luego puedes usar:
```powershell
bunny flash
bunny flash clean
```

### Opción 3: Usar Python directamente
```powershell
python bunny flash
python bunny flash clean
```

---

## Encontrar tu puerto serial

### En CMD
```cmd
mode
```
Busca los puertos COM en la lista.

### En PowerShell
```powershell
Get-WmiObject Win32_SerialPort | Select-Object Name, Description
```

---

## Ver la ayuda

```cmd
# En CMD
bunny --help
bunny -h

# En PowerShell
.\bunny.ps1 --help
python bunny --help
```

---

## Specificar puerto serial

### CMD
```cmd
set BUNNY_PORT=COM4
bunny flash
```

### PowerShell
```powershell
$env:BUNNY_PORT='COM4'
bunny flash
```

O en una línea:
```powershell
$env:BUNNY_PORT='COM4'; bunny flash
```

---

## Si algo no funciona

1. **"bunny not found"** → Usa `python bunny flash` en su lugar
2. **"idf.py not found"** → Abre ESP-IDF CMD Prompt desde Windows Start Menu
3. **"Port not found"** → Especifica el puerto manualmente con `set BUNNY_PORT=COM4`
4. **Driver USB** → Instala el driver correcto para tu ESP32 (CH340 o CP2102)

---

## Más información

- Ver [doc/WINDOWS_SETUP.md](doc/WINDOWS_SETUP.md) para guía completa
- Ver [README.md](README.md) para información del proyecto
