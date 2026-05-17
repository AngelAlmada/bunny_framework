# Setup Bunny Environment Variables

Esta guía explica cómo agregar el comando `bunny` a las variables de entorno de tu sistema para poder usarlo desde cualquier ubicación.

## ¿Por qué hacer setup?

Después de hacer setup, puedes usar:
```bash
bunny flash              # Desde cualquier directorio
bunny flash clean        # Sin necesidad de escribir la ruta completa
```

En lugar de:
```bash
cd C:\proyects\bunny_framework
bunny flash              # O python bunny flash
```

## Instalación por Plataforma

### Windows (CMD)

#### Opción 1: Script automático
```cmd
cd C:\ruta\a\bunny_framework
setup-bunny-env.bat
```

El script:
- Detecta la ubicación de bunny_framework
- Agrega al PATH de Windows
- Verifica que se agregó correctamente
- **Requiere reiniciar CMD** para que funcione

#### Opción 2: Manual (sin script)
1. Abre Variables de Entorno:
   - Presiona `Win + X` → Selecciona "Sistema"
   - O busca "Variables de entorno" en el menú de inicio
2. Haz clic en "Variables de entorno"
3. En "Variables de usuario" → Haz clic en "Path" → "Editar"
4. Haz clic en "Nuevo" y agrega: `C:\ruta\a\bunny_framework`
5. Haz clic en "Aceptar" en todos los diálogos
6. Reinicia CMD

### Windows (PowerShell)

#### Opción 1: Script automático
```powershell
cd C:\ruta\a\bunny_framework
.\setup-bunny-env.ps1
```

El script:
- Detecta la ubicación de bunny_framework
- Agrega al PATH del usuario
- Verifica que se agregó correctamente
- **Puede requerir reiniciar PowerShell** en algunos casos

#### Opción 2: Manual (editar perfil)
1. Abre PowerShell
2. Crea tu perfil (si no existe):
   ```powershell
   if (!(Test-Path -Path $PROFILE)) {
     New-Item -ItemType File -Path $PROFILE -Force
   }
   ```
3. Edita tu perfil:
   ```powershell
   notepad $PROFILE
   ```
4. Agrega al final:
   ```powershell
   $env:Path += ";C:\ruta\a\bunny_framework"
   ```
5. Guarda y cierra
6. Reinicia PowerShell

### Linux/macOS

#### Opción 1: Script automático
```bash
cd /ruta/a/bunny_framework
bash setup-bunny-env.sh
```

El script:
- Detecta tu shell (Bash, Zsh)
- Modifica `~/.bashrc` o `~/.zshrc`
- Crea un backup automático
- Te ofrece aplicar los cambios inmediatamente

#### Opción 2: Manual (editar perfil)
1. Abre tu terminal
2. Determina tu shell:
   ```bash
   echo $SHELL
   ```
3. Edita el archivo de configuración:
   ```bash
   # Para Bash
   nano ~/.bashrc
   
   # Para Zsh
   nano ~/.zshrc
   ```
4. Agrega al final:
   ```bash
   export PATH="$PATH:/ruta/a/bunny_framework"
   ```
5. Guarda con `Ctrl+O`, `Enter`, `Ctrl+X`
6. Recarga el archivo:
   ```bash
   source ~/.bashrc    # Para Bash
   # o
   source ~/.zshrc     # Para Zsh
   ```

## Setup Universal (Python)

Si prefieres un único script que funciona en cualquier plataforma:

```bash
python setup-bunny-env.py
```

El script:
- Detecta automáticamente tu SO
- En Windows, te pregunta si usas CMD o PowerShell
- En Linux/macOS, detecta tu shell
- Ejecuta el script correcto para tu plataforma

## Verificación

Después de hacer setup, verifica que funciona:

### Windows CMD
```cmd
bunny --help
bunny flash
```

### Windows PowerShell
```powershell
bunny --help
bunny flash
```

### Linux/macOS
```bash
bunny --help
bunny flash
```

Si el comando `bunny` no se encuentra, posiblemente necesites **reiniciar tu terminal**.

## Solución de Problemas

### "bunny not found" o "comando no encontrado"

**Causa:** La terminal no ha recargado las variables de entorno

**Solución:**
- Cierra completamente tu terminal y abre una nueva
- O recarga el perfil manualmente:
  - **Linux/macOS:** `source ~/.bashrc` (o `~/.zshrc`)
  - **Windows CMD:** Simplemente abre una nueva ventana
  - **Windows PS:** Simplemente abre una nueva ventana

### Windows: "Access Denied" o error de permisos

**Causa:** El script necesita permisos de administrador

**Solución:**
1. Abre CMD como administrador:
   - Presiona `Win + X` → "Símbolo del sistema (admin)"
2. Navega a bunny_framework: `cd C:\ruta\a\bunny_framework`
3. Ejecuta: `setup-bunny-env.bat`

### Error al ejecutar setup-bunny-env.ps1 en PowerShell

**Causa:** Política de ejecución de scripts bloqueada

**Solución:**
```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
.\setup-bunny-env.ps1
```

## Deshacer Setup

Si necesitas remover bunny del PATH:

### Windows
1. Abre Variables de Entorno (busca en menú inicio)
2. Selecciona "Path" en "Variables de usuario"
3. Haz clic en "Editar"
4. Encuentra la línea con la ruta de bunny_framework y elimínala
5. Haz clic en "Aceptar"
6. Reinicia tu terminal

### Linux/macOS
1. Abre tu archivo de configuración:
   ```bash
   nano ~/.bashrc    # o ~/.zshrc
   ```
2. Busca la línea que dice algo como:
   ```bash
   export PATH="$PATH:/ruta/a/bunny_framework"
   ```
3. Elimina esa línea
4. Guarda y cierra
5. Recarga:
   ```bash
   source ~/.bashrc
   ```

## Archivos de Setup Disponibles

| Archivo | Plataforma | Uso |
|---------|-----------|-----|
| `setup-bunny-env.bat` | Windows CMD | `setup-bunny-env.bat` |
| `setup-bunny-env.ps1` | Windows PowerShell | `.\setup-bunny-env.ps1` |
| `setup-bunny-env.sh` | Linux/macOS | `bash setup-bunny-env.sh` |
| `setup-bunny-env.py` | Universal (todas) | `python setup-bunny-env.py` |

## Próximos Pasos

Después de hacer setup, puedes:
- Usar `bunny flash` desde cualquier ubicación
- Usar `bunny flash clean` para limpiar y rebuildar
- Usar `bunny --help` para ver todas las opciones

Consulta [QUICK_START_WINDOWS.md](QUICK_START_WINDOWS.md) o [README.md](README.md) para más información.
