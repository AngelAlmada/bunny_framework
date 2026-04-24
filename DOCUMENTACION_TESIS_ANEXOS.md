# Documentación de tesis (anexos técnicos)

## A. Objetivo del anexo

Este anexo amplía la evidencia de implementación del Bunny Framework, detallando decisiones de diseño, fragmentos de código y estado de madurez por componente.

## B. API pública y separación C/C++

Se mantiene compatibilidad con app_main en C y módulos declarativos en C++:

```c
void bunny_begin(void);
void bunny_load_modules(void);
void bunny_loop(void);
```

Archivo: components/bunny/include/bunny.h

```cpp
class BunnySDK {
public:
    void begin();
    void load_modules();
    void loop();

    SensorBuilder  sensor (const char* name);
    CommandBuilder command(const char* name);
    EventBuilder   event  (const char* name);
    StateBuilder   state  (const char* name, Type state_type);
};
```

Archivo: components/bunny/bunny_sdk.h

## C. Ciclo de vida y orquestación interna

El SDK centraliza inicialización de configuración, red y discovery:

```cpp
void BunnySDK::begin() {
    bunny_config_load();
    bunny_discovery_init();
    bunny_network_init();
}
```

```cpp
void BunnySDK::load_modules() {
    const bunny_config_t* cfg = bunny_config_get();
    if (cfg && cfg->wifi.ssid && strlen(cfg->wifi.ssid) > 0) {
        bunny_wifi_connect_wait(cfg->wifi.timeout_ms);
    }
    bunny_network_connect();
    bunny_discovery_start();
}
```

Archivo: components/bunny/bunny_sdk.cpp

## D. Catálogo de capacidades de ejemplo

Sensor:

```cpp
Bunny.sensor("temperature")
     .description("Ambient temperature in degrees Celsius")
     .returns(NUMBER)
     .tag("environment")
     .build([]() -> double {
         return read_temperature_hw();
     });
```

Archivo: main/sensors/temperature_sensor.cpp

Evento:

```cpp
Bunny.event("motion_detected")
     .description("Triggered when the PIR sensor detects movement")
     .tag("security")
     .build([]() {
         blink_indicator_hw();
     });
```

Archivo: main/events/motion_event.cpp

Estado:

```cpp
static const char* s_fan_state = "OFF";

Bunny.state("fanState", STRING)
     .description("Current fan relay state (ON or OFF)")
     .build(
         []() -> const char* { return s_fan_state; },
         [](const char* v)   { s_fan_state = v; }
     );
```

Archivo: main/states/fan_state.cpp

## E. Registro y búsqueda de capacidades

Límite y búsqueda en estructura singleton:

```cpp
static constexpr size_t MAX_CAPABILITIES = 32;

ICapability* Registry::find(const char* name, CapabilityKind kind) const {
    for (size_t i = 0; i < _count; ++i) {
        if (_caps[i]->kind() == kind && strcmp(_caps[i]->name(), name) == 0) {
            return _caps[i];
        }
    }
    return nullptr;
}
```

Archivos: components/bunny/registry/registry.h y components/bunny/registry/registry.cpp

## F. Manifiesto JSON de capacidades

Serialización por grupos para exponer contrato al motor de procesos:

```cpp
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
```

Archivo: components/bunny/registry/registry.cpp

## G. Descubrimiento y anuncio en red

Loop de anuncio periódico por broadcast:

```c
while (s_started) {
    char payload[384];
    int written = snprintf(payload, sizeof(payload), "{...}");
    if (written >= 0) {
        sendto(s_sock, payload, (size_t)written, 0,
               (struct sockaddr*)&dest_addr, sizeof(dest_addr));
    }
    vTaskDelay(pdMS_TO_TICKS(cfg->discovery.broadcast_interval_ms));
}
```

Archivo: components/bunny/network/discovery.c

## H. Configuración y valores por defecto

Fallback de configuración compilada, útil para pruebas iniciales:

```c
#ifndef BUNNY_WIFI_SSID
#define BUNNY_WIFI_SSID "TU_RED_WIFI"
#endif

#ifndef BUNNY_WIFI_TIMEOUT_MS
#define BUNNY_WIFI_TIMEOUT_MS 10000
#endif
```

```c
static bunny_config_t s_config = {
    .device = { .id = BUNNY_DEVICE_ID, .name = BUNNY_DEVICE_NAME },
    .discovery = { .enabled = BUNNY_DISCOVERY_ENABLED, .udp_port = BUNNY_DISCOVERY_UDP_PORT },
    .webhook = { .port = BUNNY_WEBHOOK_PORT, .path = BUNNY_WEBHOOK_PATH },
};
```

Archivo: components/bunny/config/config.c

## I. Estado de madurez y limitaciones técnicas

Módulos con implementación base completa:

- Registro de capacidades y serialización JSON.
- API declarativa con builders.
- Discovery UDP real con socket y tarea FreeRTOS.

Módulos en evolución:

- Protocolo de intercambio con motor de procesos:

```c
void bunny_protocol_init(void)
{
    /* TODO */
}
```

Archivo: components/bunny/protocol/protocol.c

- Runtime de despacho:

```c
void bunny_runtime_init(void)
{
    /* TODO */
}

void bunny_runtime_tick(void)
{
    /* TODO */
}
```

Archivo: components/bunny/runtime/runtime.c

## J. Propuesta de texto breve para anexar en tesis

El estado actual del framework evidencia la viabilidad del modelo declarativo en entornos embebidos: el firmware expone capacidades tipadas y metadata estructurada, mientras que la lógica de negocio queda desacoplada en un motor externo. La madurez funcional es alta en descubrimiento, registro y composición de capacidades, con trabajo restante focalizado en protocolo y dispatcher para completar el ciclo end-to-end.
