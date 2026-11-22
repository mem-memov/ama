/** @file
 * @brief Decode exact active definition categories and original owner-backed dependency edges.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves normalized targets/members without text parsing;
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) validates exact
 * bounded source sites; [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * owns every failed graph prefix.
 */
#include <string.h>
#include "internal.h"
#include "source/unicode/unicode.h"
/** Path bytes and integer offsets must identify this exact owner's source.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) forbids
 * accepting an unrelated source or truncating a span.
 */
static SourceJsonStatus origin(DeliveryResponseReader *reader, const SourceJsonValue *value,
    const DeliveryResponseSource *owner, SourceSpan *output) {
    const char *const names[] = {"path", "start", "end"};
    const SourceJsonValue *fields[3];
    SourceJsonStatus status = source_json_schema_fields(value, names, 3, fields, &reader->owner->issue);
    if (status != SOURCE_JSON_OK) { return status; }
    const char *path = NULL;
    status = source_json_capture_path(&reader->owner->capture, reader->scratch, fields[0], &path, &reader->owner->issue);
    if (status != SOURCE_JSON_OK) { return status; }
    if (!path || strcmp(path, source_path(owner->source))) {
        return delivery_response_error(reader, fields[0], "Dependency site does not name its original owner source");
    }
    SourceSpan span = {.source = owner->source};
    status = source_json_schema_size(fields[1], &span.start, &reader->owner->issue);
    if (status != SOURCE_JSON_OK) { return status; }
    status = source_json_schema_size(fields[2], &span.end, &reader->owner->issue);
    if (status != SOURCE_JSON_OK) { return status; }
    if (!source_span_valid(span)) { return delivery_response_error(reader, value, "Dependency site is outside its source bytes"); }
    *output = span;
    return SOURCE_JSON_OK;
}
/** Read original order and preserve the distinction between requiring constructs
 * and written operands under
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02).
 */
static SourceJsonStatus dependencies(DeliveryResponseReader *reader, const SourceJsonValue *value,
    const DeliveryResponseSource *owner, DeliverySourceDependency **output) {
    if (value->kind != SOURCE_JSON_ARRAY) { return delivery_response_error(reader, value, "Expected original dependency array"); }
    DeliverySourceDependency *first = NULL, **tail = &first;
    const char *const names[] = {"target", "implicit", "member", "origin"};
    for (const SourceJsonValue *item = value->first; item; item = item->next) {
        const SourceJsonValue *fields[4];
        SourceJsonStatus status = source_json_schema_fields(item, names, 4, fields, &reader->owner->issue);
        if (status != SOURCE_JSON_OK) { return status; }
        const char *target = NULL, *member = NULL;
        status = source_json_schema_cstring(fields[0], &target, &reader->owner->issue);
        if (status != SOURCE_JSON_OK) { return status; }
        const DeliveryResponseSource *target_source = delivery_response_source(reader->owner, target);
        if (!target_source) { return delivery_response_error(reader, fields[0], "Dependency target has no captured source"); }
        if (fields[1]->kind != SOURCE_JSON_BOOLEAN) { return delivery_response_error(reader, fields[1], "Dependency implicit state is not boolean"); }
        if (fields[2]->kind != SOURCE_JSON_NULL) {
            status = source_json_capture_string(&reader->owner->capture, fields[2], &member, &reader->owner->issue);
            if (status != SOURCE_JSON_OK) { return status; }
            size_t invalid;
            if (!unicode_name((const unsigned char *)member, strlen(member), &invalid)) {
                return delivery_response_error(reader, fields[2], "Normalized constant member is not an Ama name");
            }
        }
        SourceSpan site;
        status = origin(reader, fields[3], owner, &site);
        if (status != SOURCE_JSON_OK) { return status; }
        DeliverySourceDependency *edge = source_arena_allocate(&reader->owner->arena, sizeof(*edge));
        if (!edge) { return delivery_response_memory(reader, item); }
        *edge = (DeliverySourceDependency){.target = target_source->identity, .origin = site,
            .implicit = fields[1]->boolean, .member = member};
        *tail = edge; tail = &edge->next;
    }
    *output = first;
    return SOURCE_JSON_OK;
}
/** Resolve active graph membership after all definitions exist, preserving cycles
 * without recursive traversal under
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02).
 */
static bool active(const DeliverySourceDefinition *definitions, const char *identity) {
    for (const DeliverySourceDefinition *item = definitions; item; item = item->next) {
        if (!strcmp(item->identity, identity)) { return true; }
    }
    return false;
}
SourceJsonStatus delivery_response_definitions(DeliveryResponseReader *reader, const SourceJsonValue *value) {
    if (value->kind != SOURCE_JSON_ARRAY) { return delivery_response_error(reader, value, "Expected response definition array"); }
    static const char *const kinds[] = {"concrete", "parameterized", "static", "constants"};
    const char *const names[] = {"identity", "kind", "dependencies"};
    DeliverySourceDefinition *first = NULL, **tail = &first;
    const char *previous = NULL;
    for (const SourceJsonValue *item = value->first; item; item = item->next) {
        const SourceJsonValue *fields[3];
        SourceJsonStatus status = source_json_schema_fields(item, names, 3, fields, &reader->owner->issue);
        if (status != SOURCE_JSON_OK) { return status; }
        const char *identity = NULL, *kind = NULL;
        status = source_json_schema_cstring(fields[0], &identity, &reader->owner->issue);
        if (status != SOURCE_JSON_OK) { return status; }
        const DeliveryResponseSource *source = delivery_response_source(reader->owner, identity);
        if (!source || (previous && strcmp(previous, identity) >= 0) || !source->configuration_captured) {
            return delivery_response_error(reader, fields[0], "Definition identity, order or configuration authority is invalid");
        }
        status = source_json_schema_cstring(fields[1], &kind, &reader->owner->issue);
        if (status != SOURCE_JSON_OK) { return status; }
        size_t category = 0;
        while (category < sizeof(kinds) / sizeof(*kinds) && strcmp(kind, kinds[category])) { category++; }
        if (category == sizeof(kinds) / sizeof(*kinds) || (source->native_captured && category != DELIVERY_SOURCE_CONCRETE)) {
            return delivery_response_error(reader, fields[1], "Definition category is invalid for its source observation");
        }
        DeliverySourceDependency *edges = NULL;
        status = dependencies(reader, fields[2], source, &edges);
        if (status != SOURCE_JSON_OK) { return status; }
        DeliverySourceDefinition *definition = source_arena_allocate(&reader->owner->arena, sizeof(*definition));
        if (!definition) { return delivery_response_memory(reader, item); }
        *definition = (DeliverySourceDefinition){.identity = source->identity, .kind = (DeliverySourceKind)category, .dependencies = edges};
        *tail = definition; tail = &definition->next; previous = source->identity;
    }
    for (const DeliverySourceDefinition *item = first; item; item = item->next) {
        for (const DeliverySourceDependency *edge = item->dependencies; edge; edge = edge->next) {
            if (!active(first, edge->target)) { return delivery_response_error(reader, value, "Dependency target has no active definition"); }
        }
    }
    for (const DeliveryResponseSource *source = reader->owner->sources; source; source = source->next) {
        if (source->native_captured && !active(first, source->identity)) {
            return delivery_response_error(reader, value, "Native observation has no active definition");
        }
    }
    reader->owner->definitions = first;
    return SOURCE_JSON_OK;
}
