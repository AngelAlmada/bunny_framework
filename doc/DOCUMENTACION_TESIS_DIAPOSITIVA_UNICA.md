# Documentacion de tesis (1 diapositiva)

Esta hoja resume solo el codigo mas especial y esencial del framework Bunny, evitando partes genericas.

## 1) Orquestacion de arranque (idea central)

**Que es:** El orden de arranque que separa infraestructura de red y definicion de capacidades.

**Para que sirve:** Garantiza que el dispositivo primero se prepare, luego publique lo que sabe hacer, y finalmente quede listo para operar con un motor externo.

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

## 2) Declaracion de capacidad (DSL declarativo)

**Que es:** Una capacidad tipada declarada con API fluida.

**Para que sirve:** Permite expresar "que puede hacer" el hardware sin meter logica de negocio en firmware.

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

## 3) Registro central (nucleo del contrato)

**Que es:** Un singleton que guarda y busca capacidades de forma controlada.

**Para que sirve:** Convierte modulos sueltos en un catalogo unico que luego puede serializarse para el motor.

```cpp
bool Registry::register_capability(ICapability* cap) {
    if (!cap || _count >= MAX_CAPABILITIES) return false;
    _caps[_count++] = cap;
    return true;
}

ICapability* Registry::find(const char* name, CapabilityKind kind) const {
    for (size_t i = 0; i < _count; ++i) {
        if (_caps[i]->kind() == kind && strcmp(_caps[i]->name(), name) == 0) {
            return _caps[i];
        }
    }
    return nullptr;
}
```

## 4) Descubrimiento automatico en red (zero-config)

**Que es:** Anuncio UDP periodico con identidad y endpoint del dispositivo.

**Para que sirve:** Evita configuracion manual de IP; el motor detecta dispositivos automaticamente en la red local.

```c
int written = snprintf(payload, sizeof(payload),
    "{\"bunny\":true,\"id\":\"%s\",\"name\":\"%s\",\"version\":\"%s\",\"ip\":\"%s\",\"webhook_port\":%u,\"webhook_path\":\"%s\"}",
    cfg->device.id, cfg->device.name, cfg->device.version, ip_buf,
    (unsigned)cfg->webhook.port, cfg->webhook.path);

sendto(s_sock, payload, (size_t)written, 0,
    (struct sockaddr*)&dest_addr, sizeof(dest_addr));
```

## Mensaje final para la diapositiva

Bunny propone un firmware declarativo: el ESP32 expone capacidades tipadas y el motor externo decide el flujo. Lo especial no es "hacer un endpoint", sino separar decisiones de negocio del hardware con un contrato claro, serializable y descubrible en red.