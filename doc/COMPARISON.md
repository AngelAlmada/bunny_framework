# Comparativa: Bunny Framework vs PlatformIO (y similares)

Al entrar al ecosistema de desarrollo embebido para ESP32, es natural preguntarse en qué se diferencia **Bunny Framework** de soluciones establecidas como **PlatformIO** o el uso del IDE de **Arduino**.

La respuesta corta es: **Resuelven problemas diferentes.**

Mientras que PlatformIO es una herramienta de compilación universal, Bunny Framework es una **arquitectura de software opinada** de nivel empresarial.

Aquí tienes el desglose técnico y filosófico:

## 1. El Propósito Principal

- **PlatformIO (PIO):** Es un ecosistema "universal". Su objetivo es que puedas programar un Arduino Uno, un STM32 y un ESP32 usando el mismo entorno (VS Code). Te entrega un archivo `main.cpp` en blanco y te dice *"buena suerte, escribe tu código como quieras"*.
- **Bunny Framework:** Es un framework enfocado exclusivamente en ESP32 orientado a la **arquitectura**. Su objetivo no es soportar 500 microcontroladores distintos, sino revolucionar cómo se escribe código para el ESP32. Te impone un patrón de diseño profesional (*Commands, States, Sensors, Events*) para que proyectos grandes no se vuelvan código espagueti inantenible.

## 2. Generación de Código y Scaffolding (CLI)

- **PlatformIO / Arduino:** No generan código. Si necesitas crear una clase para manejar un sensor, tienes que crear el `.h` y el `.cpp` a mano, escribir el boilerplate y vincularlos.
- **Bunny Framework:** Actúa como herramientas modernas del mundo web (tipo **NestJS** o **Angular**). Haces `bunny generate sensor humedad` y automáticamente te crea los archivos, inyecta las dependencias, e inscribe la lógica en la aplicación central. Esto acelera el desarrollo enormemente y estandariza el código de todo el equipo.

## 3. El Sistema de Compilación (Under the hood)

- **PlatformIO:** Usa su propio sistema de compilación subyacente llamado *SCons*. Esto a menudo causa que cuando Espressif lanza una nueva versión de ESP-IDF, PlatformIO tarde semanas o meses en soportarla, ya que tienen que adaptar y parchear su sistema interno.
- **Bunny Framework:** Es una envoltura (*Wrapper*) inteligente. Por debajo utiliza el motor **CMake nativo y oficial de ESP-IDF** orquestado a través de su ejecutable global `bunny.exe`. Esto significa que Bunny siempre será 100% compatible con las versiones más nuevas de Espressif desde el día cero, sin capas de traducción ni retrasos.

## 4. Filosofía de Lógica de Negocio

- **PlatformIO / Arduino:** Tú escribes la lógica del negocio (`if/else`, condicionales, rutinas) quemada (hardcodeada) directamente en el microcontrolador. Si las reglas cambian, debes re-compilar y re-flashear.
- **Bunny Framework:** Obliga a seguir la filosofía **Capabilities-first**. El firmware expone "Capacidades" (qué puede hacer el hardware), pero las reglas de negocio viven en un Motor de Procesos externo (una nube, un edge server, o una app). Esto significa que **puedes cambiar cómo se comporta el dispositivo sin volver a flashearlo**.

## 5. Manejo de Dependencias

- **PlatformIO:** Tiene su propio gestor de librerías (`platformio.ini`) y su propia tienda de paquetes que es ajena a Espressif.
- **Bunny Framework:** Se apoya nativamente en el **Component Registry de Espressif**, que es el estándar oficial de la industria para ESP-IDF, permitiendo usar componentes empresariales directamente de forma nativa.

---

### 🏆 Resumen: ¿Cuándo elegir Bunny?

Si solo quieres encender un LED rápido en 5 placas diferentes (STM32, Arduino, ESP8266), usarás **PlatformIO** o el IDE de **Arduino**.

Pero si estás desarrollando un **producto real, escalable y robusto** exclusivamente con ESP32 (como un termostato inteligente, un dispositivo IoT industrial, o un nodo domótico avanzado), elegirás **Bunny Framework**. 

¿Por qué? Porque Bunny no solo te instala el compilador, sino que te provee de una **arquitectura de software de grado empresarial** estructurada de fábrica, y un CLI mágico para generar tu código en segundos. Es el estándar para equipos que buscan profesionalizar su firmware.
