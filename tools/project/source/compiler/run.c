/** @file
 * @brief Collect a separate compiler before accepting exact captured-source facts.
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01)
 * keeps process orchestration outside compilation;
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * rejects unsuccessful children and mismatched snapshots;
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves raw stream bytes.
 */
#include "client.h"
#include <string.h>
#include "source/path/logical.h"
#include "source/io/buffer/buffer.h"
/** Capture a complete borrowed stream into a stable owner; a failed prefix never
 * becomes a SourceText under
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04).
 */
static SourceStatus capture(SourceManager *owner, FILE *stream, const char *path, const SourceText **output) {
    if (fseek(stream, 0, SEEK_SET)) { return SOURCE_IO_ERROR; }
    SourceBuffer buffer = {0};
    SourceStatus status = source_buffer_read(owner->allocator, stream, &buffer);
    if (status == SOURCE_OK) { status = source_manager_add(owner, path, buffer.bytes, buffer.length, output); }
    owner->allocator.release(owner->allocator.context, buffer.bytes);
    return status;
}
SourceCompilerStatus source_compiler_client_run(SourceCompilerClient *client, const char *compiler,
    const char *request_path, const DeliverySourceSnapshotView *view, const char *entry) {
    if (!client || client->attempted || !compiler || !*compiler || !request_path || !*request_path || !view
        || (entry && !module_path_canonical((const unsigned char *)entry, strlen(entry)))) {
        return SOURCE_COMPILER_INVALID;
    }
    client->attempted = true;
    FILE *output = tmpfile(), *errors = tmpfile();
    client->status = SOURCE_COMPILER_PROCESS;
    if (output && errors) {
        char *arguments[] = {(char *)compiler, "--source-domain", "--snapshot", (char *)request_path,
            entry ? "--probe-entry" : NULL, (char *)entry, NULL};
        bool collected = project_process(arguments, NULL, output, errors, &client->process);
        if (collected) {
            client->capture_status = capture(&client->raw, errors, "compiler.stderr", &client->diagnostics);
            if (client->capture_status == SOURCE_OK) {
                client->capture_status = capture(&client->raw, output, "compiler.stdout", &client->output);
            }
            if (client->capture_status != SOURCE_OK) { client->status = SOURCE_COMPILER_CAPTURE; }
            else if (client->process.error || client->process.signal
                || (client->process.exit_code && !(entry && client->process.exit_code == DELIVERY_SOURCE_PROBE_OPEN_EXIT))) {
                client->status = SOURCE_COMPILER_CHILD;
            } else { client->status = SOURCE_COMPILER_OK; }
        }
    }
    bool closed = true;
    if (output && fclose(output)) { closed = false; }
    if (errors && fclose(errors)) { closed = false; }
    if (!closed) { client->status = SOURCE_COMPILER_CAPTURE; client->capture_status = SOURCE_IO_ERROR; }
    if (client->status != SOURCE_COMPILER_OK) { return client->status; }
    if (delivery_source_response_read(&client->response, client->output) != SOURCE_JSON_OK) {
        client->status = SOURCE_COMPILER_PROTOCOL;
    } else if (!delivery_source_response_matches(&client->response, view)) {
        client->status = SOURCE_COMPILER_MISMATCH;
    } else {
        client->valid = true;
        client->probe_state = !entry ? DELIVERY_PROBE_NONE
            : client->process.exit_code == DELIVERY_SOURCE_PROBE_OPEN_EXIT ? DELIVERY_PROBE_OPEN : DELIVERY_PROBE_EXECUTABLE;
    }
    return client->status;
}
