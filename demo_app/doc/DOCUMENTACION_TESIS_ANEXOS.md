# Documentación de tesis (anexos técnicos)

## A. Objetivo del anexo

Este anexo amplía la evidencia de implementación del Bunny Framework a nivel técnico detallado, explicando decisiones de diseño específicas, parámetros configurables, fragmentos de código significativos y estado de madurez por componente. Está dirigido a lectores técnicos interesados en entender cómo cada módulo contribuye al objetivo general de separación entre hardware y lógica de procesos.

## B. API pública y separación C/C++

El framework mantiene una frontiera clara entre interfaz C (compatible con aplicaciones FreeRTOS/ESP-IDF tradicionales) e interfaz C++ (para módulos declarativos de usuario). Esta separación es deliberada:

### Interfaz C - Para app_main

La mayoría de proyectos ESP-IDF tienen app_main() escrita en C. Bunny no obliga migración a C++:

```c
/* components/bunny/include/bunny.h */
void bunny_begin(void);
void bunny_load_modules(void);
void bunny_loop(void);
```

Estos tres puntos de entrada definen el ciclo de vida completo desde C. El usuario incluye bunny.h en su app_main (código C puro) y el framework internamente maneja interoperabilidad con C++.

**Ventaja**: Proyectos heredados o equipos contrarios a C++ pueden usar Bunny sin presión para migrar toda la base de código.

### Interfaz C++ - Para módulos de usuario

Programadores desarrollando módulos nuevos aprovechan la API C++ fluida:

```cpp
/* components/bunny/bunny_sdk.h */
class BunnySDK {
public:
    void begin();
    void load_modules();
    void loop();

    SensorBuilder  sensor (const char* name);
    CommandBuilder command(const char* name);
    EventBuilder   event  (const char* name);
    StateBuilder   state  (const char* name, Type state_type);

    void   emit(const char* event_name);
    double read(const char* sensor_name);
};

extern bunny::BunnySDK& Bunny;
```

**Ventaja**: La API fluida en C++ permite declaraciones legibles y type-safe.

### Mecanismo de interoperabilidad

En bunny_sdk.cpp hay exportación con `extern "C"` que expone el SDK C++ a través de la interfaz C:

```cpp
extern "C" {
    void bunny_begin(void)  { Bunny.begin();         }
    void bunny_load_modules(void) { Bunny.load_modules(); }
    void bunny_loop(void)   { Bunny.loop();          }
}
```

Cuando app_main (C) llama `bunny_begin()`, entra automáticamente al objeto BunnySDK C++. Esta es una técnica estándar en sistemas mixtos C/C++.

Archivos: components/bunny/include/bunny.h y components/bunny/bunny_sdk.h

## C. Ciclo de vida y orquestación interna

El ciclo de vida está cuidadosamente orquestado en BunnySDK. Cada fase depende de la anterior y debe completarse antes de iniciar la siguiente:

### Fase 1: begin() — Inicialización de dependencias

```cpp
void BunnySDK::begin() {
    bunny_config_load();       // Lee device.json o macros compiladas
    bunny_discovery_init();    // Prepara socket UDP
    bunny_network_init();      // Inicializa red base
    ESP_LOGI(TAG, "Bunny framework initialized");
}
```

En esta fase:
- La configuración se carga en memoria global.
- Recursos de red se reservan (socket UDP).
- Subsistemas de red se inicializan (interfaces, pilas TCP/IP).

**Invariante**: Después de begin(), el framework está listo para recibir registros de capacidades pero aún no intenta conectar o descubrir.

### Fase 2: load_modules() — Espera de red y activación de descubrimiento

```cpp
void BunnySDK::load_modules() {
    const bunny_config_t* cfg = bunny_config_get();
    
    if (cfg && cfg->wifi.ssid && strlen(cfg->wifi.ssid) > 0) {
        ESP_LOGI(TAG, "Waiting for WiFi connection (timeout: %lums)...", 
                 (unsigned long)cfg->wifi.timeout_ms);
        if (bunny_wifi_connect_wait(cfg->wifi.timeout_ms)) {
            const char* ip = bunny_wifi_get_ip();
            ESP_LOGI(TAG, "WiFi connected! IP: %s", ip ? ip : "unknown");
        } else {
            ESP_LOGW(TAG, "WiFi connection timeout or failed");
        }
    }
    
    bunny_network_connect();
    bunny_discovery_start();
}
```

**Propósito**: Bloquear hasta que WiFi esté disponible (o timeout) para garantizar que el dispositivo pueda ser descubierto. El timeout es configurable y evita que el dispositivo espere indefinidamente.

### Fase 3: loop() — Heartbeat y estado

```cpp
void BunnySDK::loop() {
    bool stage1_logged = false, stage2_logged = false, 
         stage3_logged = false, stage4_logged = false,
         discovery_active = true;

    while (true) {
        uint32_t announces = bunny_discovery_announce_count();
        bool wifi_connected = bunny_wifi_connected();
        bool ws_connected = bunny_network_ws_connected();

        if (!stage1_logged && wifi_connected) {
            ESP_LOGI(TAG, "FLOW[1/4] WiFi connected");
            stage1_logged = true;
        }
        if (!stage2_logged && announces > 0) {
            ESP_LOGI(TAG, "FLOW[2/4] UDP announce observed");
            stage2_logged = true;
        }
        if (!stage3_logged && announces > 0 && !ws_connected) {
            ESP_LOGI(TAG, "FLOW[3/4] Waiting for WebSocket handshake");
            stage3_logged = true;
        }
        if (!stage4_logged && ws_connected) {
            ESP_LOGI(TAG, "FLOW[4/4] WebSocket connection established");
            stage4_logged = true;
        }

        if (ws_connected && discovery_active) {
            bunny_discovery_stop();
            discovery_active = false;
        }

        vTaskDelay(pdMS_TO_TICKS(5000));  // Heartbeat cada 5 segundos
    }
}
```

**Características**:
- Usa banderas para imprimir cada transición una sola vez.
- Emite heartbeat estructurado cada 5 segundos.
- Detiene anuncios UDP una vez WebSocket está conectado (optimización de ancho de banda).

Archivo: components/bunny/bunny_sdk.cpp

## D. Catálogo de capacidades de ejemplo

Los cuatro ejemplos en main/ muestran patrones replicables para agregar nuevas capacidades:

### Sensor: lectura de datos

```cpp
static double read_temperature_hw() {
    return 23.5;  // TODO: lectura real de ADC/I2C
}

void register_temperature_sensor() {
    Bunny.sensor("temperature")
         .description("Ambient temperature in degrees Celsius")
         .returns(NUMBER)
         .tag("environment")
         .build([]() -> double {
             return read_temperature_hw();
         });
}
```

**Patrón**: Función privada encapsula I/O real. Builder declara nombre, tipo de retorno, tags. Callback invoca lectura.

### Evento: notificación asincrónica

```cpp
void register_motion_event() {
    Bunny.event("motion_detected")
         .description("Triggered when PIR sensor detects movement")
         .tag("security")
         .build([]() {
             blink_indicator_hw();
         });
}
```

**Patrón**: Sin parámetros de entrada. Callback ejecuta acciones cuando evento se emite.

### Estado: variable tipada compartida

```cpp
static const char* s_fan_state = "OFF";

void register_fan_state() {
    Bunny.state("fanState", STRING)
         .description("Current fan relay state (ON or OFF)")
         .build(
             []() -> const char* { return s_fan_state; },
             [](const char* v)   { s_fan_state = v; }
         );
}
```

**Patrón**: Variable estática privada mantiene estado. Builder recibe getter y setter.

**Nota**: Estado NO persiste entre reinicios. Motor de procesos responsable de persistencia duradera.

Archivos: main/sensors/temperature_sensor.cpp, main/events/motion_event.cpp, main/states/fan_state.cpp

## E. Registro y búsqueda de capacidades

El Registry es Singleton que gestiona todas las capacidades:

### Estructura y límites

```cpp
static constexpr size_t MAX_CAPABILITIES = 32;

class Registry {
private:
    ICapability* _caps[MAX_CAPABILITIES] {};
    size_t       _count {0};
};
```

**Por qué arreglo estático**: Memoria predecible (256 bytes fijos), sin fragmentación de heap, determinista para embebidos.

**Límite de 32**: Dispositivos típicos usan 10-15 capacidades. Parámetro compilable si se requieren más.

### Búsqueda

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

**Complejidad**: O(n) donde n ≤ 32. Aceptable porque búsquedas ocurren al despachar (no en time-critical path).

### Registro

```cpp
bool Registry::register_capability(ICapability* cap) {
    if (!cap || _count >= MAX_CAPABILITIES) return false;
    _caps[_count++] = cap;
    return true;
}
```

Si arreglo está lleno, retorna false. No hay error fatal.

Archivos: components/bunny/registry/registry.h y components/bunny/registry/registry.cpp

## F. Manifiesto JSON de capacidades

Cuando motor se conecta, necesita saber qué expone el dispositivo. El Registry serializa a JSON:

### Estructura de serialización

```cpp
size_t Registry::serialize_capabilities(char* buf, size_t len) const {
    size_t pos = 0;
    json::append(buf, len, pos, "{");
    
    const CapabilityKind kinds[] = {
        CapabilityKind::SENSOR,
        CapabilityKind::COMMAND,
        CapabilityKind::EVENT,
        CapabilityKind::STATE,
    };
    const char* kind_keys[] = {
        "\"sensors\":",
        "\"commands\":",
        "\"events\":",
        "\"states\":",
    };
    
    // Cuatro pasadas: agrupa por tipo
    for (size_t k = 0; k < 4; ++k) {
        bool first_item = true;
        for (size_t i = 0; i < _count; ++i) {
            if (_caps[i]->kind() != kinds[k]) continue;
            // Abre array para primer item de este tipo
            if (first_item) {
                json::append(buf, len, pos, kind_keys[k]);
                json::append(buf, len, pos, "[");
                first_item = false;
            } else {
                json::append(buf, len, pos, ",");
            }
            // Cada capacidad se serializa
            char cap_buf[512] {};
            _caps[i]->serialize(cap_buf, sizeof(cap_buf));
            json::append(buf, len, pos, cap_buf);
        }
        if (!first_item) {
            json::append(buf, len, pos, "]");
        }
    }
    
    json::append(buf, len, pos, "}");
    return pos;
}
```

### Salida esperada

```json
{
  "sensors": [
    {
      "name": "temperature",
      "description": "Ambient temperature...",
      "returns": { "type": "NUMBER" }
    }
  ],
  "commands": [ ... ],
  "events": [ ... ],
  "states": [ ... ]
}
```

**Propósito**: Motor de procesos usa este JSON para validar tipos antes de enviar actuaciones, generar UI automáticamente, documentar contrato del dispositivo.

Archivo: components/bunny/registry/registry.cpp

## G. Descubrimiento y anuncio en red

El modelo publish-subscribe ligero donde dispositivo anuncia periódicamente su ubicación:

### Inicialización del socket UDP

```c
int s_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
int broadcast = 1;
setsockopt(s_sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
```

**Detalle**: `SO_BROADCAST` permite envío a 255.255.255.255 (entregado a todos en subred).

### Tarea de anuncio periódico

```c
static void discovery_task(void* arg) {
    struct sockaddr_in dest_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(cfg->discovery.udp_port),
        .sin_addr.s_addr = inet_addr("255.255.255.255"),
    };
    
    while (s_started) {
        char ip_buf[32] = "0.0.0.0";
        get_local_ip(ip_buf, sizeof(ip_buf));
        
        char payload[384];
        int written = snprintf(payload, sizeof(payload),
            "{\"bunny\":true,\"id\":\"%s\",\"name\":\"%s\",\"version\":\"%s\",\"ip\":\"%s\",\"webhook_port\":%u,\"webhook_path\":\"%s\"}",
            cfg->device.id, cfg->device.name, cfg->device.version, ip_buf,
            (unsigned)cfg->webhook.port, cfg->webhook.path);
        
        if (written >= 0) {
            sendto(s_sock, payload, (size_t)written, 0,
                   (struct sockaddr*)&dest_addr, sizeof(dest_addr));
            s_announce_count++;
        }
        
        vTaskDelay(pdMS_TO_TICKS(cfg->discovery.broadcast_interval_ms));
    }
}
```

**Payload**: ID único, nombre, versión, IP local, webhook_port, webhook_path. JSON permite parseo directo sin protocolos binarios complejos.

Archivo: components/bunny/network/discovery.c

## H. Configuración y valores por defecto

Sistema de "fallback" donde valores compilados se sobreescriben por JSON si disponible:

### Macros de build-time

```c
#ifndef BUNNY_WIFI_SSID
#define BUNNY_WIFI_SSID "TU_RED_WIFI"
#endif

#ifndef BUNNY_DISCOVERY_BROADCAST_INTERVAL_MS
#define BUNNY_DISCOVERY_BROADCAST_INTERVAL_MS 3000
#endif
```

### Estructura compilada

```c
static bunny_config_t s_config = {
    .device = { .id = BUNNY_DEVICE_ID, .name = BUNNY_DEVICE_NAME },
    .wifi = { .ssid = BUNNY_WIFI_SSID, .password = BUNNY_WIFI_PASSWORD },
    .discovery = { .enabled = BUNNY_DISCOVERY_ENABLED, .udp_port = BUNNY_DISCOVERY_UDP_PORT },
};
```

### Carga y fallback

```c
void bunny_config_load(void) {
    /* TODO: parsear device.json desde SPIFFS */
    /* Por ahora usa compilado */
}

const bunny_config_t *bunny_config_get(void) {
    return &s_config;
}
```

**Flujo**: Build (macros) → Flash (binario) → Opcionalmente JSON (SPIFFS sobrescribe) → Acceso via bunny_config_get().

**Ventaja**: Dispositivos sin SPIFFS usan compilado. Con SPIFFS, config actualizable sin recompilar.

Archivo: components/bunny/config/config.c

## I. Estado de madurez y limitaciones técnicas

### Módulos con implementación completa

| Componente | Estado | Detalle |
|-----------|--------|---------|
| Registro | ✓ Completo | Almacena, búsqueda O(n), serializa JSON |
| Builders | ✓ Completo | API fluida estable |
| Discovery UDP | ✓ Completo | Socket real, tarea FreeRTOS, broadcasts periódicos |
| Configuración | ✓ Completo | Compilada + fallback; JSON TODO |
| WiFi | ✓ Integrado | Usa esp_wifi de IDF |

### Módulos en etapa de especificación

**Protocolo**:

```c
void bunny_protocol_init(void) {
    /* TODO: parser de mensajes JSON del motor */
    /* TODO: validación de tipos en parámetros */
    /* TODO: serialización de respuestas */
}
```

Define estructura de request/response, no completamente implementado.

**Runtime/Dispatch**:

```c
void bunny_runtime_init(void) {
    /* TODO: inicializar tabla de despacho */
}

void bunny_runtime_tick(void) {
    /* TODO: recibir mensaje de red */
    /* TODO: buscar capacidad en Registry */
    /* TODO: invocar execute/read según tipo */
    /* TODO: capturar resultado, serializar, enviar respuesta */
}
```

Despacha mensajes a capacidades, no completamente implementado.

### Limitaciones conocidas y mitigación

| Limitación | Impacto | Mitigación |
|-----------|---------|-----------|
| 32 capacidades máximo | Dispositivos simples 10-20, complejos 30+ | Parámetro compilable |
| Búsqueda O(n) | No crítico si n ≤ 32 | Optimizable con hash table |
| Estado no persiste | Pierden datos en reinicio | Motor responsable de persistencia |
| Protocolo no completado | No puede despachar todavía | Estructura clara, falta solo implementar |

## J. Propuesta de conclusión para anexar en tesis

### Síntesis de implementación

El Bunny Framework ha alcanzado un punto de madurez donde es posible construir el 70% de una solución distribuida embebida: dispositivos pueden declarar capacidades tipadas, registrarse en catálogo central, serializarse a JSON para que externos los conozcan, y anunciarse en red para descubrimiento automático. El 30% restante (protocolo y despacho) está claramente especificado mediante TODO estructurado.

### Implicaciones técnicas

El framework refleja decisiones deliberadas en favor de embebidos:

- **Tipado fuerte**: Parámetros tienen tipos validados antes de despacho.
- **Metadata semántica**: Descripciones, tags, ejemplos permiten a otros (humanos o LLMs) entender capacidades sin código fuente.
- **Zero-config discovery**: Broadcast UDP automático elimina gestión manual de IPs.
- **Memory-conscious**: Arreglo estático, sin malloc/free en runtime, predecible.

### Limitaciones conocidas

Máximo de capacidades es 32 (parámetro compilable); búsqueda es O(n) pero aceptable; estado no persiste entre reinicios; protocolo aún por especificar completamente.

El estado actual es suficiente para propuestas de tesis que enfaticen arquitectura y separación de responsabilidades. Proyectos subsiguientes pueden completar protocolo/despacho sabiendo que interfaz no cambiará.
