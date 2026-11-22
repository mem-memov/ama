/** @file
 * @brief Publish one coherent unsealed source-domain response after complete decoding.
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01)
 * keeps result consumption independent from process/compiler owners;
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) rejects unknown
 * schema or execution-readiness claims; [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * retires syntax scratch before retaining plain result data.
 */
#include <string.h>
#include "internal.h"
/** A closed schema never silently ignores future fields or readiness flags under
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04).
 */
static SourceJsonStatus decode(DeliveryResponseReader *reader, const SourceJsonValue *root) {
    const char *const names[] = {"format", "inputsSealed", "versions", "sources", "definitions"};
    const SourceJsonValue *fields[5];
    SourceJsonStatus status = source_json_schema_fields(root, names, 5, fields, &reader->owner->issue);
    if (status != SOURCE_JSON_OK) { return status; }
    const char *format = NULL;
    status = source_json_schema_cstring(fields[0], &format, &reader->owner->issue);
    if (status != SOURCE_JSON_OK) { return status; }
    if (strcmp(format, "ama.source-domain.v1") || fields[1]->kind != SOURCE_JSON_BOOLEAN || fields[1]->boolean) {
        return delivery_response_error(reader, root, "Unknown source-domain format or unsupported sealed-input claim");
    }
    status = delivery_response_versions(reader, fields[2]);
    if (status == SOURCE_JSON_OK) { status = delivery_response_sources(reader, fields[3]); }
    if (status == SOURCE_JSON_OK) { status = delivery_response_definitions(reader, fields[4]); }
    return status;
}
SourceJsonStatus delivery_source_response_read(DeliverySourceResponse *response, const SourceText *source) {
    if (!response || !source || response->attempted) { return SOURCE_JSON_INVALID; }
    response->attempted = true;
    SourceArena scratch;
    source_arena_init(&scratch, response->arena.allocator);
    const SourceJsonValue *root = NULL;
    SourceJsonStatus status = source_json_parse(&scratch, source, &root, &response->issue);
    if (status == SOURCE_JSON_OK) {
        DeliveryResponseReader reader = {response, &scratch};
        status = decode(&reader, root);
    }
    source_arena_destroy(&scratch);
    response->status = status; response->valid = status == SOURCE_JSON_OK;
    return status;
}
