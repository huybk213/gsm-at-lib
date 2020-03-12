/*
 * MQTT client API example with GSM device.
 *
 * Once device is connected to network,
 * it will try to connect to mosquitto test server and start the MQTT.
 *
 * If successfully connected, it will publish data to "gsm_mqtt_topic" topic every x seconds.
 *
 * To check if data are sent, you can use mqtt-spy PC software to inspect
 * test.mosquitto.org server and subscribe to publishing topic
 */

#include "stdafx.h"
#include "gsm/apps/gsm_mqtt_client_api.h"
#include "mqtt_client_api.h"
#include "gsm/gsm_mem.h"
#include "gsm/gsm_network_api.h"


/**
 * \brief           Connection information for MQTT CONNECT packet
 */
static const gsm_mqtt_client_info_t
mqtt_client_info = {
    .keep_alive = 30,

    /* Server login data */
    .user = "",
    .pass = "",

    /* Device identifier address */
    .id = "HuyTV",
};

/**
 * \brief           Memory for temporary topic
 */
static char
mqtt_topic_str[256];

/**
 * \brief           Generate random number and write it to string
 * \param[out]      str: Output string with new number
 */
void
generate_random(char* str) {
    static uint32_t random_beg = 0x8916;
    random_beg = random_beg * 0x00123455 + 0x85654321;
    sprintf(str, "%u", (unsigned)((random_beg >> 8) & 0xFFFF));
}

/**
 * \brief           MQTT client API thread
 */
void
mqtt_client_api_thread(void const* arg) {
    gsm_mqtt_client_api_p client;
    gsm_mqtt_conn_status_t conn_status;
    gsm_mqtt_client_api_buf_p buf;
    gsmr_t res;
    char random_str[10] = {"1234"};

    /* Request network attach */
    while (gsm_network_request_attach() != gsmOK) {
        gsm_delay(1000);
    }

    /* Create new MQTT API */
    client = gsm_mqtt_client_api_new(256, 128);
    if (client == NULL) {
        goto terminate;
    }

    while (1) {
        /* Make a connection */
        printf("Joining MQTT server\r\n");

        /* Try to join */
        conn_status = gsm_mqtt_client_api_connect(client, "broker.hivemq.com", 1883, &mqtt_client_info);
        if (conn_status == GSM_MQTT_CONN_STATUS_ACCEPTED) {
            printf("Connected and accepted!\r\n");
            printf("Client is ready to subscribe and publish to new messages\r\n");
        } else {
            printf("Connect API response: %d\r\n", (int)conn_status);
            gsm_delay(5000);
            continue;
        }

        /* Subscribe to topics */
        sprintf(mqtt_topic_str, "%s", "HuyTV/123");
        if (gsm_mqtt_client_api_subscribe(client, mqtt_topic_str, GSM_MQTT_QOS_AT_LEAST_ONCE) == gsmOK) {
            printf("Subscribed to topic\r\n");
        } else {
            printf("Problem subscribing to topic!\r\n");
        }

        while (1) {
            /* Receive MQTT packet with 1000ms timeout */
            res = gsm_mqtt_client_api_receive(client, &buf, 5000);
            if (res == gsmOK) {
                if (buf != NULL) {
                    printf("Publish received!\r\n");
                    printf("Topic: %s, payload: %s\r\n", buf->topic, buf->payload);
                    gsm_mqtt_client_api_buf_free(buf);
                    buf = NULL;
                }
            } else if (res == gsmCLOSED) {
                printf("MQTT connection closed!\r\n");
                break;
            } else if (res == gsmTIMEOUT) {
                //printf("Timeout on MQTT receive function. Manually publishing.\r\n");

                /* Publish data on channel 1 */
                //generate_random(random_str);
                sprintf(mqtt_topic_str, "%s", "HuyTV/456");
                gsm_mqtt_client_api_publish(client, mqtt_topic_str, "Hihi", strlen("Hihi"), GSM_MQTT_QOS_AT_LEAST_ONCE, 0);
            }
        }
        goto terminate;
    }

terminate:
    gsm_mqtt_client_api_delete(client);
    gsm_network_request_detach();
    printf("MQTT client thread terminate\r\n");
    gsm_sys_thread_terminate(NULL);
}
