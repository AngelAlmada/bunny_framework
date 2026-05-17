# Setup Rápido - Agregar Bunny al PATH

Después de hacer setup, podrás usar `bunny flash` desde **cualquier directorio** de tu computadora.

## Instalación en 1 Minuto

### Windows (elige uno)

#### Opción A: CMD
```cmd
cd C:\ruta\a\bunny_framework
setup-bunny-env.bat
```
Luego **reinicia CMD**.

#### Opción B: PowerShell
```powershell
cd C:\ruta\a\bunny_framework
.\setup-bunny-env.ps1
```

#### Opción C: Universal (funciona en cualquier plataforma)
```cmd
cd C:\ruta\a\bunny_framework
python setup-bunny-env.py
```

### Linux/macOS

```bash
cd /ruta/a/bunny_framework
bash setup-bunny-env.sh
```

## Verificar que funcionó

Abre una **nueva terminal/cmd** y ejecuta:
```bash
bunny --help
```

Si ves la ayuda, ¡está funcionando! ✅

## Ahora puedes usar

Desde **cualquier ubicación**:
```bash
bunny flash              # Flashear el ESP32
bunny flash clean        # Limpiar y rebuildar
bunny --help             # Ver ayuda
```

Con variables de entorno:
```bash
# Windows CMD
set BUNNY_PORT=COM4 && bunny flash

# Windows PowerShell
$env:BUNNY_PORT='COM4'; bunny flash

# Linux/macOS
BUNNY_PORT=/dev/ttyUSB0 bunny flash
```

## Si algo no funciona

- **Reinicia tu terminal/cmd** (es lo más común)
- Lee [doc/SETUP_ENVIRONMENT.md](doc/SETUP_ENVIRONMENT.md) para troubleshooting detallado
- Ejecuta el setup como **administrador** en Windows si obtienes errores de permisos

## Scripts disponibles

| Script | Para | Comando |
|--------|------|---------|
| `setup-bunny-env.bat` | Windows CMD | `setup-bunny-env.bat` |
| `setup-bunny-env.ps1` | Windows PowerShell | `.\setup-bunny-env.ps1` |
| `setup-bunny-env.sh` | Linux/macOS | `bash setup-bunny-env.sh` |
| `setup-bunny-env.py` | Cualquier OS | `python setup-bunny-env.py` |

¡Listo! 🚀
