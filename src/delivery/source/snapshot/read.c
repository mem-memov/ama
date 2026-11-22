/** @file
 * @brief Publish one fully decoded explicit captured-source request and retire syntax scratch.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires complete staged-input authority before normal frontend checking;
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) rejects unknown
 * request versions/fields; [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * separates temporary JSON state from retained result ownership.
 */
#include <string.h>
#include "internal.h"
/** Decode the closed request schema before any model becomes valid.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * permits no missing-field, scope or filesystem fallback.
 */
static SourceJsonStatus decode(DeliverySnapshotReader *reader, const SourceJsonValue *root) {
    const char *const names[] = {"format", "versions", "sources"};
    const SourceJsonValue *fields[3];
    SourceJsonStatus status = source_json_schema_fields(root, names, 3, fields, &reader->owner->issue);
    if (status != SOURCE_JSON_OK) { return status; }
    const char *format = NULL;
    status = source_json_schema_cstring(fields[0], &format, &reader->owner->issue);
    if (status != SOURCE_JSON_OK) { return status; }
    if (strcmp(format, "ama.source-snapshot.v1")) {
        return delivery_snapshot_error(reader, fields[0], "Unknown captured source input format");
    }
    status = delivery_snapshot_versions(reader, fields[1]);
    if (status != SOURCE_JSON_OK) { return status; }
    return delivery_snapshot_entries(reader, fields[2]);
}
SourceJsonStatus delivery_source_snapshot_read(DeliverySourceSnapshot *snapshot, const SourceText *source) {
    if (!snapshot || !source || snapshot->attempted) { return SOURCE_JSON_INVALID; }
    snapshot->attempted = true;
    SourceArena scratch;
    source_arena_init(&scratch, snapshot->arena.allocator);
    const SourceJsonValue *root = NULL;
    SourceJsonStatus status = source_json_parse(&scratch, source, &root, &snapshot->issue);
    if (status == SOURCE_JSON_OK) {
        DeliverySnapshotReader reader = {snapshot, &scratch};
        status = decode(&reader, root);
    }
    source_arena_destroy(&scratch);
    snapshot->status = status; snapshot->valid = status == SOURCE_JSON_OK;
    return status;
}
