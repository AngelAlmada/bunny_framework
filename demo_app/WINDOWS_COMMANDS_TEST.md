# Test script para verificar los comandos en Windows

Este script contiene los pasos para probar que los comandos funcionan correctamente en Windows.

## Requisitos previos
- Python 3.7+ instalado y en PATH
- ESP-IDF instalado y configurado
- Un ESP32 conectado al puerto serial

## Test 1: Verificar help en Windows CMD
```cmd
cd C:\proyects\bunny_framework
python bunny --help
python bunny -h
python bunny help
```

Resultado esperado:
- Mostrar ejemplos específicos para Windows (CMD y PowerShell)
- Mencionar "set BUNNY_PORT=COM4 && bunny flash" para CMD

## Test 2: Usar bunny.bat en Windows CMD
```cmd
cd C:\proyects\bunny_framework
bunny --help
bunny flash
```

Resultado esperado:
- bunny.bat wrapper ejecuta correctamente
- Detecta automáticamente el puerto serial

## Test 3: Usar bunny.ps1 en PowerShell
```powershell
cd C:\proyects\bunny_framework
.\bunny.ps1 -help
.\bunny.ps1 flash
```

Resultado esperado:
- bunny.ps1 wrapper ejecuta correctamente
- Muestra los ejemplos para PowerShell

## Test 4: Variable de entorno BUNNY_PORT en CMD
```cmd
set BUNNY_PORT=COM4
python bunny flash
```

## Test 5: Variable de entorno BUNNY_PORT en PowerShell
```powershell
$env:BUNNY_PORT='COM4'
.\bunny.ps1 flash
```

## Archivos modificados
1. `bunny` - Script principal actualizado con ejemplos por SO
2. `bunny.bat` - NUEVO: Wrapper para CMD
3. `bunny.ps1` - NUEVO: Wrapper para PowerShell
4. `README.md` - Agregada sección de Comandos rápidos
5. `doc/WINDOWS_SETUP.md` - NUEVO: Guía completa de Windows

## Cambios en el código
- Función `usage()` ahora detecta el SO y muestra ejemplos apropiados
- Función `_find_serial_port_windows()` intenta detectar puertos disponibles
- Compatible hacia atrás: sigue funcionando en Linux y macOS
