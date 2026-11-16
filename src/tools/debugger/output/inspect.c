/** @file
 * @brief Acquire and release an exact checked observation before any subsequent execution or stream acknowledgement.
 * [PAR-02](../../../../spec/language/async-and-parallelism.md#par-02) excludes concurrent
 * execution; [DEBUG-03](../../../../spec/implementation/diagnostics.md#debug-03) leaves terminated storage absent.
 */
#include "../internal.h"
/** Context status and identities are independent from an inspection command's own result. */
static bool state(ToolJsonWriter *writer, const AmaDebugContext *context) {
    return tool_json_raw(writer, "{\"identity\":") && tool_json_unsigned(writer, context->context)
        && tool_json_raw(writer, ",\"phase\":") && tool_json_unsigned(writer, context->phase)
        && tool_json_raw(writer, ",\"root\":") && ama_debug_client_reference(writer, context->root)
        && tool_json_raw(writer, ",\"current\":") && ama_debug_client_reference(writer, context->current)
        && tool_json_raw(writer, ",\"failed\":") && tool_json_boolean(writer, context->failed)
        && tool_json_raw(writer, ",\"status\":") && ama_debug_client_status(writer, context->status)
        && tool_json_raw(writer, ",\"type_count\":") && tool_json_unsigned(writer, context->type_count)
        && tool_json_raw(writer, ",\"constant_count\":") && tool_json_unsigned(writer, context->constant_count)
        && tool_json_raw(writer, "}");
}
/** Even a failed write retires the session before returning; no callback outlives its lease. */
bool ama_debug_client_inspect(const AmaDebugClientHost *host, ToolJsonWriter *writer, AmaStatus *status) {
    AmaDebugSession session = {0};
    *status = host->begin(host->context, &session);
    if (status->code != AMA_OK) { return tool_json_raw(writer, "null"); }
    AmaDebugContext context = {0};
    *status = ama_debug_context(&session, &context);
    bool rendered = status->code == AMA_OK ? tool_json_raw(writer, "{\"context\":") && state(writer, &context)
        && tool_json_raw(writer, ",\"instances\":[") : tool_json_raw(writer, "null");
    if (status->code == AMA_OK) {
        AmaDebugClientOutput output = {.writer=writer, .session=&session, .first=true};
        if (rendered) {
            *status = ama_debug_instances(&session, ama_debug_client_instance, &output);
            if (status->code == AMA_OK) { *status = output.status; }
        }
        rendered = rendered && tool_json_raw(writer, "]}");
    }
    AmaStatus ended = ama_debug_end(&session);
    if (status->code == AMA_OK) { *status = ended; }
    return rendered;
}
