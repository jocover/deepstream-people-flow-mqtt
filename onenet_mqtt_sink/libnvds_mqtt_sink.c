#include <stdio.h>
#include "MQTTAsync.h"
#include "nvds_logger.h"
#include "nvds_msgapi.h"

#define NVDS_MSGAPI_PROTOCOL "ONENET_MQTT"
#define NVDS_MSGAPI_VERSION "1.0"
#define NVDS_MQTT_LOG_CAT "DSLOG:NVDS_ONTNET_MQTT"

static char *clientid = "jetson_nano";
static char *username = "JuglEXwpou";
static char *password = "version=2018-10-31&res=products%2FJuglEXwpou%2Fdevices%2Fjetson_nano&et=2147483647&method=sha1&sign=XXXXXXXXXXXXXXXXXXXXXXXXX%3D";
static char *addr = "tcp://studio-mqtt.heclouds.com:1883";

typedef struct mqtt_context {
    nvds_msgapi_connect_cb_t connect_cb;
    nvds_msgapi_send_cb_t send_cb;
    MQTTAsync client;
    int connected;
    void *send_user_ctx;
} mqtt_context;

void onDisconnect(void *context, MQTTAsync_successData *response) {
    mqtt_context *ctx = (mqtt_context *)context;
    ctx->connect_cb(context, NVDS_MSGAPI_EVT_DISCONNECT);
    ctx->connected = 0;
    printf("Successful disconnection\n");
}

void connlost(void *context, char *cause) {
    mqtt_context *ctx = (mqtt_context *)context;
    MQTTAsync client = (MQTTAsync)ctx->client;
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    int rc;
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
    printf("Reconnecting\n");
    conn_opts.keepAliveInterval = 30;
    conn_opts.username = username;
    conn_opts.password = password;
    conn_opts.cleansession = 1;
    conn_opts.MQTTVersion = MQTTVERSION_3_1_1;
    if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) {
        printf("Failed to start connect, return code %d\n", rc);
    } else {
        ctx->connected = 1;
    }
}

void onConnectFailure(void *context, MQTTAsync_failureData *response) {
    mqtt_context *ctx = (mqtt_context *)context;
    ctx->connect_cb(context, NVDS_MSGAPI_EVT_SERVICE_DOWN);
    ctx->connected = 0;
    printf("Connect failed, rc %d\n", response ? response->code : 0);
}

void onConnect(void *context, MQTTAsync_successData *response) {
    mqtt_context *ctx = (mqtt_context *)context;

    ctx->connect_cb(context, NVDS_MSGAPI_EVT_SUCCESS);
    ctx->connected = 1;

    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    int rc;

    if ((rc = MQTTAsync_subscribe(ctx->client, "$sys/JuglEXwpou/jetson_nano/thing/property/post/reply", 1, &opts)) != MQTTASYNC_SUCCESS) {
        printf("Failed to start subscribe, return code %d\n", rc);
    }

    printf("Successful connection\n");
}

void onPublish(void *context, MQTTAsync_successData *response) {
    mqtt_context *ctx = (mqtt_context *)context;
    ctx->send_cb(ctx->send_user_ctx, NVDS_MSGAPI_OK);
}

void onPublishFailure(void *context, MQTTAsync_failureData *response) {
    mqtt_context *ctx = (mqtt_context *)context;
    ctx->send_cb(ctx->send_user_ctx, NVDS_MSGAPI_ERR);
}

NvDsMsgApiHandle nvds_msgapi_connect(char *connection_str, nvds_msgapi_connect_cb_t connect_cb, char *config_path) {
    mqtt_context *ctx = (mqtt_context *)malloc(sizeof(mqtt_context));
    memset(ctx, 0, sizeof(mqtt_context));
    ctx->connect_cb = connect_cb;
    ctx->connected = 0;

    int rc;
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;

    MQTTAsync_create(&ctx->client, addr, clientid, MQTTCLIENT_PERSISTENCE_NONE, NULL);

    MQTTAsync_setCallbacks(ctx->client, NULL, connlost, NULL, NULL);

    conn_opts.keepAliveInterval = 30;
    conn_opts.username = username;
    conn_opts.password = password;
    conn_opts.cleansession = 1;
    conn_opts.MQTTVersion = MQTTVERSION_3_1_1;
    conn_opts.onSuccess = onConnect;
    conn_opts.onFailure = onConnectFailure;
    conn_opts.context = ctx;

    if ((rc = MQTTAsync_connect(ctx->client, &conn_opts)) != MQTTASYNC_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
    }

    return (NvDsMsgApiHandle)ctx;
}

NvDsMsgApiErrorType nvds_msgapi_subscribe(NvDsMsgApiHandle h_ptr, char **topics, int num_topics, nvds_msgapi_subscribe_request_cb_t cb, void *user_ctx) {
    printf("nvds_msgapi_subscribe\n");
    return NVDS_MSGAPI_OK;
}

NvDsMsgApiErrorType nvds_msgapi_send(NvDsMsgApiHandle h_ptr, char *topic, const uint8_t *payload, size_t nbuf) { return NVDS_MSGAPI_OK; }

NvDsMsgApiErrorType nvds_msgapi_send_async(NvDsMsgApiHandle h_ptr, char *topic, const uint8_t *payload, size_t nbuf, nvds_msgapi_send_cb_t cb, void *user_ctx) {
    int rc;
    if (h_ptr == NULL) {
        nvds_log(NVDS_MQTT_LOG_CAT, LOG_ERR,
                 "MQTT connection handle passed for send_async() = NULL. Send "
                 "failed\n");
        return NVDS_MSGAPI_ERR;
    }
    if (payload == NULL || nbuf <= 0) {
        nvds_log(NVDS_MQTT_LOG_CAT, LOG_ERR,
                 "mqtt: send_async() either payload is NULL or payload length <=0. "
                 "Send failed\n");
        return NVDS_MSGAPI_ERR;
    }

    if (!cb) {
        nvds_log(NVDS_MQTT_LOG_CAT, LOG_ERR, "Send callback cannot be NULL. Async send failed\n");
        return NVDS_MSGAPI_ERR;
    }

    mqtt_context *ctx = (mqtt_context *)h_ptr;
    ctx->send_cb = cb;
    ctx->send_user_ctx = user_ctx;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    opts.onSuccess = onPublish;
    opts.onFailure = onPublishFailure;
    opts.context = ctx;

    if ((rc = MQTTAsync_send(ctx->client, "$sys/JuglEXwpou/jetson_nano/thing/property/post", nbuf, payload, 1, 0, &opts)) != MQTTASYNC_SUCCESS) {
        printf("Failed to start MQTTAsync_send, return code %d\n", rc);
        return NVDS_MSGAPI_ERR;
    }

    return NVDS_MSGAPI_OK;
}

void nvds_msgapi_do_work(NvDsMsgApiHandle h_ptr) {
    if (h_ptr == NULL) {
        nvds_log(NVDS_MQTT_LOG_CAT, LOG_ERR,
                 "MQTT connection handle passed for send_async() = NULL. Send "
                 "failed\n");
    }
}

NvDsMsgApiErrorType nvds_msgapi_disconnect(NvDsMsgApiHandle h_ptr) {
    if (h_ptr == NULL) {
        nvds_log(NVDS_MQTT_LOG_CAT, LOG_ERR,
                 "MQTT connection handle passed for send_async() = NULL. Send "
                 "failed\n");
        return NVDS_MSGAPI_ERR;
    }

    mqtt_context *ctx = (mqtt_context *)h_ptr;

    MQTTAsync_destroy(&ctx->client);

    return NVDS_MSGAPI_OK;
}

char *nvds_msgapi_getversion() { return NVDS_MSGAPI_VERSION; }

char *nvds_msgapi_get_protocol_name() { return (char *)NVDS_MSGAPI_PROTOCOL; }

NvDsMsgApiErrorType nvds_msgapi_connection_signature(char *broker_str, char *cfg, char *output_str, int max_len) {

    return NVDS_MSGAPI_OK;
}
