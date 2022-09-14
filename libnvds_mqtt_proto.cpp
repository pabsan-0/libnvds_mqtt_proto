#include <string.h>
#include <iostream>
#include "nvds_logger.h"
#include "nvds_msgapi.h"
#include "MQTTClient.h"
using namespace std;

#define NVDS_MQTT_LOG_CAT    "DSLOG:NVDS_MQTT_PROTO"
#define NVDS_MSGAPI_VERSION  "5.0"
#define NVDS_MSGAPI_PROTOCOL "MQTT"
#define QOS     0
#define TIMEOUT 10000L


bool is_valid_mqtt_connection_str(char* connection_str, string& burl, string& bport) {
    if (connection_str == NULL) {
        nvds_log(NVDS_MQTT_LOG_CAT, LOG_ERR, "mqtt connection string cant be NULL");
        return false;
    }

    string str(connection_str);
    size_t n=0;
    for(int i=0;i<str.length();i++)
        if (str[i]==';')
            n++;

    if (n < 1) {
        nvds_log(NVDS_MQTT_LOG_CAT, LOG_ERR, "MQTT connection string format is invalid.");
        return false;
    }
	char cp[25];
	strcpy(cp,connection_str);
	const char sep[2] = ";";
	burl = strtok(cp, sep);
	bport = strtok(NULL,sep);

    if (burl == "" || bport == "") {
        nvds_log(NVDS_MQTT_LOG_CAT, LOG_ERR, "Kafka connection string is invalid. hostname or port is empty\n");
        return false;
    }
    return true;
}



NvDsMsgApiHandle nvds_msgapi_connect(char* connection_str, nvds_msgapi_connect_cb_t connect_cb, char* config_path) {
    nvds_log_open();
    string burl = "", bport = "";
    if (!is_valid_mqtt_connection_str(connection_str, burl, bport))
        return NULL;

    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

    int rc;
    string address = "tcp://" + burl + ":" + bport;

    if ((rc = MQTTClient_create(&client, address.data(), "pablo@deepstream", MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS) {
        nvds_log(NVDS_MQTT_LOG_CAT, LOG_ERR, ("Failed to create client, return code " + to_string(rc) + "\n").data());
        return NULL;
    }

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        nvds_log(NVDS_MQTT_LOG_CAT, LOG_ERR, ("Failed to connect, return code " + to_string(rc) + "\n").data());
        return NULL;
    }
    nvds_log(NVDS_MQTT_LOG_CAT, LOG_INFO, "MQTT connection successful\n");

    // std::cout << "pablo: attempted to mqttconnect" << std::endl;

    return (NvDsMsgApiHandle)(client);
}

NvDsMsgApiErrorType nvds_msgapi_subscribe(NvDsMsgApiHandle h_ptr, char** topics, int num_topics, nvds_msgapi_subscribe_request_cb_t cb, void* user_ctx) {
    // std::cout << "pablo: subscribed to a topic" << std::endl;
    return NVDS_MSGAPI_OK;
}

NvDsMsgApiErrorType nvds_msgapi_send(NvDsMsgApiHandle h_ptr, char* topic, const uint8_t* payload, size_t nbuf) {
    MQTTClient* client = (MQTTClient*)h_ptr;

    // std::cout << topic << std::endl;
    // std::cout << "sync" << std::endl << std::endl;

    int rc;
    if (h_ptr == NULL) {
        nvds_log(NVDS_MQTT_LOG_CAT, LOG_ERR, "MQTT connection handle passed for send() = NULL. Send failed\n");
        return NVDS_MSGAPI_ERR;
    }
    if (topic == NULL || !strcmp(topic, "")) {
        nvds_log(NVDS_MQTT_LOG_CAT, LOG_ERR, "MQTT topic not specified.Send failed\n");
        return NVDS_MSGAPI_ERR;
    }
    if (payload == NULL || nbuf <= 0) {
        nvds_log(NVDS_MQTT_LOG_CAT, LOG_ERR, "MQTT: Either send payload is NULL or payload length <=0. Send failed\n");
        return NVDS_MSGAPI_ERR;
    }
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    pubmsg.payload = (void*)payload;
    pubmsg.payloadlen = nbuf;
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
    MQTTClient_deliveryToken token;
    if ((rc = MQTTClient_publishMessage(client, topic, &pubmsg, &token)) != MQTTCLIENT_SUCCESS) {
        nvds_log(NVDS_MQTT_LOG_CAT, LOG_DEBUG, "Failed to publish message, return code %d\n", rc);
        return NVDS_MSGAPI_ERR;
    }
    rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);


    return NVDS_MSGAPI_OK;
}

NvDsMsgApiErrorType nvds_msgapi_send_async(NvDsMsgApiHandle h_ptr, char* topic, const uint8_t* payload, size_t nbuf, nvds_msgapi_send_cb_t send_callback, void* user_ptr) {
        MQTTClient* client = (MQTTClient*)h_ptr;

    int rc;
    if (h_ptr == NULL) {
        nvds_log(NVDS_MQTT_LOG_CAT, LOG_ERR, "MQTT connection handle passed for send() = NULL. Send failed\n");
        return NVDS_MSGAPI_ERR;
    }
    if (topic == NULL || !strcmp(topic, "")) {
        nvds_log(NVDS_MQTT_LOG_CAT, LOG_ERR, "MQTT topic not specified.Send failed\n");
        return NVDS_MSGAPI_ERR;
    }
    if (payload == NULL || nbuf <= 0) {
        nvds_log(NVDS_MQTT_LOG_CAT, LOG_ERR, "MQTT: Either send payload is NULL or payload length <=0. Send failed\n");
        return NVDS_MSGAPI_ERR;
    }
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    pubmsg.payload = (void*)payload;
    pubmsg.payloadlen = nbuf;
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
    MQTTClient_deliveryToken token;
    MQTTClient_publishMessage(client, topic, &pubmsg, &token);

    return NVDS_MSGAPI_OK;
}

void nvds_msgapi_do_work(NvDsMsgApiHandle h_ptr) {
    if (h_ptr == NULL) {
        nvds_log(NVDS_MQTT_LOG_CAT, LOG_ERR, "MQTT connection handle passed for dowork() = NULL. No actions taken\n");
        return;
    }
    nvds_log(NVDS_MQTT_LOG_CAT, LOG_DEBUG, "nvds_msgapi_do_work\n");
    //****nvds_mqtt_client_poll((NvDsKafkaClientHandle*)h_ptr);
}

NvDsMsgApiErrorType nvds_msgapi_disconnect(NvDsMsgApiHandle h_ptr) {
    if (!h_ptr) {
        nvds_log(NVDS_MQTT_LOG_CAT, LOG_DEBUG, "nvds_msgapi_disconnect called with null handle\n");
        return NVDS_MSGAPI_ERR;
    }
    int rc;
    MQTTClient* client = (MQTTClient*)h_ptr;
    if ((rc = MQTTClient_disconnect(client, 10000)) != MQTTCLIENT_SUCCESS)
        nvds_log(NVDS_MQTT_LOG_CAT, LOG_DEBUG, "Failed to disconnect, return code %d\n", rc);
    MQTTClient_destroy(client);
    nvds_log_close();
    return NVDS_MSGAPI_OK;
}

char* nvds_msgapi_getversion() {
    return (char*)NVDS_MSGAPI_VERSION;
}

char* nvds_msgapi_get_protocol_name() {
    return (char*)NVDS_MSGAPI_PROTOCOL;
}

NvDsMsgApiErrorType nvds_msgapi_connection_signature(char* broker_str, char* cfg, char* output_str, int max_len) {
    return NVDS_MSGAPI_OK;
}