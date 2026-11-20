/** @file
 * @brief Bind move-only native ports to declared noncopyable Ama modules with ordinary failure results.
 * Entry borrows typed byte data only synchronously and gives the queue a fresh independent allocation.
 */
/** Only native payload rights move through host setup; no declared Ama field is concealed. */
static const char *channel_port_hook(const ScaffoldType *type, AbiOperation operation) {
    const char *path = type->plan->type->module->source->logical_path;
    if (strcmp(path, "/test/fixture/v1.0.0/src/sender") == 0) {
        if (operation == ABI_DEFAULT) {
            return " { void test_channel_claim_sender(AmaNativeChannelSender *); *destination=(AMA_NATIVE(State)){.address=destination}; test_channel_claim_sender(&destination->sender); return ama_status_success(); }\n";
        }
        if (operation == ABI_DESTROY) { return " { ama_native_channel_sender_drop(&state->sender); return ama_status_success(); }\n"; }
    } else if (strcmp(path, "/test/fixture/v1.0.0/src/receiver") == 0) {
        if (operation == ABI_DEFAULT) {
            return " { void test_channel_claim_receiver(AmaNativeChannelReceiver *); *destination=(AMA_NATIVE(State)){.address=destination}; test_channel_claim_receiver(&destination->receiver); return ama_status_success(); }\n";
        }
        if (operation == ABI_DESTROY) { return " { ama_native_channel_receiver_drop(&state->receiver); return ama_status_success(); }\n"; }
    }
    return NULL;
}

/** Actual send/take are distinct module entries; full is an ordinary failure, never busy refusal. */
static bool channel_port_entry(FILE *output, const ScaffoldType *type) {
    const char *path = type->plan->type->module->source->logical_path;
    const char *body = NULL;
    if (strcmp(path, "/test/fixture/v1.0.0/src/role") == 0) {
        body = "bool test_channel_role(void); results->AMA_OUTPUT(0)->payload->value=test_channel_role() ? 1 : 0;\n";
    } else if (strcmp(path, "/test/fixture/v1.0.0/src/sender") == 0) {
        body =
            "AmaByteView bytes; *status=ama_n_98bd03817466d12d563a6e9709025da3fa29b415e0e82ad0e6fa0e11f2e7414b_view(parameters->AMA_INPUT(0)->payload,&bytes);\n"
            "if(status->code!=AMA_OK) return; AmaNativeMessageBytes message={0};\n"
            "*status=ama_native_code_acquire(&services->code,&message.code); if(status->code!=AMA_OK) return;\n"
            "if(bytes.length!=0) message.data=malloc(bytes.length); message.length=bytes.length;\n"
            "if(bytes.length!=0 && message.data==NULL) { ama_native_message_bytes_drop(&message); *status=ama_status_make(AMA_OUT_OF_MEMORY,NULL,0); return; }\n"
            "if(bytes.length!=0) memcpy(message.data,bytes.data,bytes.length);\n"
            "AmaNativeChannelResult sent=ama_native_channel_send(&state->sender,&message); ama_native_message_bytes_drop(&message);\n"
            "*status=sent.status; if(status->code!=AMA_OK) return;\n"
            "if(sent.reason!=AMA_CHANNEL_ACCEPTED) { const char *description=NULL;\n"
            "switch(sent.reason) { case AMA_CHANNEL_FULL: description=\"Channel full\"; break;\n"
            "case AMA_CHANNEL_TOO_LARGE: description=\"Message too large\"; break;\n"
            "case AMA_CHANNEL_CLOSED: description=\"Channel closed\"; break;\n"
            "case AMA_CHANNEL_ABORTED: description=\"Channel aborted\"; break;\n"
            "case AMA_CHANNEL_SCHEMA_REJECTED: description=\"Message rejected\"; break;\n"
            "default: *status=ama_status_make(AMA_INVALID_STATE,NULL,0); return; }\n"
            "*status=ama_n_edcb3e348eb3d3a06b885e12e77fcb6bd0b86d265d2077a038d757a4f799f515_set(results->AMA_OUTPUT(0),(AmaByteView){(const unsigned char *)description,strlen(description)});\n"
            "if(status->code!=AMA_OK) return; }\n";
    } else if (strcmp(path, "/test/fixture/v1.0.0/src/receiver") == 0) {
        body =
            "AmaNativeMessageBytes message={0}; AmaNativeChannelResult received=ama_native_channel_take(&state->receiver,&message);\n"
            "*status=received.status; if(status->code!=AMA_OK) return;\n"
            "if(received.reason==AMA_CHANNEL_RECEIVED) *status=ama_n_98bd03817466d12d563a6e9709025da3fa29b415e0e82ad0e6fa0e11f2e7414b_replace(results->AMA_OUTPUT(0)->payload,(AmaByteView){message.data,message.length});\n"
            "else { static const unsigned char description[]=\"Channel not ready\";\n"
            "*status=ama_n_edcb3e348eb3d3a06b885e12e77fcb6bd0b86d265d2077a038d757a4f799f515_set(results->AMA_OUTPUT(1),(AmaByteView){description,sizeof(description)-1}); }\n"
            "ama_native_message_bytes_drop(&message); if(status->code!=AMA_OK) return;\n";
    } else if (strcmp(path, "/test/fixture/v1.0.0/src/check") == 0 || strcmp(path, "/test/fixture/v1.0.0/src/checkfull") == 0) {
        body = strcmp(path, "/test/fixture/v1.0.0/src/check") == 0
            ? "if(parameters->AMA_INPUT(0)->payload->failed) { *status=ama_status_make(AMA_NATIVE_FAILURE,NULL,0); return; }\n"
            : "if(!parameters->AMA_INPUT(0)->payload->failed) { *status=ama_status_make(AMA_NATIVE_FAILURE,NULL,0); return; } puts(\"full\");\n";
    }
    if (body == NULL) { return message_entry(output, type); }
    int written = fputs(body, output);
    assert(written != EOF);
    return true;
}
