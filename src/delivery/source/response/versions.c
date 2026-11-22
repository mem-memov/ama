/** @file
 * @brief Decode reported unique version observations and package metadata.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) requires package.ini;
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves exact analysis identities; [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * retains partially prepared version records without publishing a valid input.
 */
#include <string.h>
#include "internal.h"
#include "source/path/logical.h"
SourceJsonStatus delivery_response_versions(DeliveryResponseReader *reader, const SourceJsonValue *value) {
    if (value->kind != SOURCE_JSON_ARRAY) { return delivery_response_error(reader, value, "Expected reported version array"); }
    DeliveryResponseVersion *first = NULL, **tail = &first;
    const char *const names[] = {"identity", "directory", "configuration"};
    for (const SourceJsonValue *item = value->first; item; item = item->next) {
        const SourceJsonValue *fields[3];
        SourceJsonStatus status = source_json_schema_fields(item, names, 3, fields, &reader->owner->issue);
        if (status != SOURCE_JSON_OK) { return status; }
        const char *identity = NULL, *directory = NULL;
        const SourceText *configuration = NULL;
        status = source_json_capture_string(&reader->owner->capture, fields[0], &identity, &reader->owner->issue);
        if (status != SOURCE_JSON_OK) { return status; }
        ModulePathShape shape;
        if (!module_path_describe((const unsigned char *)identity, strlen(identity), &shape)
            || shape.root != MODULE_ROOT_PROJECT || shape.extent != MODULE_PATH_VERSION) {
            return delivery_response_error(reader, fields[0], "Reported version identity is not a canonical ordinary version");
        }
        for (const DeliveryResponseVersion *prior = first; prior; prior = prior->next) {
            if (!strcmp(prior->identity, identity)) {
                return delivery_response_error(reader, fields[0], "Reported version identity is repeated");
            }
        }
        status = source_json_capture_path(&reader->owner->capture, reader->scratch, fields[1], &directory, &reader->owner->issue);
        if (status != SOURCE_JSON_OK) { return status; }
        if (!directory) { return delivery_response_error(reader, fields[1], "Reported version directory is missing"); }
        status = source_json_capture_text(&reader->owner->capture, reader->scratch, fields[2], &configuration, &reader->owner->issue);
        if (status != SOURCE_JSON_OK) { return status; }
        if (!configuration) { return delivery_response_error(reader, fields[2], "Reported version requires package.ini bytes"); }
        DeliveryResponseVersion *version = source_arena_allocate(&reader->owner->arena, sizeof(*version));
        if (!version) { return delivery_response_memory(reader, item); }
        *version = (DeliveryResponseVersion){.identity = identity, .directory = directory, .configuration = configuration};
        *tail = version; tail = &version->next;
    }
    reader->owner->versions = first;
    return SOURCE_JSON_OK;
}
