# Configuración de Bunny en Windows

Esta guía te ayuda a configurar y usar el framework Bunny en Windows.

## Requisitos

1. **Python 3.7+** instalado y en PATH
2. **ESP-IDF** (versión recomendada: v5.0 o superior)
3. **Git** (para clonar y trabajar con el repositorio)
4. **Driver USB** para tu ESP32 (CH340, CP2102 o similar)

## Instalación Rápida

### 1. Instalar Python 3
- Descarga desde https://www.python.org/
- **Importante:** Marca "Add Python to PATH" durante la instalación

### 2. Instalar ESP-IDF
Opción A: Instalador oficial
- Descarga desde https://docs.espressif.com/projects/esp-idf/en/stable/esp32/
- Sigue el instalador para Windows

Opción B: Manual (PowerShell)
```powershell
cd %USERPROFILE%
git clone -b v5.0 --depth 1 https://github.com/espressif/esp-idf.git
cd esp-idf
.\install.bat
.\export.bat
```

### 3. Verificar instalación
```cmd
idf.py --version
python --version
```

## Usar el comando `bunny`

El framework incluye un comando CLI para flashear el ESP32 fácilmente.

### En CMD (Símbolo del sistema)
```cmd
cd C:\ruta\a\bunny_framework
bunny flash
bunny flash clean
```

### En PowerShell
```powershell
cd C:\ruta\a\bunny_framework
.\bunny.ps1 flash
.\bunny.ps1 flash clean
```

O con alias (opcional, agrégalo a tu perfil de PowerShell):
```powershell
Set-Alias bunny '.\bunny.ps1'
bunny flash
```

### Especificar puerto serial
```cmd
REM CMD
set BUNNY_PORT=COM4 && bunny flash
```

```powershell
# PowerShell
$env:BUNNY_PORT='COM4'
.\bunny.ps1 flash
```

## Encontrar tu puerto serial

### Con CMD
```cmd
mode
```

### Con PowerShell
```powershell
Get-WmiObject Win32_SerialPort | Select-Object Name, Description
```

O visualizar en:
- Gestor de dispositivos → Puertos (COM y LPT)

## Troubleshooting

### "python not found" o "idf.py not found"

**Solución 1:** Abrir ESP-IDF Command Prompt
- Windows Start Menu → ESP-IDF Command Prompt
- Ejecutar ahí: `bunny flash`

**Solución 2:** Agregar ESP-IDF a PATH
```cmd
set PATH=%PATH%;C:\esp\esp-idf\tools
set IDF_PATH=C:\esp\esp-idf
```

### "El dispositivo no se detecta"

1. Instala el driver USB correcto:
   - ESP32 típicos: CH340 o CP2102
   - Busca "CH340 driver Windows" o "CP2102 driver"

2. Verifica el puerto en Gestor de dispositivos

3. Especifica manualmente:
```cmd
set BUNNY_PORT=COM4 && bunny flash
```

### "Permission denied" en PowerShell

Si ves un error de ejecución de scripts:
```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

## Buildear manualmente (sin comando `bunny`)

```cmd
idf.py build
idf.py -p COM3 flash monitor
```

## Variables de entorno

| Variable | Descripción | Ejemplo |
|----------|-------------|---------|
| `BUNNY_PORT` | Puerto serial (COM1, COM4, etc.) | `set BUNNY_PORT=COM4` |
| `BUNNY_BAUD` | Velocidad en baudios (default: 115200) | `set BUNNY_BAUD=230400` |
| `IDF_PATH` | Ruta a ESP-IDF | `set IDF_PATH=C:\esp\esp-idf` |

## Siguiente paso

- Consulta [README.md](../README.md) para información del framework
- Revisa [DEVELOPER_GUIDE.md](./DEVELOPER_GUIDE.md) para crear tus propios módulos
- Lee [BCP_SPECIFICATION.md](./BCP_SPECIFICATION.md) para el protocolo de comunicación
