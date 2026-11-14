/** @file
 * @brief Serialize deterministic component/export/schema metadata without private layouts.
 * [EMBED-01](../../../spec/implementation/c-library.md#embed-01) publishes
 * explicit target, ABI, codec identities and link settings beside generated artifacts.
 */
#include <assert.h>
#include "../internal.h"
/** Emit one JSON string from validated UTF-8, preserving controls as exact escapes. */
static bool string(FILE *output, const char *text) {
    static const char digits[] = "0123456789abcdef";
    if (fputc('"', output) == EOF) { return false; }
    for (const unsigned char *p = (const unsigned char *)text; *p != 0; p++) {
        if (*p == '"' || *p == '\\') {
            if (fputc('\\', output) == EOF || fputc(*p, output) == EOF) { return false; }
        } else if (*p < 32) {
            if (fprintf(output, "\\u00%c%c", digits[*p >> 4], digits[*p & 15]) < 0) { return false; }
        } else if (fputc(*p, output) == EOF) { return false; }
    }
    return fputc('"', output) != EOF;
}
/** Write a fixed key and one escaped UTF-8 value, propagating every stream error. */
static bool property(FILE *output, const char *key, const char *value) {
    return string(output, key) && fputc(':', output) != EOF && string(output, value);
}
bool library_manifest_write(const LibraryBuild *build, FILE *output) {
    assert(build->valid);
    if (fputs("{\"format\":\"ama-c-build-1\",\"stage\":\"resolved\",", output) == EOF
        || !property(output, "component", build->component) || fputc(',', output) == EOF
        || !property(output, "identity", build->identity->internal_name) || fputc(',', output) == EOF
        || !property(output, "abi", build->abi) || fputc(',', output) == EOF
        || !property(output, "target", build->target) || fputc(',', output) == EOF
        || !property(output, "calling_convention", build->calling_convention)
        || fputs(",\"exports\":[", output) == EOF) { return false; }
    for (const LibraryExport *item = build->exports; item != NULL; item = item->next) {
        if ((item != build->exports && fputc(',', output) == EOF) || fputc('{', output) == EOF
            || !property(output, "name", item->name) || fputc(',', output) == EOF
            || !property(output, "module", item->module) || fputc(',', output) == EOF
            || !property(output, "identity", item->identity->internal_name)
            || fprintf(output, ",\"operations\":%u}", item->operations) < 0) { return false; }
    }
    if (fputs("],\"schemas\":[", output) == EOF) { return false; }
    static const char *const profiles[] = {"integer-v1", "utf8-v1", "condition-v1", "failure-v1", "list-v1"};
    static const char *const representations[] = {
        "sign-and-owned-big-endian-magnitude", "owned-utf8-octets", "boolean",
        "failure-condition-and-owned-utf8-description", "owned-array-of-declared-element"};
    for (const LibrarySchema *item = build->schemas; item != NULL; item = item->next) {
        const char *name = item->identity->internal_name;
        if ((item != build->schemas && fputc(',', output) == EOF) || fputc('{', output) == EOF
            || !property(output, "name", item->name) || fputc(',', output) == EOF
            || !property(output, "identity", name) || fputc(',', output) == EOF
            || !property(output, "module", item->type->module->source->logical_path) || fputc(',', output) == EOF
            || !property(output, "profile", profiles[item->kind]) || fputc(',', output) == EOF
            || !property(output, "representation", representations[item->kind])
            || fprintf(output, ",\"encoder\":\"%s_encode\",\"decoder\":\"%s_decode\",\"release\":\"%s_release\",\"clone\":\"%s_clone\"", name, name, name, name) < 0) { return false; }
        if (item->element != NULL && (fputc(',', output) == EOF || !property(output, "element", item->element->name))) { return false; }
        if (fputc('}', output) == EOF) { return false; }
    }
    if (fputs("],\"bindings\":[", output) == EOF) { return false; }
    for (const LibraryBinding *item = build->bindings; item != NULL; item = item->next) {
        if ((item != build->bindings && fputc(',', output) == EOF) || fputc('{', output) == EOF
            || !property(output, "export", item->export_name) || fputc(',', output) == EOF
            || !property(output, "field", item->field_name) || fputc(',', output) == EOF
            || !property(output, "schema", item->schema_name)
            || fprintf(output, ",\"input\":%s,\"ordinal\":%zu}", item->input ? "true" : "false", item->field->role_ordinal) < 0) { return false; }
    }
    if (fputs("],\"settings\":[", output) == EOF) { return false; }
    for (const LibrarySetting *item = build->settings; item != NULL; item = item->next) {
        if ((item != build->settings && fputc(',', output) == EOF) || fputc('{', output) == EOF
            || !property(output, "kind", item->kind) || fputc(',', output) == EOF
            || !property(output, "value", item->value) || fputc('}', output) == EOF) { return false; }
    }
    return fputs("]}\n", output) != EOF && fflush(output) == 0;
}
