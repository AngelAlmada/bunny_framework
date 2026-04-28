# Motor de Procesos — Integración con Bunny Framework

## Visión General

El **Bunny Framework** en ESP32 implementa un patrón de comunicación en dos capas:

1. **UDP Discovery**: El ESP32 anuncia su presencia periódicamente
2. **WebSocket Bidireccional**: Motor de Procesos se conecta para enviar comandos y recibir eventos

```
┌─────────────────────────────────────────────────────────┐
│                    Motor de Procesos                    │
│  (Computadora/Servidor — ejecuta lógica de negocio)    │
└────────────┬──────────────────────────┬─────────────────┘
             │                          │
             │ UDP listen 5555          │ WebSocket connect
             │ parse JSON               │ ws://ip:port/path
             ▼                          ▼
┌───────────────────────────────────────────────────────────┐
│               ESP32 (Bunny Framework)                     │
│  ┌────────────────────────────────────────────────────┐   │
│  │ UDP Server (broadcast → 255.255.255.255:5555)      │   │
│  │ cada 3000ms envía JSON metadata                    │   │
│  └────────────────────────────────────────────────────┘   │
│  ┌────────────────────────────────────────────────────┐   │
│  │ WebSocket Server (HTTP en puerto 8080)            │   │
│  │ Recibe comandos JSON → ejecuta capabilities      │   │
│  │ Emite eventos JSON → motor actúa sobre el mundo  │   │
│  └────────────────────────────────────────────────────┘   │
└───────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────┐
│                    Monitor (opcional)                   │
│  (Puede conectarse al WebSocket para spy/debug)        │
└─────────────────────────────────────────────────────────┘
```

---

## 1. UDP Discovery — Cómo el Motor Descubre Dispositivos

### ESP32: Broadcast UDP cada 3000ms

**Endpoint**: `255.255.255.255:5555` (broadcast en LAN)

**Payload JSON**:
```json
{
  "bunny":true,
  "id":"esp32-001",
  "name":"Mi Dispositivo Bunny",
  "version":"0.1.0",
  "ip":"192.168.1.100",
  "webhook_port":8080,
  "webhook_path":"/bunny"
}
```

**Campos configurables** (ver `config/device.json`):
- `id`: Identificador único del dispositivo
- `name`: Nombre legible
- `version`: Versión del firmware
- `webhook_port`: Puerto donde escucha el servidor HTTP (default: 8080)
- `webhook_path`: Path del endpoint WebSocket (default: "/bunny")

### Motor de Procesos: Escuchar Broadcasts

El motor debe:

1. **Crear socket UDP escuchando en puerto 5555**:
```python
import socket
import json

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind(('0.0.0.0', 5555))

while True:
    data, addr = sock.recvfrom(512)
    try:
        packet = json.loads(data.decode())
        if packet.get('bunny'):
            print(f"Bunny device found: {packet['name']} at {packet['ip']}:{packet['webhook_port']}")
            # Conectar WebSocket aquí
    except:
        pass
```

2. **Parsear JSON y extraer datos**:
   - `ip`: IP del dispositivo en LAN
   - `webhook_port`: Puerto del servidor HTTP
   - `webhook_path`: Path del endpoint WebSocket

3. **Guardar el dispositivo detectado** para posterior conexión

---

## 2. WebSocket Connection — Cómo el Motor se Conecta

### ESP32: Server WebSocket

**URL**: `ws://{ip}:{webhook_port}{webhook_path}`

Ejemplo: `ws://192.168.1.100:8080/bunny`

**Características**:
- Servidor HTTP con soporte WebSocket (ESP-IDF httpd)
- Acepta múltiples conexiones simultáneas
- ManejManeja frames de texto (JSON)

### Motor de Procesos: WebSocket Client

El motor se conecta como cliente WebSocket:

```python
import websocket
import json

def on_message(ws, message):
    """Recibe mensajes del ESP32 (eventos, respuestas)"""
    payload = json.loads(message)
    print(f"Event received: {payload}")

def on_error(ws, error):
    print(f"WebSocket error: {error}")

def on_close(ws, close_status_code, close_msg):
    print(f"WebSocket closed")

def on_open(ws):
    """Conexión establecida"""
    print(f"Connected to Bunny device")

# Conectar
ws = websocket.WebSocketApp(
    f"ws://192.168.1.100:8080/bunny",
    on_open=on_open,
    on_message=on_message,
    on_error=on_error,
    on_close=on_close
)

ws.run_forever()
```

O con `asyncio` + `websockets`:

```python
import asyncio
import websockets
import json

async def motor_client():
    uri = "ws://192.168.1.100:8080/bunny"
    async with websockets.connect(uri) as ws:
        print("Connected to Bunny device")
        
        # Enviar comando
        await ws.send(json.dumps({
            "type": "command",
            "command": "fan_command",
            "params": {"speed": 100}
        }))
        
        # Recibir respuesta/eventos
        async for message in ws:
            event = json.loads(message)
            print(f"Event: {event}")

asyncio.run(motor_client())
```

---

## 3. Protocolo WebSocket — Formato de Mensajes

### Motor → ESP32 (Comandos)

```json
{
  "type": "command",
  "command": "fan_command",
  "params": {
    "speed": 100,
    "mode": "auto"
  }
}
```

**Flujo**:
1. Motor envía JSON con `type: "command"`
2. ESP32 Router busca la capability en Registry
3. Ejecuta `CommandCapability::execute()`
4. Responde con resultado

### ESP32 → Motor (Eventos)

```json
{
  "type": "event",
  "event": "motion_event",
  "timestamp": 1713734400,
  "data": {
    "detected": true,
    "confidence": 0.95
  }
}
```

**Flujo**:
1. Sensor o lógica emite evento en ESP32
2. EventCapability publica vía WebSocket
3. Motor recibe y actúa

### ESP32 → Motor (Sensores/Estados)

```json
{
  "type": "sensor_read",
  "sensor": "temperature_sensor",
  "value": 23.5,
  "unit": "celsius",
  "timestamp": 1713734400
}
```

```json
{
  "type": "state",
  "state": "fan_state",
  "value": 50,
  "get_result": 50,
  "set_result": "ok"
}
```

---

## 4. Estado de Conexión — FLOW

El framework mantiene un estado de conexión visible en el monitor:

```
FLOW[0/4] Waiting WiFi/LAN + motor de procesos
FLOW[1/4] WiFi connected at IP: 192.168.1.100
FLOW[2/4] UDP announce observed in network (N sent)
FLOW[3/4] Waiting for WebSocket handshake from motor de procesos
FLOW[4/4] WebSocket connection established (real)
```

### Estados:

| Estado | Significado | Motor hace... |
|--------|-------------|---------------|
| 0-1 | WiFi no listo | Espera y reintenta escuchar UDP |
| 2 | UDP funciona | Detecta dispositivo, prepara WebSocket |
| 3 | Esperando WebSocket | Motor se conecta aquí |
| 4 | Conectado | Envía comandos, recibe eventos |

En `bunny_sdk.cpp` puedes ver cómo transita entre estados:
```cpp
if (!stage1_logged && wifi_connected) {
    ESP_LOGI(TAG, "FLOW[1/4] WiFi connected at IP: %s", wifi_ip);
}
if (!stage2_logged && announces > 0) {
    ESP_LOGI(TAG, "FLOW[2/4] UDP announce observed...");
}
if (!stage4_logged && ws_connected) {
    ESP_LOGI(TAG, "FLOW[4/4] WebSocket connection established");
}
```

---

## 5. Archivo de Configuración — device.json

**Ubicación**: `config/device.json`

```json
{
  "device": {
    "id": "esp32-001",
    "name": "Mi Dispositivo Bunny",
    "description": "Dispositivo ESP32...",
    "type": "esp32",
    "version": "0.1.0"
  },
  "network": {
    "wifi": {
      "ssid": "tu-red-wifi",
      "password": "tu-contraseña",
      "auth_type": "WPA2",
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
  },
  "capabilities": {
    "sensors": [],
    "commands": [],
    "events": []
  }
}
```

**Motor debe leer**:
- `discovery.udp_port`: Puerto donde escuchar
- `webhook_port`: Puerto para WebSocket
- `webhook_path`: Path para WebSocket
- `device.id`, `device.name`: Para logging

---

## 6. Ejemplo Completo: Motor en Python

```python
#!/usr/bin/env python3
import socket
import json
import threading
import asyncio
import websockets

class BunnyMotor:
    def __init__(self, name="motor-1"):
        self.name = name
        self.devices = {}  # {device_id: device_info}
        self.connections = {}  # {device_id: websocket}
    
    def discover_devices(self):
        """Escucha broadcasts UDP del ESP32"""
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind(('0.0.0.0', 5555))
        
        print(f"[{self.name}] Listening for Bunny devices on UDP:5555...")
        
        while True:
            data, addr = sock.recvfrom(512)
            try:
                packet = json.loads(data.decode())
                if packet.get('bunny'):
                    device_id = packet['id']
                    self.devices[device_id] = packet
                    print(f"[{self.name}] Found device: {packet['name']} "
                          f"({device_id}) at {packet['ip']}:{packet['webhook_port']}")
                    
                    # Auto-connect en background
                    threading.Thread(
                        target=self.connect_device,
                        args=(device_id,),
                        daemon=True
                    ).start()
            except json.JSONDecodeError:
                pass
    
    async def connect_device(self, device_id):
        """Conecta WebSocket a un dispositivo"""
        if device_id not in self.devices:
            return
        
        info = self.devices[device_id]
        uri = f"ws://{info['ip']}:{info['webhook_port']}{info['webhook_path']}"
        
        try:
            print(f"[{self.name}] Connecting to {device_id} at {uri}...")
            async with websockets.connect(uri) as ws:
                self.connections[device_id] = ws
                print(f"[{self.name}] Connected to {device_id}")
                
                # Recibir mensajes
                async for message in ws:
                    event = json.loads(message)
                    await self.on_event(device_id, event)
        except Exception as e:
            print(f"[{self.name}] Connection error: {e}")
        finally:
            if device_id in self.connections:
                del self.connections[device_id]
    
    async def on_event(self, device_id, event):
        """Procesa evento/respuesta del ESP32"""
        event_type = event.get('type', 'unknown')
        
        if event_type == 'event':
            print(f"[{self.name}] Event from {device_id}: "
                  f"{event['event']} = {event.get('data', {})}")
        elif event_type == 'sensor_read':
            print(f"[{self.name}] Sensor {event['sensor']}: "
                  f"{event['value']} {event.get('unit', '')}")
        else:
            print(f"[{self.name}] Message from {device_id}: {event}")
    
    async def send_command(self, device_id, command_name, params=None):
        """Envía comando a un dispositivo"""
        if device_id not in self.connections:
            print(f"[{self.name}] Device {device_id} not connected")
            return False
        
        msg = {
            "type": "command",
            "command": command_name,
            "params": params or {}
        }
        
        try:
            ws = self.connections[device_id]
            await ws.send(json.dumps(msg))
            print(f"[{self.name}] Sent command to {device_id}: {command_name}")
            return True
        except Exception as e:
            print(f"[{self.name}] Send error: {e}")
            return False
    
    def run(self):
        """Inicia el motor"""
        # UDP discovery en thread separado
        threading.Thread(target=self.discover_devices, daemon=True).start()
        
        # Ejemplo: espera 5s y envía comando
        asyncio.run(self._example_commands())
    
    async def _example_commands(self):
        """Ejemplo de cómo usar el motor"""
        await asyncio.sleep(5)  # Espera a que se descubra
        
        for device_id in list(self.devices.keys()):
            print(f"\n[{self.name}] Sending test command to {device_id}...")
            await self.send_command(device_id, "fan_command", {"speed": 75})
            await asyncio.sleep(2)

if __name__ == '__main__':
    motor = BunnyMotor("motor-principal")
    motor.run()
```

---

## 7. Testing en Local

### 1. Compilar y flashear ESP32:
```bash
cd bunny_framework
idf.py build
idf.py -p /dev/ttyUSB0 flash
idf.py -p /dev/ttyUSB0 monitor
```

Verás en monitor:
```
bunny: WiFi connected! IP: 192.168.1.100
bunny: FLOW[1/4] WiFi connected at IP: 192.168.1.100
bunny: FLOW[2/4] UDP announce observed in network (1 sent)
bunny: FLOW[3/4] Waiting for WebSocket handshake...
```

### 2. En otra terminal, escuchar UDP:
```bash
nc -ul 5555
# Verás JSON periódicamente
# {"bunny":true,"id":"esp32-001","name":"Mi Dispositivo Bunny",...}
```

### 3. Conectar con wscat (cliente WebSocket):
```bash
npm install -g wscat
wscat -c ws://192.168.1.100:8080/bunny
# Escribe mensajes JSON
```

En monitor verás:
```
bunny: WebSocket message received: {"type":"command",...}
bunny: FLOW[4/4] WebSocket connection established
```

---

## 8. Checklist de Integración del Motor

- [ ] Motor crea socket UDP en puerto 5555
- [ ] Motor parsea JSON de broadcasts Bunny
- [ ] Motor extrae `ip`, `webhook_port`, `webhook_path`
- [ ] Motor conecta WebSocket a `ws://{ip}:{port}{path}`
- [ ] Motor puede recibir mensajes JSON del ESP32
- [ ] Motor puede enviar comandos JSON al ESP32
- [ ] Motor reconoce cuando ESP32 llega a FLOW[4/4]
- [ ] Motor maneja desconexiones/reconexiones
- [ ] Motor implementa handler para `type: "event"`
- [ ] Motor implementa handler para `type: "sensor_read"`

---

## 9. Conceptos Clave para el Motor

| Concepto | Descripción |
|----------|-------------|
| **Registry** | ESP32 mantiene lista de capabilities (sensores, comandos, eventos, estados) |
| **Capability** | Una funcionalidad que el dispositivo PUEDE hacer (no volitivo) |
| **Sensor** | Lee datos (temperatura, humedad) |
| **Command** | Ejecuta una acción (encender ventilador) |
| **Event** | Emite notificación (movimiento detectado) |
| **State** | Almacena valor que puede leerse/escribirse |
| **motor de procesos** | Decide CUÁNDO y CÓMO usar las capabilities |

El motor observa el mundo (sensores/eventos), decide acciones, envía comandos al ESP32.

---

## 10. Próximos Pasos

1. Implementar parsing JSON en el código actual del motor
2. Agregar reconnection logic (reintentos si cae conexión)
3. Agregar queue de comandos (si WebSocket cae, encolar)
4. Implementar heartbeat/keep-alive
5. Agregar serialización de `device_id` en mensajes (para multi-device)
