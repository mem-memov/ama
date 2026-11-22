/** @file
 * @brief Own decoded protocol names, physical byte paths and coherent captured source data.
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01)
 * shares data owners without compiler or project effects;
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves exact bytes;
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires coherent repeated physical captures;
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) separates retained
 * values from temporary syntax/byte-decoding storage.
 */
#include "internal.h"
void source_json_capture_init(SourceJsonCapture *capture, SourceAllocator allocator) {
    *capture = (SourceJsonCapture){0};
    source_arena_init(&capture->arena, allocator); source_manager_init(&capture->sources, allocator);
}
void source_json_capture_destroy(SourceJsonCapture *capture) {
    source_arena_destroy(&capture->arena); source_manager_destroy(&capture->sources);
    *capture = (SourceJsonCapture){0};
}
SourceJsonStatus source_json_capture_error(const SourceJsonValue *value, SourceJsonIssue *issue, const char *message) {
    if (issue) { *issue = (SourceJsonIssue){value ? value->origin : (SourceSpan){0}, message}; }
    return SOURCE_JSON_INVALID;
}
SourceJsonStatus source_json_capture_memory(const SourceJsonValue *value, SourceJsonIssue *issue) {
    if (issue) { *issue = (SourceJsonIssue){value ? value->origin : (SourceSpan){0}, "Cannot retain captured protocol data"}; }
    return SOURCE_JSON_OUT_OF_MEMORY;
}
