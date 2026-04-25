# Documentación de tesis (cuerpo principal)

## 1. Resumen del aporte técnico

Bunny Framework propone una arquitectura declarativa para firmware en ESP32 que aborda un problema fundamental en el desarrollo de sistemas embebidos distribuidos: la necesidad de desacoplar completamente la lógica de ejecución de hardware de la lógica de toma de decisiones en procesos empresariales.

Tradicionalmente, los dispositivos embebidos implementan tanto la interacción con hardware como las reglas de negocio en el mismo código, lo que genera dificultades de mantenimiento, testing y evolución. Bunny resuelve esto mediante un modelo donde el ESP32 actúa como un **expositor de capacidades** (sensores que producen datos, comandos que ejecutan acciones, eventos que notifican sucesos y estados que mantienen información tipada), mientras que un **motor de procesos externo** es responsable de toda la lógica condicional, temporal y de flujo.

Este enfoque permite:

- **Separación clara entre lógica de hardware y lógica de procesos**: el firmware no contiene decisiones comerciales, solo expone qué puede hacer. El motor de procesos decide cuándo y cómo hacerlo.
- **Capacidades tipadas y documentadas mediante metadata**: cada capacidad incluye descripción, parámetros con tipos, valores de retorno y ejemplos, facilitando tanto la integración manual como la generación automática de código.
- **Integración reproducible mediante configuración del dispositivo**: la identidad, parámetros de red y endpoint del dispositivo se controlan mediante archivo de configuración JSON, permitiendo desplegar múltiples instancias sin recompilación.

## 2. Arquitectura funcional

La arquitectura se organiza en tres capas con responsabilidades bien definidas:

### Capa de aplicación (main/)

Contiene los módulos específicos del usuario donde se declaran las capacidades reales del dispositivo. Esta capa es donde el desarrollador implementa las interacciones con hardware (lectura de sensores ADC, control de GPIO, comunicación I2C, etc.) dentro de callbacks que son invocados por el runtime. La separación es clara: el usuario escribe dónde ocurren las acciones de hardware, pero no cuándo ni en qué contexto lógico ocurren. Los módulos de ejemplo (temperatura, ventilador, movimiento, estado) demuestran patrones que pueden ser replicados para agregar nuevas capacidades.

### Capa SDK (components/bunny/)

Proporciona la infraestructura declarativa y el runtime que gestiona todo el ciclo de vida del framework. Incluye los builders (API fluida para declarar capacidades), el registro central que almacena todas las capacidades en un singleton, los tipos base que definen qué es una capacidad, y la orquestación de inicialización. Esta capa actúa como intermediaria entre la aplicación del usuario y la infraestructura de red, permitiendo que el usuario no necesite conocer detalles de sockets, protocolos o tareas de FreeRTOS.

### Capa de red y configuración (config/, network/)

Maneja todo lo relacionado con conectividad: lectura de configuración desde device.json, conexión a WiFi, descubrimiento UDP periódico hacia el motor de procesos y preparación para comunicación WebSocket. Esta capa también proporciona puntos de enganche para el protocolo de intercambio de mensajes con el motor. La separación de esta capa permite que cambios en estrategia de red (por ejemplo, adoptar mDNS en lugar de UDP broadcast) no afecten la definición de capacidades del usuario.

## 3. Flujo de arranque del sistema

El flujo de arranque está cuidadosamente orquestado en cuatro fases para garantizar que todos los sistemas dependientes estén listos antes de iniciar operación. En la función entrada app_main() de FreeRTOS/ESP-IDF, se ejecutan:

**Fase 1 - Inicialización del framework (bunny_begin)**: Esta fase carga la configuración del dispositivo desde device.json (o valores por defecto compilados), inicializa los subsistemas de red (WiFi) y prepara el descubrimiento UDP. En este punto, el dispositivo aún no expone ninguna capacidad; solo prepara la infraestructura.

```c
void app_main(void)
{
    bunny_begin();  // Carga config, WiFi, discovery setup

    register_temperature_sensor();
    register_fan_command();
    register_motion_event();
    register_fan_state();

    bunny_load_modules();
    bunny_loop();
}
```

**Fase 2 - Registro de capacidades**: Los módulos llaman a sus funciones de registro (register_*), que construyen objetos de capacidad mediante el fluent API y los registran en el Registry central. En este punto, el contrato del dispositivo está completo, pero aún no es públicamente visible.

**Fase 3 - Carga de módulos (bunny_load_modules)**: Confirmando que todas las capacidades están registradas, el framework inicia la búsqueda de conexión WiFi con timeout configurable y luego inicia el descubrimiento UDP. Esta fase asegura que el dispositivo sea localizable en la red antes de intentar operación completa.

**Fase 4 - Bucle principal (bunny_loop)**: El framework entra en un loop infinito que mantiene heartbeat de conectividad, informa progreso a través de ESP-IDF logging (FLOW[0/4] a FLOW[4/4]) y aguarda instrucciones del motor de procesos.

Archivo fuente: main/bunny_framework.c

## 4. Modelo declarativo de capacidades

El SDK ofrece una API fluida (fluent API) que permite declarar capacidades sin mezclar lógica de negocio, control de flujo condicional o decisiones temporales. La motivación detrás de este enfoque es permitir que el usuario describa **qué el dispositivo puede hacer** de forma clara y estructurada, mientras que el **motor de procesos decide cuándo y cómo hacerlo**.

Considere el ejemplo de un comando que controla un ventilador:

```cpp
void register_fan_command() {
    Bunny.command("setFanState")
         .description("Turn the fan relay ON or OFF")
         .param("state", STRING, "Target state: ON or OFF")
         .affects("fanState")
         .execute([](const bunny::Params& p) {
             const char* state = p.get_string("state");
             set_fan_hw(strcmp(state, "ON") == 0);
         });
}
```

**Desglose del patrón**:

- **`.command("setFanState")`**: Nombra la capacidad de forma única y legible. El motor de procesos invocará esta capacidad por este nombre.

- **`.description(...)`**: Proporciona documentación semántica que puede consumir tanto humanos como agentes de IA para entender qué realiza la acción.

- **`.param("state", STRING, ...)`**: Define el contrato de entrada. El parámetro tiene nombre, tipo (STRING) y descripción. El framework validará automáticamente que el motor de procesos envíe un STRING, evitando errores tipográficos o incompatibilidades. Múltiples parámetros pueden encadenarse.

- **`.affects("fanState")`**: Metadata que indica que esta acción modifica el estado del ventilador. El motor de procesos puede usar esta información para entender dependencias y sincronizar estado.

- **`.execute([...])`**: El callback que realiza la acción en hardware. Note que **no contiene condicionales de negocio**: solo ejecuta la acción solicitada. No verifica si es hora de apagar, no consulta horarios, no valida rangos de temperatura. Solo hace lo que se le pide.

Este patrón asegura que el firmware sea agnóstico respecto a decisiones empresariales. Si mañana se requiere apagar el ventilador en horarios nocturnos, se agrega una regla en el motor de procesos, no en el firmware.

Archivo fuente: main/commands/fan_command.cpp

## 5. Registro central y serialización de capacidades

El Registry es un singleton (única instancia global) que actúa como "libro de registro" de todas las capacidades del dispositivo. Su responsabilidad es doble: almacenar las capacidades en memoria durante ejecución y poder serializarlas a JSON para transmitir al motor de procesos.

### Almacenamiento en memoria

Durante la fase de registro, cada capacidad (sensor, comando, evento o estado) es almacenada en un arreglo estático:

```cpp
bool Registry::register_capability(ICapability* cap) {
    if (!cap || _count >= MAX_CAPABILITIES) return false;
    _caps[_count++] = cap;
    return true;
}
```

El límite `MAX_CAPABILITIES` (32 en la implementación actual) es un parámetro de configuración compilada que respeta las restricciones de memoria del ESP32. La decisión de usar un arreglo estático (en lugar de lista dinámica) obedece a restricciones de memoria embebida: sin heap fragmentado, sin malloc/free en runtime, comportamiento predecible.

### Búsqueda y despacho

Cuando el motor de procesos envía una solicitud, el dispatcher busca la capacidad por nombre y tipo:

```cpp
ICapability* Registry::find(const char* name, CapabilityKind kind) const {
    for (size_t i = 0; i < _count; ++i) {
        if (_caps[i]->kind() == kind && strcmp(_caps[i]->name(), name) == 0) {
            return _caps[i];
        }
    }
    return nullptr;
}
```

Esta búsqueda es O(n) pero aceptable dada la expectativa de pocas decenas de capacidades. Si un motor intenta invocar una capacidad inexistente, la búsqueda retorna nullptr y el framework puede reportar error estructurado al motor.

### Serialización a JSON

Al arrancar o ante solicitud, el Registry serializa todas las capacidades a un manifiesto JSON que el motor de procesos consume para saber qué expone el dispositivo:

```cpp
size_t Registry::serialize_capabilities(char* buf, size_t len) const {
    json::append(buf, len, pos, "{");
    // Itera cuatro veces: una por sensor, comando, evento, estado
    // Agrupa capacidades por tipo
    json::append(buf, len, pos, "}");
    return pos;
}
```

Este manifiesto genera un JSON estructurado donde el motor puede consultar automáticamente qué capacidades expone el dispositivo, incluyendo tipos de parámetros, descripciones y ejemplos, sin necesidad de documentación manual externa.

Archivos fuente: components/bunny/registry/registry.h y components/bunny/registry/registry.cpp

## 6. Descubrimiento en red (UDP broadcast)

Uno de los retos de sistemas embebidos distribuidos es: ¿cómo encuentra el motor de procesos la dirección IP de cada dispositivo sin necesidad de configuración manual? Bunny resuelve esto mediante UDP broadcast periódico, un mecanismo probado en protocolos de descubrimiento como mDNS, SSDP y UPnP.

El dispositivo crea un socket UDP y envía periódicamente (cada 3 segundos por defecto) un mensaje a la dirección de broadcast (255.255.255.255), que es entregado a todos los hosts en la subred:

```c
int written = snprintf(payload, sizeof(payload),
    "{\"bunny\":true,\"id\":\"%s\",\"name\":\"%s\",\"version\":\"%s\",\"ip\":\"%s\",\"webhook_port\":%u,\"webhook_path\":\"%s\"}",
    cfg->device.id, cfg->device.name, cfg->device.version, ip_buf,
    (unsigned)cfg->webhook.port, cfg->webhook.path);

sendto(s_sock, payload, (size_t)written, 0,
    (struct sockaddr*)&dest_addr, sizeof(dest_addr));
```

El payload incluye información esencial: identificador único del dispositivo, nombre amigable, versión de firmware, IP local actual, puerto WebSocket y path donde se aceptan comandos. El motor de procesos escucha estos anuncios y, cuando los identifica (por `"bunny":true`), puede conectarse automáticamente. Una vez la conexión WebSocket se establece, el descubrimiento UDP puede detenerse (ahorro de ancho de banda). Si la conexión se pierde, el dispositivo puede reanudar broadcasts para ser redescubierto.

Archivo fuente: components/bunny/network/discovery.c

## 7. Configuración del dispositivo

La configuración declarativa es un pilar del diseño: permite que múltiples instancias del mismo firmware funcionen con parámetros distintos sin necesidad de recompilación. El archivo config/device.json es leído por bunny_config_load() durante inicialización y define:

```json
{
  "device": {
    "id": "esp32-001",
    "name": "Si estas viendo esto, ya lograste conectar todo. HOLA MUNDO en Esp32",
    "description": "Dispositivo ESP32 gestionado por Bunny Framework",
    "type": "esp32",
    "version": "0.1.0"
  },
  "network": {
    "wifi": {
      "ssid": "angeltest123",
      "password": "test1234",
      "auth_type": "WPA",
      "max_retries": 5,
      "timeout_ms": 10000
    }
  },
  "discovery": {
    "enabled": true,
    "udp_port": 5555,
    "broadcast_interval_ms": 3000
  },
  "webhook": {
    "port": 8080,
    "path": "/bunny"
  }
}
```

### Organización de configuración

- **device**: Identidad y metadatos del nodo. El `id` debe ser único en la red.
- **network.wifi**: Credenciales y parámetros de conexión. El `timeout_ms` limita espera antes de considerar WiFi no disponible.
- **discovery**: Parámetros del anuncio UDP. El `broadcast_interval_ms` balancea visibilidad y ancho de banda.
- **webhook**: Endpoint donde el motor de procesos se conecta. Configurables para múltiples instancias.

### Flujo de carga

Durante `bunny_config_load()`: se intenta leer device.json desde SPIFFS; si falla, se usan valores compilados desde macros CMake. Esto permite desplegar el mismo binario en múltiples dispositivos, cada uno obtiene su identidad de su copia local de device.json.

Archivo fuente: config/device.json

## 8. Estado de implementación reportable en tesis

El proyecto ha alcanzado un nivel de madurez donde los componentes fundamentales están completamente funcionales, permitiendo demostrar la viabilidad del modelo arquitectónico propuesto.

**Implementado completamente**:

- **Registro tipado y serialización de capacidades**: El sistema recibe múltiples capacidades declaradas, las almacena de forma tipada y las serializa a JSON con metadata completa, permitiendo que el motor de procesos sepa exactamente qué puede hacer cada dispositivo.

- **API declarativa con builders**: La API fluida permite que el código de usuario sea conciso, legible y sin lógica de negocio. Los cuatro tipos de capacidades tienen builders que se encadenan de forma natural.

- **Flujo de red con discovery UDP y ciclo de runtime**: El dispositivo implementa descubrimiento real mediante UDP broadcast, puede conectarse a WiFi, y mantiene un heartbeat que informa su estado. El logging estructura el progreso en cuatro fases (FLOW[0/4] a FLOW[4/4]).

- **Configuración declarativa con fallback**: device.json se carga en runtime; si falla, se usan valores compilados desde macros CMake.

**En etapa de especificación (marcado como TODO)**:

- **Protocolo de intercambio JSON**: Define estructura de mensajes que motor envía y cómo dispositivo responde.
- **Runtime de despacho**: Recibe mensajes de red, busca capacidades en Registry, invoca callbacks, captura resultados.

Esta aproximación es metodológicamente sólida: los TODO están claramente marcados en el código, indicando exactamente dónde va trabajo futuro, sin ocultar incertidumbre.

## 9. Conclusión para cuerpo principal

El Bunny Framework demuestra que es posible construir sistemas embebidos distribuidos con separación clara entre lógica de hardware y lógica de procesos. El núcleo arquitectónico ya está implementado: mecanismo declarativo donde el dispositivo expone capacidades tipadas con metadata rica, registro central que serializa a JSON, y descubrimiento automático en red.

La arquitectura enfatiza desacoplamiento: el firmware del ESP32 no toma decisiones de negocio, solo expone acciones y datos tipados. El motor de procesos, ejecutándose en máquinas más potentes, decide cuándo, cómo y bajo qué condiciones actuar. Esta división de responsabilidades simplifica testing, mantenimiento y escalabilidad.

El estado de madurez permite construir prototipos funcionales de dispositivos reales. Los TODOs marcados en protocolo y runtime son puntos claros de continuación para trabajo futuro, pero no bloquean la demostración del concepto fundamental.
