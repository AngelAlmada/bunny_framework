#include "discovery.h"
#include "../config/config.h"

void bunny_discovery_init(void)
{
    /* TODO:
       1. Obtener bunny_config_get() para leer id, name, webhook port/path
       2. Crear socket UDP (SOCK_DGRAM)
       3. Configurar broadcast (SO_BROADCAST)
       4. Preparar el mensaje JSON de anuncio con los datos del dispositivo
    */
}

void bunny_discovery_start(void)
{
    /* TODO:
       1. Lanzar tarea FreeRTOS que cada discovery.broadcast_interval_ms
          envíe el mensaje JSON por UDP broadcast al puerto discovery.udp_port
       2. El mensaje incluye: id, name, version, ip local, webhook_port, webhook_path
    */
}

void bunny_discovery_stop(void)
{
    /* TODO: detener la tarea FreeRTOS y cerrar el socket */
}
