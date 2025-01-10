#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mosquitto.h>
#include <mysql/mysql.h>  // Para la conexión con la base de datos MariaDB

// Función callback que se ejecuta cuando se recibe un mensaje
void mensaje_recibido(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message) {
    // Lógica de procesamiento del mensaje
    printf("Mensaje recibido: %s\n", (char *)message->payload);

    // Aquí debes realizar la conexión a la base de datos y almacenar el mensaje
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;
    
    conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "Error en mysql_init(): %s\n", mysql_error(conn));
        return;
}
    // Conectar a la base de datos (ajusta estos parámetros)
    if (mysql_real_connect(conn, "localhost", "root", "root", "productor_consumidor", 0, NULL, 0) == NULL) {
        fprintf(stderr, "Error de conexión: %s\n", mysql_error(conn));
        mysql_close(conn);
        return;
    }

    // Crear la consulta para insertar el mensaje en la base de datos
    char query[1024];
    snprintf(query, sizeof(query), "INSERT INTO mensajes2 (mensaje) VALUES ('%s')", (char*)message->payload);

    // Ejecutar la consulta
    if (mysql_query(conn, query)) {
        fprintf(stderr, "Error al insertar mensaje: %s\n", mysql_error(conn));
    }

    // Cerrar la conexión
    mysql_close(conn);
}

int main() {
    struct mosquitto *client;
    int rc;

    // Inicializar la biblioteca Mosquitto
    mosquitto_lib_init();

    // Crear un cliente MQTT
    client = mosquitto_new(NULL, true, NULL);
    if (client == NULL) {
        fprintf(stderr, "Error al crear el cliente Mosquitto\n");
        return 1;
    }

    // Configurar las callbacks
    mosquitto_connect_callback_set(client, NULL);
    mosquitto_message_callback_set(client, mensaje_recibido);

    // Conectar al broker Mosquitto
    rc = mosquitto_connect(client, "localhost", 1883, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Error al conectar al broker: %s\n", mosquitto_strerror(rc));
        return 1;
    }

    // Suscribirse al topic
    rc = mosquitto_subscribe(client, NULL, "topic/test", 0);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Error al suscribirse al topic: %s\n", mosquitto_strerror(rc));
        return 1;
    }

    // Mantener la conexión y esperar por mensajes
    mosquitto_loop_forever(client, -1, 1);

    // Liberar recursos
    mosquitto_destroy(client);
    mosquitto_lib_cleanup();

    return 0;
}
