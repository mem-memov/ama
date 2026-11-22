/** @file
 * @brief Stream exact prepared source/version/companion tables into the existing captured-input schema.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves
 * bytes and unsigned identity values; [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * refuses missing/ambiguous capture authority without fallback.
 */
#include <string.h>
#include "write.h"
#include "../wire.h"
/** Exact singleton lookup never chooses a first conflicting row under
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
static const DeliverySourceCompanions *companion(const DeliverySourceCompanions *rows, const char *identity) {
    const DeliverySourceCompanions *found = NULL;
    for (const DeliverySourceCompanions *row = rows; row; row = row->next) {
        if (!row->identity) { return NULL; }
        if (strcmp(row->identity, identity)) { continue; }
        if (found) { return NULL; }
        found = row;
    }
    return found;
}
/** Source objects always retain a nonempty physical observation key under
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04).
 */
static bool text(const SourceText *source) { return !source || *source_path(source); }
/** Validate basic borrowed view authority before emitting any request bytes.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * keeps source/configuration and companion meanings explicit.
 */
static bool prepared(const ModuleSnapshotEntry *entries, size_t count, const DeliverySnapshotVersion *versions,
    const DeliverySourceCompanions *rows) {
    if (count && !entries) { return false; }
    for (const DeliverySnapshotVersion *version = versions; version; version = version->next) {
        if (!version->identity || !*version->identity || !version->directory || !*version->directory
            || !version->configuration || !text(version->configuration)) { return false; }
        for (const DeliverySnapshotVersion *prior = versions; prior != version; prior = prior->next) {
            if (!strcmp(prior->identity, version->identity)) { return false; }
        }
    }
    for (size_t index = 0; index < count; index++) {
        const ModuleSnapshotEntry *entry = &entries[index];
        if (!entry->logical_path || !*entry->logical_path || !entry->text || !text(entry->text)
            || !text(entry->configuration) || (entry->device && !(entry->device > (dev_t)0))
            || (entry->inode && !(entry->inode > (ino_t)0))
            || (dev_t)(uintmax_t)entry->device != entry->device || (ino_t)(uintmax_t)entry->inode != entry->inode) { return false; }
        const DeliverySourceCompanions *row = companion(rows, entry->logical_path);
        if (!row || (row->body && !*row->body) || (row->header && !*row->header) || !text(row->abi)) { return false; }
        for (size_t prior = 0; prior < index; prior++) {
            if (!strcmp(entries[prior].logical_path, entry->logical_path)
                || !strcmp(source_path(entries[prior].text), source_path(entry->text))) { return false; }
        }
    }
    for (const DeliverySourceCompanions *row = rows; row; row = row->next) {
        bool found = false;
        for (size_t index = 0; index < count; index++) {
            if (row->identity && !strcmp(row->identity, entries[index].logical_path)) { found = true; break; }
        }
        if (!found) { return false; }
    }
    return true;
}
/** Borrowed rows remain in explicit supplied order; that order does not select
 * authority under [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
static bool version_array(ToolJsonWriter *writer, const DeliverySnapshotVersion *versions) {
    if (!tool_json_raw(writer, "\"versions\":[")) { return false; }
    bool comma = false;
    for (const DeliverySnapshotVersion *version = versions; version; version = version->next) {
        if ((comma && !tool_json_raw(writer, ",")) || !tool_json_raw(writer, "{\"identity\":")
            || !tool_json_string(writer, version->identity) || !tool_json_raw(writer, ",\"directory\":")
            || !delivery_source_wire_path(writer, version->directory) || !tool_json_raw(writer, ",\"configuration\":")
            || !delivery_source_wire_text(writer, version->configuration) || !tool_json_raw(writer, "}")) { return false; }
        comma = true;
    }
    return tool_json_raw(writer, "]");
}
/** Preserve captured absence and opaque ABI bytes without interpreting source
 * roles under [COMP-04](../../../../spec/implementation/compiler.md#comp-04).
 */
static bool source_row(ToolJsonWriter *writer, const ModuleSnapshotEntry *entry, const DeliverySourceCompanions *row) {
    return tool_json_raw(writer, "{\"identity\":") && tool_json_string(writer, entry->logical_path)
        && tool_json_raw(writer, ",\"source\":") && delivery_source_wire_text(writer, entry->text)
        && tool_json_raw(writer, ",\"device\":") && tool_json_unsigned(writer, (uintmax_t)entry->device)
        && tool_json_raw(writer, ",\"inode\":") && tool_json_unsigned(writer, (uintmax_t)entry->inode)
        && tool_json_raw(writer, ",\"configuration\":") && delivery_source_wire_text(writer, entry->configuration)
        && tool_json_raw(writer, ",\"companions\":{\"body\":") && delivery_source_wire_path(writer, row->body)
        && tool_json_raw(writer, ",\"header\":") && delivery_source_wire_path(writer, row->header)
        && tool_json_raw(writer, ",\"abi\":") && delivery_source_wire_text(writer, row->abi)
        && tool_json_raw(writer, "}}");
}
bool delivery_source_snapshot_write(FILE *output, const DeliverySourceSnapshotView *view) {
    if (!output || !view || !prepared(view->entries, view->count, view->versions, view->companions)) { return false; }
    const ModuleSnapshotEntry *entries = view->entries;
    size_t count = view->count;
    const DeliverySnapshotVersion *versions = view->versions;
    const DeliverySourceCompanions *companions = view->companions;
    ToolJsonWriter writer; tool_json_init(&writer, output);
    if (!tool_json_raw(&writer, "{\"format\":\"ama.source-snapshot.v1\",") || !version_array(&writer, versions)
        || !tool_json_raw(&writer, ",\"sources\":[")) { return false; }
    for (size_t index = 0; index < count; index++) {
        if ((index && !tool_json_raw(&writer, ","))
            || !source_row(&writer, &entries[index], companion(companions, entries[index].logical_path))) { return false; }
    }
    return tool_json_raw(&writer, "]}\n") && tool_json_finish(&writer);
}
