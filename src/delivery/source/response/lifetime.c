/** @file
 * @brief Decode source-domain replies into independent plain captured data and dependency records.
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01)
 * keeps consumers independent of compiler owners;
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves exact identity and original binding facts;
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) retains bytes and bounded source sites;
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns every partial result.
 */
#include "internal.h"
void delivery_source_response_init(DeliverySourceResponse *response, SourceAllocator allocator) {
    *response = (DeliverySourceResponse){.status = SOURCE_JSON_INVALID};
    source_arena_init(&response->arena, allocator); source_json_capture_init(&response->capture, allocator);
}
void delivery_source_response_destroy(DeliverySourceResponse *response) {
    source_arena_destroy(&response->arena); source_json_capture_destroy(&response->capture);
    *response = (DeliverySourceResponse){0};
}
SourceJsonStatus delivery_response_error(DeliveryResponseReader *reader, const SourceJsonValue *value, const char *message) {
    reader->owner->issue = (SourceJsonIssue){value ? value->origin : (SourceSpan){0}, message};
    return SOURCE_JSON_INVALID;
}
SourceJsonStatus delivery_response_memory(DeliveryResponseReader *reader, const SourceJsonValue *value) {
    reader->owner->issue = (SourceJsonIssue){value ? value->origin : (SourceSpan){0}, "Cannot retain the complete source-domain response"};
    return SOURCE_JSON_OUT_OF_MEMORY;
}
