/** @file
 * @brief Own decoded logical strings, physical paths and coherent captured text bytes.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves
 * byte values and refuses NUL-truncated paths; [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires one coherent byte snapshot per captured physical key;
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) keeps temporary
 * JSON buffers separate from retained result source owners.
 */
#include <string.h>
#include "internal.h"
SourceJsonStatus delivery_snapshot_string(DeliverySnapshotReader *reader, const SourceJsonValue *value, const char **output) {
    const char *text = NULL;
    SourceJsonStatus status = source_json_schema_cstring(value, &text, &reader->owner->issue);
    if (status != SOURCE_JSON_OK) { return status; }
    const char *copy = source_arena_copy(&reader->owner->arena, text);
    if (!copy) { return delivery_snapshot_memory(reader, value); }
    *output = copy;
    return SOURCE_JSON_OK;
}
SourceJsonStatus delivery_snapshot_path(DeliverySnapshotReader *reader, const SourceJsonValue *value, const char **output) {
    if (value->kind == SOURCE_JSON_NULL) { *output = NULL; return SOURCE_JSON_OK; }
    SourceJsonText bytes;
    SourceJsonStatus status = source_json_schema_bytes(reader->scratch, value, &bytes, &reader->owner->issue);
    if (status != SOURCE_JSON_OK) { return status; }
    if (!bytes.length || memchr(bytes.bytes, 0, bytes.length)) {
        return delivery_snapshot_error(reader, value, "Captured physical path must be nonempty and NUL-free");
    }
    const char *copy = source_arena_copy(&reader->owner->arena, (const char *)bytes.bytes);
    if (!copy) { return delivery_snapshot_memory(reader, value); }
    *output = copy;
    return SOURCE_JSON_OK;
}
SourceJsonStatus delivery_snapshot_text(DeliverySnapshotReader *reader, const SourceJsonValue *value, const SourceText **output) {
    if (value->kind == SOURCE_JSON_NULL) { *output = NULL; return SOURCE_JSON_OK; }
    const char *const names[] = {"path", "content"};
    const SourceJsonValue *fields[2];
    SourceJsonStatus status = source_json_schema_fields(value, names, 2, fields, &reader->owner->issue);
    if (status != SOURCE_JSON_OK) { return status; }
    const char *path = NULL;
    status = delivery_snapshot_path(reader, fields[0], &path);
    if (status != SOURCE_JSON_OK) { return status; }
    if (!path) { return delivery_snapshot_error(reader, fields[0], "Captured source requires a physical path"); }
    SourceJsonText bytes;
    status = source_json_schema_bytes(reader->scratch, fields[1], &bytes, &reader->owner->issue);
    if (status != SOURCE_JSON_OK) { return status; }
    for (const DeliverySnapshotText *item = reader->owner->texts; item; item = item->next) {
        if (strcmp(source_path(item->source), path)) { continue; }
        if (source_length(item->source) != bytes.length || memcmp(source_bytes(item->source), bytes.bytes, bytes.length)) {
            return delivery_snapshot_error(reader, value, "One captured physical path has conflicting byte snapshots");
        }
        *output = item->source;
        return SOURCE_JSON_OK;
    }
    DeliverySnapshotText *item = source_arena_allocate(&reader->owner->arena, sizeof(*item));
    if (!item) { return delivery_snapshot_memory(reader, value); }
    const SourceText *source = NULL;
    if (source_manager_add(&reader->owner->sources, path, bytes.bytes, bytes.length, &source) != SOURCE_OK) {
        return delivery_snapshot_memory(reader, value);
    }
    *item = (DeliverySnapshotText){reader->owner->texts, source}; reader->owner->texts = item;
    *output = source;
    return SOURCE_JSON_OK;
}
