/** @file
 * @brief Decode explicit captured source inputs without filesystem discovery or execution.
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01)
 * separates prepared data from project effects;
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves explicit original analysis identities;
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * checks captured staged bytes without probing future paths;
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) retains exact
 * byte values and failure coordinates;
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns all failed scratch.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) requires package metadata;
 * [SRC-02](../../../../spec/language/source-and-names.md#src-02) keeps exact registry identities.
 */
#include "internal.h"
void delivery_source_snapshot_init(DeliverySourceSnapshot *snapshot, SourceAllocator allocator) {
    *snapshot = (DeliverySourceSnapshot){.status = SOURCE_JSON_INVALID};
    source_arena_init(&snapshot->arena, allocator);
    source_manager_init(&snapshot->sources, allocator);
}
void delivery_source_snapshot_destroy(DeliverySourceSnapshot *snapshot) {
    source_arena_destroy(&snapshot->arena);
    source_manager_destroy(&snapshot->sources);
    *snapshot = (DeliverySourceSnapshot){0};
}
SourceJsonStatus delivery_snapshot_error(DeliverySnapshotReader *reader, const SourceJsonValue *value, const char *message) {
    reader->owner->issue = (SourceJsonIssue){value ? value->origin : (SourceSpan){0}, message};
    return SOURCE_JSON_INVALID;
}
SourceJsonStatus delivery_snapshot_memory(DeliverySnapshotReader *reader, const SourceJsonValue *value) {
    reader->owner->issue = (SourceJsonIssue){value ? value->origin : (SourceSpan){0}, "Cannot retain the complete captured source input"};
    return SOURCE_JSON_OUT_OF_MEMORY;
}
