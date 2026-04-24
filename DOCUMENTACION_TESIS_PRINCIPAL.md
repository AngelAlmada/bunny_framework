# Documentación de tesis (cuerpo principal)

## 1. Resumen del aporte técnico

Bunny Framework propone una arquitectura declarativa para firmware en ESP32 donde el dispositivo expone capacidades (sensores, comandos, eventos y estados), mientras que la lógica de negocio se delega al motor de procesos.

Este enfoque permite:

- Separación clara entre lógica de hardware y lógica de procesos.
- Capacidades tipadas y documentadas mediante metadata.
- Integración reproducible mediante configuración del dispositivo.

## 2. Arquitectura funcional

La arquitectura se organiza en tres capas:

1. Capa de aplicación: módulos del usuario en main/ (sensor, command, event, state).
2. Capa SDK: builders y registro central de capacidades en components/bunny/.
3. Capa de red y configuración: discovery UDP, conexión de red y parámetros del dispositivo.

## 3. Flujo de arranque del sistema

En el punto de entrada, se inicializa el framework, se registran capacidades y se entra al bucle de ejecución:

```c
void app_main(void)
{
    bunny_begin();

    register_temperature_sensor();
    register_fan_command();
    register_motion_event();
    register_fan_state();

    bunny_load_modules();
    bunny_loop();
}
```

Archivo fuente: main/bunny_framework.c

## 4. Modelo declarativo de capacidades

El SDK ofrece una API fluida para declarar comportamiento sin mezclar reglas de negocio. Ejemplo de comando:

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

Archivo fuente: main/commands/fan_command.cpp

## 5. Registro central y serialización de capacidades

Las capacidades se almacenan en un Registry singleton y se serializan para exponer el manifiesto del dispositivo:

```cpp
bool Registry::register_capability(ICapability* cap) {
    if (!cap || _count >= MAX_CAPABILITIES) return false;
    _caps[_count++] = cap;
    return true;
}
```

```cpp
size_t Registry::serialize_capabilities(char* buf, size_t len) const {
    json::append(buf, len, pos, "{");
    // ... agrupa por sensor/command/event/state
    json::append(buf, len, pos, "}");
    return pos;
}
```

Archivos fuente: components/bunny/registry/registry.h y components/bunny/registry/registry.cpp

## 6. Descubrimiento en red (UDP broadcast)

El dispositivo anuncia periódicamente su identidad y endpoint para facilitar descubrimiento por el motor de procesos:

```c
int written = snprintf(payload, sizeof(payload),
    "{\"bunny\":true,\"id\":\"%s\",\"name\":\"%s\",\"version\":\"%s\",\"ip\":\"%s\",\"webhook_port\":%u,\"webhook_path\":\"%s\"}",
    cfg->device.id, cfg->device.name, cfg->device.version, ip_buf,
    (unsigned)cfg->webhook.port, cfg->webhook.path);

sendto(s_sock, payload, (size_t)written, 0,
       (struct sockaddr*)&dest_addr, sizeof(dest_addr));
```

Archivo fuente: components/bunny/network/discovery.c

## 7. Configuración del dispositivo

La identidad y parámetros de red del nodo se controlan en configuración declarativa:

```json
{
  "device": {
    "id": "esp32-001",
    "name": "Si estas viendo esto, ya lograste conectar todo. HOLA MUNDO en Esp32"
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

Archivo fuente: config/device.json

## 8. Estado de implementación reportable en tesis

Implementado:

- Registro tipado y serialización de capacidades.
- API declarativa para sensores, comandos, eventos y estados.
- Flujo de red con discovery UDP y ciclo de runtime con heartbeat.

Pendiente (etapa de consolidación):

- Despacho completo en protocolo y runtime (marcado como TODO en módulos específicos).

## 9. Conclusión para cuerpo principal

El framework ya demuestra el núcleo arquitectónico: desacoplar firmware y lógica de negocio mediante capacidades declarativas, contrato de metadata y mecanismos de descubrimiento/red para operación distribuida.
