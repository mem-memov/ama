/** @file
 * @brief Execute explicit debugger commands, acknowledge observations and always dispose the borrowed host.
 * [RT-16](../../../spec/implementation/runtime.md#rt-16) requires explicit execution
 * boundaries; [ERROR-02](../../../spec/implementation/diagnostics.md#error-02) separates technical failure from refusal.
 */
#include <stdlib.h>
#include <inttypes.h>
#include "internal.h"
/** A stable spelling table is exhaustive for the closed command enumeration. */
static const char *name(AmaDebugClientCommand command) {
    switch (command) {
        case AMA_DEBUG_CLIENT_RUN: return "run";
        case AMA_DEBUG_CLIENT_RESUME: return "resume";
        case AMA_DEBUG_CLIENT_INSPECT: return "inspect";
        case AMA_DEBUG_CLIENT_DISPOSE: return "dispose";
        case AMA_DEBUG_CLIENT_QUIT: return "quit";
        default: return "invalid";
    }
}
/** This reports a command status without modifying the target's program/refusal status. */
static bool command(const AmaDebugClientHost *host, AmaDebugClientCommand request, ToolJsonWriter *writer, AmaStatus *status) {
    if (!tool_json_raw(writer, "{\"schema\":\"ama.debugger.v1\",\"command\":")
        || !tool_json_string(writer, name(request)) || !tool_json_raw(writer, ",\"data\":")) { return false; }
    *status = ama_status_success();
    bool rendered;
    if (request == AMA_DEBUG_CLIENT_INSPECT) {
        rendered = ama_debug_client_inspect(host, writer, status);
    } else if (request == AMA_DEBUG_CLIENT_RUN || request == AMA_DEBUG_CLIENT_RESUME) {
        AmaContextOutcome result = host->execute(host->context,
            request == AMA_DEBUG_CLIENT_RUN ? AMA_ACCESS_ENTRY : AMA_ACCESS_RESUME);
        *status = result.status;
        const char *outcome = result.outcome == AMA_HOST_COMPLETED ? "completed"
            : result.outcome == AMA_HOST_YIELDED ? "yielded" : "failed";
        rendered = tool_json_raw(writer, "{\"outcome\":") && tool_json_string(writer, outcome)
            && tool_json_raw(writer, ",\"root\":")
            && ama_debug_client_reference(writer, result.root)
            && tool_json_raw(writer, "}");
    } else {
        if (request == AMA_DEBUG_CLIENT_DISPOSE) { *status = host->dispose(host->context); }
        if (request == AMA_DEBUG_CLIENT_INVALID) { *status = ama_status_make(AMA_INVALID_STATE, "Invalid debugger command", 24); }
        rendered = tool_json_raw(writer, "null");
    }
    return rendered && tool_json_raw(writer, ",\"status\":") && ama_debug_client_status(writer, *status)
        && tool_json_raw(writer, "}\n");
}
/** No command follows a failed acknowledgement; nevertheless cleanup still consumes host resources. */
int ama_debug_client_run(const AmaDebugClientHost *host, FILE *input, FILE *output, FILE *errors) {
    if (host == NULL || host->execute == NULL || host->begin == NULL || host->dispose == NULL
        || input == NULL || output == NULL || errors == NULL) { return EXIT_FAILURE; }
    ToolJsonWriter writer;
    tool_json_init(&writer, output);
    bool success = true;
    for (;;) {
        AmaDebugClientCommand request = ama_debug_client_read(input);
        if (request == AMA_DEBUG_CLIENT_END) { break; }
        if (request == AMA_DEBUG_CLIENT_IO) {
            (void)fputs("Debugger command input failed\n", errors);
            success = false;
            break;
        }
        AmaStatus status = ama_status_success();
        bool rendered = command(host, request, &writer, &status);
        bool flushed = tool_json_finish(&writer);
        if (!rendered || !flushed) {
            (void)fputs("Debugger output failed; completed execution is not rolled back\n", errors);
            success = false;
            break;
        }
        if (status.code != AMA_OK) { success = false; }
        if (request == AMA_DEBUG_CLIENT_QUIT) { break; }
    }
    AmaStatus disposed = host->dispose(host->context);
    if (disposed.code != AMA_OK) {
        (void)fprintf(errors, "Debugger host final status is unsuccessful (code %" PRIu32 ")\n", disposed.code);
        success = false;
    }
    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
