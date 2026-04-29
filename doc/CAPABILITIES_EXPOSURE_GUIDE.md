# Guia de Exposicion de Capacidades BCP

Esta guia define como Bunny expone al motor las capacidades registradas por el desarrollador y como el motor debe consumirlas.

---

## 1. Objetivo

Permitir que el motor descubra de manera determinista el contrato funcional del dispositivo (sensores, comandos, eventos y estados) sin inspeccionar firmware.

---

## 2. Momento de exposicion

Bunny envia el manifiesto de capacidades automaticamente justo despues de responder `handshake_ack`.

Flujo:

1. Motor envia `handshake_init`.
2. Bunny responde `handshake_ack`.
3. Bunny envia `capabilities_manifest`.

Regla:

- El motor SHOULD esperar `capabilities_manifest` despues del handshake para inicializar su mapa de acciones.

---

## 3. Formato del mensaje

Tipo: `capabilities_manifest`

Direccion: ESP32 -> Motor

```json
{
  "type": "capabilities_manifest",
  "status": "ok",
  "device_id": "esp32-001",
  "protocol_version": "0.1.0",
  "capabilities": {
    "sensors": [],
    "commands": [],
    "events": [],
    "states": []
  }
}
```

---

## 4. Solicitud explicita desde el motor

El motor puede solicitar un refresh del manifiesto enviando:

```json
{
  "type": "capabilities_request"
}
```

Respuesta de Bunny:

- `capabilities_manifest` con el mismo formato definido.

---

## 5. Estructura interna de capabilities

El objeto `capabilities` agrupa por tipo:

- `sensors`
- `commands`
- `events`
- `states`

Cada item incluye metadata declarada por el desarrollador, por ejemplo:

- `name`
- `kind`
- `description`
- `returns`
- `params`
- `tags`
- `affects`
- `example`

---

## 6. Reglas para el motor

- El motor MUST validar que `type=capabilities_manifest`.
- El motor MUST validar que `capabilities` es objeto JSON.
- El motor SHOULD cachear el manifiesto por `device_id`.
- El motor MAY pedir `capabilities_request` tras reconexion para refrescar estado.

---

## 7. Casos de error

Si Bunny no puede serializar capacidades o enviar el manifiesto:

- la sesion se considera degradada,
- el motor debe aplicar su politica normal de timeout/reconexion.

---

## 8. Referencias

- Especificacion BCP: [BCP_SPECIFICATION.md](BCP_SPECIFICATION.md)
- Guia de handshake: [HANDSHAKE_IMPLEMENTATION_GUIDE.md](HANDSHAKE_IMPLEMENTATION_GUIDE.md)
- Implementacion en firmware: [../components/bunny/network/network.c](../components/bunny/network/network.c)
- Serializacion de capacidades: [../components/bunny/registry/registry.cpp](../components/bunny/registry/registry.cpp)
