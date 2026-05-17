# Bunny Install Command

El comando `bunny install` (o `bunny i`) automatiza todo el proceso de instalación del entorno de desarrollo de Bunny Framework.

## Qué Instala

El comando `bunny install` realiza las siguientes tareas:

1. **Verifica Python 3.7+** — Necesario para ESP-IDF y Bunny
2. **Instala ESP-IDF** — El framework oficial de Espressif para ESP32
3. **Configura Variables de Entorno** — Para poder usar `bunny` desde cualquier ubicación

## Uso Básico

### Instalación Completa
```bash
bunny install
```

El script automáticamente:
- Detecta tu sistema operativo (Windows/Linux/macOS)
- Verifica Python
- Descarga e instala ESP-IDF en `~/esp/esp-idf`
- Configura las variables de entorno
- Te indica los próximos pasos

### Instalación Rápida (omitir pasos)
```bash
# Omitir verificación de Python
bunny install --skip-python

# Omitir instalación de ESP-IDF
bunny install --skip-espidf

# Omitir setup de variables de entorno
bunny install --skip-env

# Combinaciones
bunny install --skip-python --skip-env
```

## Requisitos Previos

### Windows
- **Conexión a internet** — Para descargar ESP-IDF
- **Git instalado** — Para clonar repositorios
- **Espacio en disco** — ~2GB para ESP-IDF

### Linux/macOS
- **Conexión a internet**
- **Git instalado**
- **Herramientas de compilación** (gcc, make, etc.)
- **Espacio en disco** — ~2GB para ESP-IDF

## Pasos Detallados

### Paso 1: Ejecutar Instalación
```bash
cd C:\ruta\a\bunny_framework    # Windows
cd /ruta/a/bunny_framework      # Linux/macOS

bunny install
```

### Paso 2: Esperar a que Termine
El instalador mostrará el progreso:
```
[bunny] Checking Python installation...
[bunny] ✓ Python 3.11.2 is installed

[bunny] Checking ESP-IDF installation...
[bunny] ESP-IDF not found, installing...
[bunny] Cloning ESP-IDF to /home/user/esp/esp-idf...
[bunny] Running ESP-IDF installer...
[bunny] ✓ ESP-IDF installed successfully

[bunny] Setting up environment variables...
[bunny] ✓ Environment setup completed

================================================================================
✓ Installation completed successfully!
================================================================================
```

### Paso 3: Reiniciar Terminal
Cierra y abre una nueva terminal/cmd para que los cambios de variables de entorno tomen efecto.

### Paso 4: Verificar
```bash
bunny --help
```

## Dónde se Instala

### ESP-IDF
```
Windows:  C:\Users\YourUsername\esp\esp-idf
Linux:    /home/username/esp/esp-idf
macOS:    /Users/username/esp/esp-idf
```

### Variables de Entorno
- **Windows**: Se agregan al registro de usuarios (HKCU\Environment)
- **Linux/macOS**: Se agregan a ~/.bashrc o ~/.zshrc

### Comando bunny
Se agrega a PATH para poder usarlo desde cualquier ubicación

## Solución de Problemas

### "Git not found"
```
Error: git command not found
```

**Solución:**
1. Instala Git desde https://git-scm.com/download/
2. Reinicia tu terminal
3. Intenta de nuevo: `bunny install`

### "Python version too old"
```
Error: Python 3.7+ is required
```

**Solución:**
1. Instala Python 3.7+ desde https://www.python.org/
2. Asegúrate de marcar "Add Python to PATH"
3. Reinicia tu terminal
4. Intenta de nuevo: `bunny install`

### "ESP-IDF installation failed"
```
Error: Failed to install ESP-IDF
```

**Solución:**
1. Verifica tu conexión a internet
2. Borra la carpeta `~/esp/esp-idf` si existe
3. Intenta de nuevo: `bunny install`
4. Si persiste, instala manualmente:
   - Descarga desde https://github.com/espressif/esp-idf/releases
   - Coloca en `~/esp/esp-idf`
   - Ejecuta: `bunny install --skip-espidf`

### "Environment setup failed"
```
Error: Failed to setup environment
```

**Solución:**
1. En Windows, abre terminal como **Administrador**
2. En Linux/macOS, asegúrate de tener permisos de escritura en tu home
3. Intenta de nuevo: `bunny install`

### No reconoce "bunny" después de install
```
bunny: command not found
```

**Solución:**
1. **Reinicia completamente tu terminal/cmd**
2. En Linux/macOS, ejecuta: `source ~/.bashrc` (o `~/.zshrc`)
3. En Windows, abre una nueva ventana de CMD/PowerShell
4. Verifica: `bunny --help`

## Opciones Avanzadas

### Omitir verificaciones individuales
```bash
# Solo verificar, no instalar nada
bunny install --skip-python --skip-espidf --skip-env

# Solo instalar Python y ESP-IDF, no configurar PATH
bunny install --skip-env

# Solo configurar PATH (si ya tienes todo instalado)
bunny install --skip-python --skip-espidf
```

### Variables de Entorno Personalizadas
Si necesitas instalar ESP-IDF en un directorio diferente:

```bash
# Windows
set IDF_PATH=C:\custom\path\to\esp-idf
bunny install --skip-espidf

# Linux/macOS
export IDF_PATH=/custom/path/to/esp-idf
./bunny install --skip-espidf
```

## Después de la Instalación

Una vez completada la instalación, puedes:

### 1. Flashear tu ESP32
```bash
bunny flash
bunny flash clean    # Limpiar build y flashear
```

### 2. Especificar puerto serial
```bash
# Windows CMD
set BUNNY_PORT=COM4 && bunny flash

# Windows PowerShell
$env:BUNNY_PORT='COM4'; bunny flash

# Linux/macOS
BUNNY_PORT=/dev/ttyUSB0 bunny flash
```

### 3. Ver ayuda
```bash
bunny --help
bunny install --help    # Próximamente
```

## Flujo Completo (Primer Usuario)

```
1. Descargar bunny_framework
   $ git clone https://github.com/...

2. Entrar al directorio
   $ cd bunny_framework

3. Ejecutar install
   $ bunny install

4. Seguir instrucciones en pantalla
   (esperar a que termine)

5. Reiniciar terminal
   (cerrar y abrir nueva)

6. Flashear ESP32
   $ bunny flash

7. ¡Listo!
```

## Información de Sistema

El comando `bunny install` detecta automáticamente:

- **SO**: Windows, Linux o macOS
- **Shell**: Bash, Zsh (en Linux/macOS)
- **Python**: Versión instalada
- **ESP-IDF**: Si ya está instalado
- **Puertos seriales**: ESP32 conectados

## Ver También

- [README.md](../README.md) — Información general del proyecto
- [SETUP_QUICK_START.md](../SETUP_QUICK_START.md) — Configuración rápida
- [WINDOWS_SETUP.md](./WINDOWS_SETUP.md) — Guía específica para Windows
- [SETUP_ENVIRONMENT.md](./SETUP_ENVIRONMENT.md) — Configuración de variables de entorno

## Notas Importantes

- La instalación **requiere conexión a internet** para descargar ESP-IDF
- Se descarga ~600MB y descomprime a ~2GB
- El proceso puede tomar **5-15 minutos** dependiendo de tu velocidad de internet
- Después de instalar, **reinicia tu terminal** para que funcione
- Si algo falla, prueba ejecutar como **administrador** (Windows) o con **sudo** (Linux/macOS, si es necesario)

## Soporte

Si encuentras problemas:

1. Lee esta guía nuevamente
2. Consulta [SETUP_ENVIRONMENT.md](./SETUP_ENVIRONMENT.md)
3. Verifica que tienes todos los requisitos previos
4. Intenta con: `bunny install --skip-env` (si el problema es con variables de entorno)
