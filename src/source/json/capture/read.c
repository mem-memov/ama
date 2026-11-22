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
#include <string.h>
#include "internal.h"
SourceJsonStatus source_json_capture_string(SourceJsonCapture *capture, const SourceJsonValue *value, const char **output, SourceJsonIssue *issue) {
    if (!capture || !value || !output || !issue) { return source_json_capture_error(value, issue, "Invalid captured string owner"); }
    const char *text = NULL;
    SourceJsonStatus status = source_json_schema_cstring(value, &text, issue);
    if (status != SOURCE_JSON_OK) { return status; }
    const char *copy = source_arena_copy(&capture->arena, text);
    if (!copy) { return source_json_capture_memory(value, issue); }
    *output = copy;
    return SOURCE_JSON_OK;
}
SourceJsonStatus source_json_capture_path(SourceJsonCapture *capture, SourceArena *scratch, const SourceJsonValue *value, const char **output, SourceJsonIssue *issue) {
    if (!capture || !scratch || !value || !output || !issue || scratch == &capture->arena) {
        return source_json_capture_error(value, issue, "Invalid or aliased capture scratch/result owner");
    }
    if (value->kind == SOURCE_JSON_NULL) { *output = NULL; return SOURCE_JSON_OK; }
    SourceJsonText bytes;
    SourceJsonStatus status = source_json_schema_bytes(scratch, value, &bytes, issue);
    if (status != SOURCE_JSON_OK) { return status; }
    if (!bytes.length || memchr(bytes.bytes, 0, bytes.length)) {
        return source_json_capture_error(value, issue, "Captured physical path must be nonempty and NUL-free");
    }
    const char *copy = source_arena_copy(&capture->arena, (const char *)bytes.bytes);
    if (!copy) { return source_json_capture_memory(value, issue); }
    *output = copy;
    return SOURCE_JSON_OK;
}
SourceJsonStatus source_json_capture_text(SourceJsonCapture *capture, SourceArena *scratch, const SourceJsonValue *value, const SourceText **output, SourceJsonIssue *issue) {
    if (!capture || !scratch || !value || !output || !issue || scratch == &capture->arena) {
        return source_json_capture_error(value, issue, "Invalid or aliased capture scratch/result owner");
    }
    if (value->kind == SOURCE_JSON_NULL) { *output = NULL; return SOURCE_JSON_OK; }
    const char *const names[] = {"path", "content"};
    const SourceJsonValue *fields[2];
    SourceJsonStatus status = source_json_schema_fields(value, names, 2, fields, issue);
    if (status != SOURCE_JSON_OK) { return status; }
    const char *path = NULL;
    status = source_json_capture_path(capture, scratch, fields[0], &path, issue);
    if (status != SOURCE_JSON_OK) { return status; }
    if (!path) { return source_json_capture_error(fields[0], issue, "Captured source requires a physical path"); }
    SourceJsonText bytes;
    status = source_json_schema_bytes(scratch, fields[1], &bytes, issue);
    if (status != SOURCE_JSON_OK) { return status; }
    for (const SourceJsonCapturedText *item = capture->texts; item; item = item->next) {
        if (strcmp(source_path(item->source), path)) { continue; }
        if (source_length(item->source) != bytes.length || memcmp(source_bytes(item->source), bytes.bytes, bytes.length)) {
            return source_json_capture_error(value, issue, "One captured physical path has conflicting byte snapshots");
        }
        *output = item->source;
        return SOURCE_JSON_OK;
    }
    SourceJsonCapturedText *item = source_arena_allocate(&capture->arena, sizeof(*item));
    if (!item) { return source_json_capture_memory(value, issue); }
    const SourceText *source = NULL;
    if (source_manager_add(&capture->sources, path, bytes.bytes, bytes.length, &source) != SOURCE_OK) {
        return source_json_capture_memory(value, issue);
    }
    *item = (SourceJsonCapturedText){capture->texts, source}; capture->texts = item;
    *output = source;
    return SOURCE_JSON_OK;
}
