#include <json-glib/json-glib.h>
#include <nvdsmeta.h>
#include <stdio.h>
#include "nvmsgconv.h"

#define MAX_STEAM 8

typedef struct NvDsPeopleNet {
    guint32 count;
    guint32 stream_id;
} NvDsPeopleNet;

typedef struct custon_msg {
    guint32 stream_stat[MAX_STEAM];
    gint32 interval;
    struct timespec ts;
    guint32 flag;
} custon_msg;

NvDsMsg2pCtx *nvds_msg2p_ctx_create(const gchar *file, NvDsPayloadType type) {
    NvDsMsg2pCtx *ctx = (NvDsMsg2pCtx *)g_malloc(sizeof(NvDsMsg2pCtx));
    custon_msg *data = (custon_msg *)g_malloc(sizeof(custon_msg));

    memset(data, 0, sizeof(custon_msg));

    data->interval = 20;  // 20 sec

    clock_gettime(CLOCK_REALTIME, &data->ts);

    ctx->privData = data;

    ctx->payloadType = type;

    printf("nvds_msg2p_ctx_create\n");
    return ctx;
}

void nvds_msg2p_ctx_destroy(NvDsMsg2pCtx *ctx) {
    g_free(ctx->privData);
    g_free(ctx);
    printf("nvds_msg2p_ctx_destroy\n");
}

NvDsPayload *nvds_msg2p_generate(NvDsMsg2pCtx *ctx, NvDsEvent *events,
                                 guint size) {
    NvDsPayload *payload = (NvDsPayload *)g_malloc0(sizeof(NvDsPayload));

    NvDsEventMsgMeta *meta = events->metadata;
    custon_msg *data = (custon_msg *)ctx->privData;

    if (ctx->payloadType == NVDS_PAYLOAD_CUSTOM) {
        if (meta->objType == NVDS_OBJECT_TYPE_CUSTOM) {
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);

            NvDsEventMsgMeta *meta = events->metadata;
            NvDsPeopleNet *obj = (NvDsPeopleNet *)meta->extMsg;
            data->stream_stat[obj->stream_id] = obj->count;

            data->flag |= 1 << meta->sensorId;

            if (data->ts.tv_sec + data->interval < ts.tv_sec) {
                data->ts = ts;
                // flag == 0xff all stream info update finish

                if (data->flag == 0xff) {
                    data->flag = 0;
                }
                int total = 0;
                for (int i = 0; i < MAX_STEAM; i++) {
                    total += data->stream_stat[i];
                }

                char packet_id[18] = {0};
                sprintf(packet_id, "%ld", ts.tv_sec);

                JsonObject *pRoot = json_object_new();
                json_object_set_string_member(pRoot, "id", packet_id);
                json_object_set_string_member(pRoot, "version", "1.0");
                JsonObject *pParams = json_object_new();
                json_object_set_object_member(pRoot, "params", pParams);

                JsonObject *people_flow = json_object_new();
                json_object_set_object_member(pParams, "people_flow", people_flow);

                JsonObject *flow_struct = json_object_new();
                for (int i = 0; i < MAX_STEAM; i++) {
                    json_object_set_int_member(flow_struct, g_strdup_printf("count%d", i), data->stream_stat[i]);
                }

                json_object_set_object_member(people_flow, "value", flow_struct);

                JsonObject *people_total = json_object_new();
                json_object_set_object_member(pParams, "people_total", people_total);
                json_object_set_int_member(people_total, "value", total);

                JsonNode *root = json_node_new(JSON_NODE_OBJECT);
                JsonGenerator *gen = json_generator_new();
                json_node_take_object(root, pRoot);
                json_generator_set_root(gen, root);

                payload->payload = json_generator_to_data(gen, &payload->payloadSize);

                json_node_free(root);
            }
        }
    } else {
        payload = NULL;
    }

    return payload;
}

NvDsPayload **nvds_msg2p_generate_multiple(NvDsMsg2pCtx *ctx, NvDsEvent *events,
                                           guint eventSize,
                                           guint *payloadCount) {
    printf("nvds_msg2p_generate_multiple\n");
    return NULL;
}

NvDsPayload *nvds_msg2p_generate_new(NvDsMsg2pCtx *ctx, void *metadataInfo) {
    printf("nvds_msg2p_generate_new\n");
    /*
    gchar *message = NULL;
    gint len = 0;
    NvDsMsg2pMetaInfo *meta_info = (NvDsMsg2pMetaInfo *)metadataInfo;
    NvDsFrameMeta *frame_meta = (NvDsFrameMeta *)meta_info->frameMeta;
    NvDsObjectMeta *obj_meta = (NvDsObjectMeta *)meta_info->objMeta;
  */
    return NULL;
}

NvDsPayload **nvds_msg2p_generate_multiple_new(NvDsMsg2pCtx *ctx,
                                               void *metadataInfo,
                                               guint *payloadCount) {
    printf("nvds_msg2p_generate_multiple_new\n");
    return NULL;
}

void nvds_msg2p_release(NvDsMsg2pCtx *ctx, NvDsPayload *payload) {
    g_free(payload->payload);
    g_free(payload);
}
