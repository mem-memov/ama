/** @file
 * @brief Decode explicit unique selected-version contexts and required package metadata.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) requires package.ini;
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves exact analysis identities; [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * retains partially prepared version records without publishing a valid input.
 */
#include <string.h>
#include "internal.h"
#include "source/path/logical.h"
SourceJsonStatus delivery_snapshot_versions(DeliverySnapshotReader *reader, const SourceJsonValue *value) {
    if (value->kind != SOURCE_JSON_ARRAY) { return delivery_snapshot_error(reader, value, "Expected captured version array"); }
    DeliverySnapshotVersion *first = NULL, **tail = &first;
    const char *const names[] = {"identity", "directory", "configuration"};
    for (const SourceJsonValue *item = value->first; item; item = item->next) {
        const SourceJsonValue *fields[3];
        SourceJsonStatus status = source_json_schema_fields(item, names, 3, fields, &reader->owner->issue);
        if (status != SOURCE_JSON_OK) { return status; }
        const char *identity = NULL, *directory = NULL;
        const SourceText *configuration = NULL;
        status = delivery_snapshot_string(reader, fields[0], &identity);
        if (status != SOURCE_JSON_OK) { return status; }
        ModulePathShape shape;
        if (!module_path_describe((const unsigned char *)identity, strlen(identity), &shape)
            || shape.root != MODULE_ROOT_PROJECT || shape.extent != MODULE_PATH_VERSION) {
            return delivery_snapshot_error(reader, fields[0], "Captured version identity is not a canonical ordinary version");
        }
        for (const DeliverySnapshotVersion *prior = first; prior; prior = prior->next) {
            if (!strcmp(prior->identity, identity)) {
                return delivery_snapshot_error(reader, fields[0], "Captured version identity is repeated");
            }
        }
        status = delivery_snapshot_path(reader, fields[1], &directory);
        if (status != SOURCE_JSON_OK) { return status; }
        if (!directory) { return delivery_snapshot_error(reader, fields[1], "Captured version directory is missing"); }
        status = delivery_snapshot_text(reader, fields[2], &configuration);
        if (status != SOURCE_JSON_OK) { return status; }
        if (!configuration) { return delivery_snapshot_error(reader, fields[2], "Captured version requires package.ini bytes"); }
        DeliverySnapshotVersion *version = source_arena_allocate(&reader->owner->arena, sizeof(*version));
        if (!version) { return delivery_snapshot_memory(reader, item); }
        *version = (DeliverySnapshotVersion){.identity = identity, .directory = directory, .configuration = configuration};
        *tail = version; tail = &version->next;
    }
    reader->owner->versions = first;
    return SOURCE_JSON_OK;
}
