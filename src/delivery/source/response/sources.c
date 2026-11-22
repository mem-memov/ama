/** @file
 * @brief Retain exact source/configuration/native observations without guessing activation.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves canonical source/version identity;
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) distinguishes
 * uncaptured data from observed absence;
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial observations.
 */
#include <string.h>
#include "internal.h"
#include "source/path/logical.h"
const DeliveryResponseSource *delivery_response_source(const DeliverySourceResponse *response, const char *identity) {
    for (const DeliveryResponseSource *item = response->sources; item; item = item->next) {
        if (!strcmp(item->identity, identity)) { return item; }
    }
    return NULL;
}
/** Use canonical component boundaries to find an ordinary version; standard
 * definitions have no package scope under
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02).
 */
static bool version_present(const DeliverySourceResponse *owner, const char *identity) {
    ModulePathShape shape;
    if (!module_path_describe((const unsigned char *)identity, strlen(identity), &shape)) { return false; }
    if (shape.root == MODULE_ROOT_STANDARD) { return true; }
    for (const DeliveryResponseVersion *version = owner->versions; version; version = version->next) {
        if (strlen(version->identity) == shape.version_length
            && !memcmp(version->identity, identity, shape.version_length)) { return true; }
    }
    return false;
}
/** Retain only coherent capture observations; false does not invent absence or
 * permit non-null hidden metadata under
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04).
 */
static SourceJsonStatus native(DeliveryResponseReader *reader, const SourceJsonValue *value, DeliveryResponseSource *source) {
    const char *const names[] = {"captured", "body", "header", "abi"};
    const SourceJsonValue *fields[4];
    SourceJsonStatus status = source_json_schema_fields(value, names, 4, fields, &reader->owner->issue);
    if (status != SOURCE_JSON_OK) { return status; }
    if (fields[0]->kind != SOURCE_JSON_BOOLEAN) { return delivery_response_error(reader, fields[0], "Native capture state is not boolean"); }
    source->native_captured = fields[0]->boolean;
    status = source_json_capture_path(&reader->owner->capture, reader->scratch, fields[1], &source->native_body, &reader->owner->issue);
    if (status != SOURCE_JSON_OK) { return status; }
    status = source_json_capture_path(&reader->owner->capture, reader->scratch, fields[2], &source->native_header, &reader->owner->issue);
    if (status != SOURCE_JSON_OK) { return status; }
    status = source_json_capture_text(&reader->owner->capture, reader->scratch, fields[3], &source->native_abi, &reader->owner->issue);
    if (status != SOURCE_JSON_OK) { return status; }
    if (!source->native_captured && (source->native_body || source->native_header || source->native_abi)) {
        return delivery_response_error(reader, value, "Uncaptured native observation contains data");
    }
    return SOURCE_JSON_OK;
}
SourceJsonStatus delivery_response_sources(DeliveryResponseReader *reader, const SourceJsonValue *value) {
    if (value->kind != SOURCE_JSON_ARRAY) { return delivery_response_error(reader, value, "Expected response source array"); }
    DeliveryResponseSource *first = NULL, **tail = &first;
    const char *const names[] = {"identity", "source", "configurationCaptured", "configuration", "native"};
    for (const SourceJsonValue *item = value->first; item; item = item->next) {
        const SourceJsonValue *fields[5];
        SourceJsonStatus status = source_json_schema_fields(item, names, 5, fields, &reader->owner->issue);
        if (status != SOURCE_JSON_OK) { return status; }
        DeliveryResponseSource prepared = {0};
        status = source_json_capture_string(&reader->owner->capture, fields[0], &prepared.identity, &reader->owner->issue);
        if (status != SOURCE_JSON_OK) { return status; }
        if (!module_path_canonical((const unsigned char *)prepared.identity, strlen(prepared.identity))
            || !version_present(reader->owner, prepared.identity)) {
            return delivery_response_error(reader, fields[0], "Response module identity or selected version is invalid");
        }
        status = source_json_capture_text(&reader->owner->capture, reader->scratch, fields[1], &prepared.source, &reader->owner->issue);
        if (status != SOURCE_JSON_OK) { return status; }
        if (!prepared.source) { return delivery_response_error(reader, fields[1], "Response source bytes are missing"); }
        for (const DeliveryResponseSource *prior = first; prior; prior = prior->next) {
            if (!strcmp(prior->identity, prepared.identity) || prior->source == prepared.source) {
                return delivery_response_error(reader, item, "Response source identity or physical path is repeated");
            }
        }
        if (fields[2]->kind != SOURCE_JSON_BOOLEAN) { return delivery_response_error(reader, fields[2], "Configuration capture state is not boolean"); }
        prepared.configuration_captured = fields[2]->boolean;
        status = source_json_capture_text(&reader->owner->capture, reader->scratch, fields[3], &prepared.configuration, &reader->owner->issue);
        if (status != SOURCE_JSON_OK) { return status; }
        if (!prepared.configuration_captured && prepared.configuration) {
            return delivery_response_error(reader, fields[3], "Uncaptured configuration observation contains data");
        }
        status = native(reader, fields[4], &prepared);
        if (status != SOURCE_JSON_OK) { return status; }
        DeliveryResponseSource *source = source_arena_allocate(&reader->owner->arena, sizeof(*source));
        if (!source) { return delivery_response_memory(reader, item); }
        *source = prepared; *tail = source; tail = &source->next;
    }
    reader->owner->sources = first;
    return SOURCE_JSON_OK;
}
