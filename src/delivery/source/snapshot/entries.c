/** @file
 * @brief Decode canonical captured source rows, metadata and unconditional companions.
 * [SRC-02](../../../../spec/language/source-and-names.md#src-02) preserves
 * exact module identities; [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * supplies selected version contexts; [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * keeps captured absence explicit; [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * owns failed row preparation; [COMP-04](../../../../spec/implementation/compiler.md#comp-04)
 * refuses lossy filesystem identity conversions.
 */
#include <stdint.h>
#include <string.h>
#include "internal.h"
#include "source/path/logical.h"
/** Canonical component boundaries, not textual prefix guesses, select a version.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * retains each ordinary definition's exact selected-version owner.
 */
static bool version_present(const DeliverySourceSnapshot *owner, const char *identity) {
    ModulePathShape shape;
    if (!module_path_describe((const unsigned char *)identity, strlen(identity), &shape)) { return false; }
    if (shape.root == MODULE_ROOT_STANDARD) { return true; }
    for (const DeliverySnapshotVersion *version = owner->versions; version; version = version->next) {
        if (strlen(version->identity) == shape.version_length
            && !memcmp(version->identity, identity, shape.version_length)) { return true; }
    }
    return false;
}
/** Verify exact target-type round trips and nonnegative identity values.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) forbids
 * truncating a device/inode before catalog alias validation.
 */
static SourceJsonStatus metadata(DeliverySnapshotReader *reader, const SourceJsonValue *device,
    const SourceJsonValue *inode, ModuleSnapshotEntry *entry) {
    uintmax_t value;
    SourceJsonStatus status = source_json_schema_unsigned(device, &value, &reader->owner->issue);
    if (status != SOURCE_JSON_OK) { return status; }
    entry->device = (dev_t)value;
    if ((uintmax_t)entry->device != value || (value && !(entry->device > (dev_t)0))) {
        return delivery_snapshot_error(reader, device, "Captured device identity exceeds its target type");
    }
    status = source_json_schema_unsigned(inode, &value, &reader->owner->issue);
    if (status != SOURCE_JSON_OK) { return status; }
    entry->inode = (ino_t)value;
    if ((uintmax_t)entry->inode != value || (value && !(entry->inode > (ino_t)0))) {
        return delivery_snapshot_error(reader, inode, "Captured inode identity exceeds its target type");
    }
    return SOURCE_JSON_OK;
}
/** All same-basename data is captured unconditionally; native classification is
 * deferred to the production parser under
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
static SourceJsonStatus companions(DeliverySnapshotReader *reader, const SourceJsonValue *value,
    const char *identity, DeliverySourceCompanions **output) {
    const char *const names[] = {"body", "header", "abi"};
    const SourceJsonValue *fields[3];
    SourceJsonStatus status = source_json_schema_fields(value, names, 3, fields, &reader->owner->issue);
    if (status != SOURCE_JSON_OK) { return status; }
    const char *body = NULL, *header = NULL;
    const SourceText *abi = NULL;
    status = delivery_snapshot_path(reader, fields[0], &body);
    if (status != SOURCE_JSON_OK) { return status; }
    status = delivery_snapshot_path(reader, fields[1], &header);
    if (status != SOURCE_JSON_OK) { return status; }
    status = delivery_snapshot_text(reader, fields[2], &abi);
    if (status != SOURCE_JSON_OK) { return status; }
    DeliverySourceCompanions *row = source_arena_allocate(&reader->owner->arena, sizeof(*row));
    if (!row) { return delivery_snapshot_memory(reader, value); }
    *row = (DeliverySourceCompanions){.identity = identity, .body = body, .header = header, .abi = abi};
    *output = row;
    return SOURCE_JSON_OK;
}
SourceJsonStatus delivery_snapshot_entries(DeliverySnapshotReader *reader, const SourceJsonValue *value) {
    if (value->kind != SOURCE_JSON_ARRAY) { return delivery_snapshot_error(reader, value, "Expected captured source array"); }
    size_t count = 0;
    for (const SourceJsonValue *item = value->first; item; item = item->next) {
        if (count == SIZE_MAX / sizeof(ModuleSnapshotEntry)) { return delivery_snapshot_memory(reader, value); }
        count++;
    }
    ModuleSnapshotEntry *entries = count ? source_arena_allocate(&reader->owner->arena, count * sizeof(*entries)) : NULL;
    if (count && !entries) { return delivery_snapshot_memory(reader, value); }
    DeliverySourceCompanions *first = NULL, **tail = &first;
    const char *const names[] = {"identity", "source", "device", "inode", "configuration", "companions"};
    size_t index = 0;
    for (const SourceJsonValue *item = value->first; item; item = item->next, index++) {
        const SourceJsonValue *fields[6];
        SourceJsonStatus status = source_json_schema_fields(item, names, 6, fields, &reader->owner->issue);
        if (status != SOURCE_JSON_OK) { return status; }
        ModuleSnapshotEntry entry = {0};
        status = delivery_snapshot_string(reader, fields[0], &entry.logical_path);
        if (status != SOURCE_JSON_OK) { return status; }
        if (!module_path_canonical((const unsigned char *)entry.logical_path, strlen(entry.logical_path))
            || !version_present(reader->owner, entry.logical_path)) {
            return delivery_snapshot_error(reader, fields[0], "Captured module identity or selected version is invalid");
        }
        status = delivery_snapshot_text(reader, fields[1], &entry.text);
        if (status != SOURCE_JSON_OK) { return status; }
        if (!entry.text) { return delivery_snapshot_error(reader, fields[1], "Captured module source bytes are missing"); }
        for (size_t prior = 0; prior < index; prior++) {
            if (!strcmp(entries[prior].logical_path, entry.logical_path) || entries[prior].text == entry.text) {
                return delivery_snapshot_error(reader, item, "Captured module identity or physical source path is repeated");
            }
        }
        status = metadata(reader, fields[2], fields[3], &entry);
        if (status != SOURCE_JSON_OK) { return status; }
        status = delivery_snapshot_text(reader, fields[4], &entry.configuration);
        if (status != SOURCE_JSON_OK) { return status; }
        DeliverySourceCompanions *row = NULL;
        status = companions(reader, fields[5], entry.logical_path, &row);
        if (status != SOURCE_JSON_OK) { return status; }
        entries[index] = entry; *tail = row; tail = &row->next;
    }
    reader->owner->entries = entries; reader->owner->count = count;
    reader->owner->companions = first;
    return SOURCE_JSON_OK;
}
