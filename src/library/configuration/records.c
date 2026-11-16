/** @file
 * @brief Decode exact library records into explicit owned declarations.
 * [EMBED-01](../../../spec/implementation/c-library.md#embed-01) requires
 * duplicate and unsupported declarations to fail, rather than selecting by order.
 */
#include <stdint.h>
#include <string.h>
#include "../internal.h"
/** Split only required separators and retain exact field spans for
 * [TOOL-01](../../../spec/implementation/tools-and-conformance.md#tool-01)
 * settings rewrites; final-field bytes are never searched for embedded paths. */
static bool fields(LibraryBuild *build, SourceSpan site, char **values, size_t count, SourceSpan *sites) {
    const unsigned char *bytes = source_bytes(site.source);
    size_t start = site.start;
    for (size_t index = 0; index < count; index++) {
        size_t end = start;
        if (index + 1 == count) { end = site.end; }
        else {
            while (site.end - end >= 3 && memcmp(bytes + end, " | ", 3) != 0) { end++; }
            if (site.end - end < 3) { return library_fail(build, site, "Library record has too few fields separated by ' | '"); }
        }
        if (end == start || bytes[start] == ' ' || bytes[end - 1] == ' ') {
            return library_fail(build, site, "Library record fields must be nonempty with no surrounding spaces");
        }
        if (sites != NULL) { sites[index] = (SourceSpan){site.source,start,end}; }
        size_t length = end - start;
        if (length == SIZE_MAX) { return library_fail(build, site, "Library record size exceeds the supported range"); }
        values[index] = library_allocate(build, length + 1);
        if (values[index] == NULL) { return false; }
        memcpy(values[index], bytes + start, length);
        start = end + (index + 1 == count ? 0 : 3);
    }
    return true;
}
/** Reject repeated scalar metadata even when repeated bytes happen to agree. */
static bool scalar(LibraryBuild *build, SourceSpan site, const char **slot, const char *value) {
    if (*slot != NULL) { return library_fail(build, site, "Duplicate library metadata record"); }
    *slot = value;
    return true;
}
/** Parse a nonempty set of exact, unrepeated public operations. */
static unsigned operations(const char *text) {
    unsigned result = 0;
    for (const char *start = text; *start != '\0';) {
        const char *end = strchr(start, ',');
        size_t length = end == NULL ? strlen(start) : (size_t)(end - start);
        unsigned bit = length == 5 && memcmp(start, "entry", 5) == 0 ? LIBRARY_ENTRY
            : length == 6 && memcmp(start, "resume", 6) == 0 ? LIBRARY_RESUME
            : length == 4 && memcmp(start, "read", 4) == 0 ? LIBRARY_READ : 0;
        if (bit == 0 || (result & bit) != 0) { return 0; }
        result |= bit;
        if (end == NULL) { return result; }
        start = end + 1;
        if (*start == '\0') { return 0; }
    }
    return result;
}
/** Append a schema without discovering a transport from its native payload. */
static bool schema_record(LibraryBuild *build, SourceSpan site, char **v, SourceSpan *sites) {
    static const char *const kinds[] = {"integer", "text", "condition", "failure", "list"};
    size_t kind = 0;
    while (kind < sizeof(kinds) / sizeof(*kinds) && strcmp(v[2], kinds[kind]) != 0) { kind++; }
    if (!library_public_identifier(v[1]) || library_schema_find(build, v[1]) != NULL
        || kind == sizeof(kinds) / sizeof(*kinds)) {
        return library_fail(build, site, "Invalid, duplicate or unsupported public transport schema");
    }
    LibrarySchema *schema = library_allocate(build, sizeof(*schema));
    if (schema == NULL) { return false; }
    *schema = (LibrarySchema){.name = v[1], .argument = v[3], .argument_site = sites[3], .kind = (LibrarySchemaKind)kind, .site = site};
    LibrarySchema **tail = &build->schemas;
    while (*tail != NULL) { tail = &(*tail)->next; }
    *tail = schema;
    return true;
}
/** Append an explicitly named executable export with its exact operation set. */
static bool export_record(LibraryBuild *build, SourceSpan site, char **v, SourceSpan *sites) {
    unsigned selected = operations(v[3]);
    if (!library_public_identifier(v[1]) || !module_path_valid(v[2]) || selected == 0
        || library_export_find(build, v[1]) != NULL || build->export_count == SIZE_MAX) {
        return library_fail(build, site, "Invalid or duplicate library export, module path or operation set");
    }
    LibraryExport *exported = library_allocate(build, sizeof(*exported));
    if (exported == NULL) { return false; }
    *exported = (LibraryExport){.name = v[1], .module = v[2], .module_site = sites[2], .operations = selected, .site = site};
    LibraryExport **tail = &build->exports;
    while (*tail != NULL) { tail = &(*tail)->next; }
    *tail = exported;
    build->export_count++;
    return true;
}
/** Retain named mappings; resolution checks exact fields, uniqueness and completeness. */
static bool binding_record(LibraryBuild *build, SourceSpan site, char **v) {
    LibraryBinding *binding = library_allocate(build, sizeof(*binding));
    if (binding == NULL) { return false; }
    *binding = (LibraryBinding){.export_name = v[1], .field_name = v[2], .schema_name = v[3],
        .input = strcmp(v[0], "input") == 0, .site = site};
    LibraryBinding **tail = &build->bindings;
    while (*tail != NULL) { tail = &(*tail)->next; }
    *tail = binding;
    return true;
}
bool library_configuration_record(LibraryBuild *build, SourceSpan site) {
    const unsigned char *bytes = source_bytes(site.source);
    size_t key_end = site.start;
    while (site.end - key_end >= 3 && memcmp(bytes + key_end, " | ", 3) != 0) { key_end++; }
    if (site.end - key_end < 3) { return library_fail(build, site, "Library records require an exact ' | ' separator"); }
    char *v[4];
    SourceSpan key_site = {site.source, site.start, key_end};
    if (!fields(build, key_site, v, 1, NULL)) { return false; }
    const char *key = v[0];
    size_t count = strcmp(key, "schema") == 0 || strcmp(key, "export") == 0
        || strcmp(key, "input") == 0 || strcmp(key, "output") == 0 ? 4 : 2;
    SourceSpan sites[4];
    if (!fields(build, site, v, count, sites)) { return false; }
    if (strcmp(key, "ama-library") == 0) {
        if (build->format_seen || build->component != NULL || build->exports != NULL || build->schemas != NULL
            || strcmp(v[1], "1") != 0) { return library_fail(build, site, "The unique first library format record must select edition 1"); }
        build->format_seen = true;
        return true;
    }
    if (!build->format_seen) { return library_fail(build, site, "The first noncomment record must be 'ama-library | 1'"); }
    if (strcmp(key, "component") == 0) { return scalar(build, site, &build->component, v[1]); }
    if (strcmp(key, "abi") == 0) { return scalar(build, site, &build->abi, v[1]); }
    if (strcmp(key, "target") == 0) {
        bool valid = scalar(build, site, &build->target, v[1]);
        if (valid) { build->target_site = site; }
        return valid;
    }
    if (strcmp(key, "calling-convention") == 0) { return scalar(build, site, &build->calling_convention, v[1]); }
    if (strcmp(key, "schema") == 0) { return schema_record(build, site, v, sites); }
    if (strcmp(key, "export") == 0) { return export_record(build, site, v, sites); }
    if (strcmp(key, "input") == 0 || strcmp(key, "output") == 0) { return binding_record(build, site, v); }
    if (strcmp(key, "c-flag") == 0 || strcmp(key, "link-flag") == 0 || strcmp(key, "link-file") == 0 || strcmp(key, "dependency-file") == 0) {
        LibrarySetting *setting = library_allocate(build, sizeof(*setting));
        if (setting == NULL) { return false; }
        *setting = (LibrarySetting){.kind = key, .value = v[1], .value_site = sites[1], .site = site};
        LibrarySetting **tail = &build->settings;
        while (*tail != NULL) { tail = &(*tail)->next; }
        *tail = setting;
        return true;
    }
    return library_fail(build, site, "Unknown library configuration record");
}
