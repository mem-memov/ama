/** @file
 * @brief Expose native previews solely through host-authorized type-specific readers.
 * [PROTO-03](../../../../spec/implementation/native-interface.md#proto-03) keeps layout
 * knowledge explicit; [PAR-05](../../../../spec/language/async-and-parallelism.md#par-05) keeps reader lifetime with its provider.
 */
#include "../internal.h"
/** One synchronous typed observation; the runtime checks adapter shape and callback count. Component contract: [PROTO-03](../../../../spec/implementation/native-interface.md#proto-03), [PAR-05](../../../../spec/language/async-and-parallelism.md#par-05). */
typedef struct { ToolJsonWriter *writer; bool emitted; } NativeOutput;
/** Integer magnitude bytes and declared order remain lossless, without host-width conversion. Component contract: [PROTO-03](../../../../spec/implementation/native-interface.md#proto-03), [PAR-05](../../../../spec/language/async-and-parallelism.md#par-05). */
static bool value(void *context, const AmaDebugNativeValue *native) {
    NativeOutput *output = context;
    ToolJsonWriter *writer = output->writer;
    output->emitted = true;
    return tool_json_raw(writer, "{\"kind\":") && tool_json_unsigned(writer, native->kind)
        && tool_json_raw(writer, ",\"flag\":") && tool_json_boolean(writer, native->flag)
        && tool_json_raw(writer, ",\"negative\":") && tool_json_boolean(writer, native->negative)
        && tool_json_raw(writer, ",\"little_endian\":") && tool_json_boolean(writer, native->little_endian)
        && tool_json_raw(writer, ",\"bytes\":") && tool_json_bytes(writer, native->bytes.data, native->bytes.length)
        && tool_json_raw(writer, "}");
}
/** An absent reader is a documented unavailable view, not an invitation to dump State bytes. Component contract: [PROTO-03](../../../../spec/implementation/native-interface.md#proto-03), [PAR-05](../../../../spec/language/async-and-parallelism.md#par-05). */
bool ama_debug_client_native(AmaDebugClientOutput *output, const AmaDebugInstance *instance) {
    ToolJsonWriter *writer = output->writer;
    bool registered = false;
    for (size_t index = 0; index < output->session->reader_count; index++) {
        if (output->session->readers[index].type_id == instance->reference.local.type_id) { registered = true; break; }
    }
    if (!tool_json_raw(writer, "{\"registered\":") || !tool_json_boolean(writer, registered)
        || !tool_json_raw(writer, ",\"value\":")) { return false; }
    NativeOutput native = {.writer=writer};
    AmaStatus status = registered ? ama_debug_native(output->session, instance->reference, value, &native)
        : ama_status_success();
    if (!native.emitted && !tool_json_raw(writer, "null")) { return false; }
    if (output->status.code == AMA_OK) { output->status = status; }
    return tool_json_raw(writer, ",\"status\":") && ama_debug_client_status(writer, status)
        && tool_json_raw(writer, "}");
}
