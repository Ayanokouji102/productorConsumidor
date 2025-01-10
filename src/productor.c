#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MQTTClient.h>

// Definiciones necesarias para MQTT
#define ADDRESS "tcp://localhost:1883" // Dirección del broker MQTT
#define CLIENTID "Productor"           // ID del cliente MQTT
#define TOPIC "topic/test"             // Topic al que se publicarán los mensajes
#define QOS 1                          // Quality of Service nivel 1
#define TIMEOUT 10000L                 // Tiempo de espera para publicar

// Estructura del mensaje
typedef struct {
    int matricula;
    char nombre[50];
    char primer_apellido[50];
    char segundo_apellido[50];
    char materia[50];
    float calificacion;
    char carrera[50];
} Mensaje;

// Función para convertir la estructura en una cadena JSON-like
void mensaje_a_json(Mensaje *mensaje, char *buffer, size_t size) {
    snprintf(buffer, size,
             "{\"matricula\":%d,\"nombre\":\"%s\",\"primer_apellido\":\"%s\","
             "\"segundo_apellido\":\"%s\",\"materia\":\"%s\",\"calificacion\":%.2f,\"carrera\":\"%s\"}",
             mensaje->matricula, mensaje->nombre, mensaje->primer_apellido,
             mensaje->segundo_apellido, mensaje->materia, mensaje->calificacion,
             mensaje->carrera);
}

int main() {
    // Configuración del cliente MQTT
    MQTTClient client;
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);

    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;

    // Conexión al broker
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Error conectando al broker. Código de error: %d\n", rc);
        return EXIT_FAILURE;
    }

    printf("Conectado al broker MQTT en %s\n", ADDRESS);

    // Crear un mensaje para enviar
    Mensaje mensaje = {
        .matricula = 12345,
        .nombre = "Juan",
        .primer_apellido = "Pérez",
        .segundo_apellido = "Gómez",
        .materia = "Sistemas Operativos",
        .calificacion = 9.5,
        .carrera = "Ingeniería en Computación"
    };

    // Convertir el mensaje a formato JSON-like
    char payload[512];
    mensaje_a_json(&mensaje, payload, sizeof(payload));
    printf("Mensaje preparado: %s\n", payload);

    // Publicar el mensaje
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    pubmsg.payload = payload;
    pubmsg.payloadlen = strlen(payload);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;

    if ((rc = MQTTClient_publishMessage(client, TOPIC, &pubmsg, NULL)) != MQTTCLIENT_SUCCESS) {
        printf("Error publicando el mensaje. Código de error: %d\n", rc);
        MQTTClient_disconnect(client, TIMEOUT);
        MQTTClient_destroy(&client);
        return EXIT_FAILURE;
    }

    printf("Mensaje publicado correctamente en el topic: %s\n", TOPIC);

    // Finalizar conexión
    MQTTClient_disconnect(client, TIMEOUT);
    MQTTClient_destroy(&client);

    return EXIT_SUCCESS;
}
