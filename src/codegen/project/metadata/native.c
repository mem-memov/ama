/** @file
 * @brief Serialize selected native companions and nonfatal package guidance.
 * [BUILD-01](../../../../spec/implementation/application-builds.md#build-01)
 * binds exact snapshots to concrete component origins;
 * [BUILD-05](../../../../spec/implementation/application-builds.md#build-05)
 * records absent/unavailable README guidance without fabricating instructions.
 */
#include <string.h>
#include "internal.h"

/** Keep every physical path byte and exact binary file payload under
 * [BUILD-01](../../../../spec/implementation/application-builds.md#build-01).
 * Paths do not assert that a subsequent external mutation is equivalent.
 */
static bool file(ToolJsonWriter *writer, const char *path, const SourceText *content) {
    return tool_json_raw(writer, "{\"path\":")
        && (path == NULL ? tool_json_raw(writer, "null")
            : delivery_metadata_bytes(writer, (const unsigned char *)path, strlen(path)))
        && tool_json_raw(writer, ",\"content\":")
        && (content == NULL ? tool_json_raw(writer, "null")
            : delivery_metadata_bytes(writer, source_bytes(content), source_length(content)))
        && tool_json_raw(writer, "}");
}
/** Encode all optional-file states explicitly, retaining exact failed paths;
 * [BUILD-05](../../../../spec/implementation/application-builds.md#build-05)
 * forbids disguising unavailable guidance as absent.
 */
static bool optional(ToolJsonWriter *writer, const DeliveryFileSnapshot *snapshot) {
    if (snapshot == NULL) { return tool_json_raw(writer, "null"); }
    const char *state = snapshot->state == DELIVERY_FILE_PRESENT ? "present"
        : snapshot->state == DELIVERY_FILE_ABSENT ? "absent" : "unavailable";
    const char *issue = snapshot->issue == PATH_IO_ERROR ? "io" : "shape";
    return tool_json_raw(writer, "{\"state\":") && tool_json_string(writer, state)
        && tool_json_raw(writer, ",\"issue\":")
        && (snapshot->state == DELIVERY_FILE_UNAVAILABLE ? tool_json_string(writer, issue) : tool_json_raw(writer, "null"))
        && tool_json_raw(writer, ",\"file\":") && file(writer, snapshot->path, snapshot->content)
        && tool_json_raw(writer, "}");
}

/** Serialize the declared module-owned unit without pretending it exhausts
 * external CMake/provider inputs under
 * [BUILD-01](../../../../spec/implementation/application-builds.md#build-01).
 */
static bool resources(ToolJsonWriter *writer, const DeliveryNativeResource *first) {
    if (!tool_json_raw(writer, "[")) { return false; }
    for (const DeliveryNativeResource *row = first; row != NULL; row = row->next) {
        if ((row != first && !tool_json_raw(writer, ","))
            || !tool_json_raw(writer, "{\"directory\":") || !tool_json_boolean(writer, row->directory)
            || !tool_json_raw(writer, ",\"snapshot\":") || !optional(writer, &row->file)
            || !tool_json_raw(writer, "}")) { return false; }
    }
    return tool_json_raw(writer, "]");
}

/** Link each native origin to all actual concrete descriptor IDs under
 * [BUILD-01](../../../../spec/implementation/application-builds.md#build-01),
 * using the existing model's IDs rather than inventing another type registry.
 */
static bool type_ids(ToolJsonWriter *writer, const NativeScaffold *scaffold, const ModuleSource *source) {
    if (!tool_json_raw(writer, "[")) { return false; }
    bool first = true;
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        if (type->plan->type->module->source != source) { continue; }
        if ((!first && !tool_json_raw(writer, ","))
            || !tool_json_unsigned(writer, codegen_type_id(scaffold, type))) { return false; }
        first = false;
    }
    return tool_json_raw(writer, "]");
}

bool delivery_metadata_native(ToolJsonWriter *writer, const NativeScaffold *scaffold) {
    if (!tool_json_raw(writer, ",\"native\":[")) { return false; }
    bool first = true;
    for (const DeliveryNativeCapture *row = scaffold->native_inputs; row != NULL; row = row->next) {
        const ModuleSource *source = row->origin->source;
        const SourceText *body = project_inputs_content(&scaffold->frontend->inputs, PROJECT_INPUT_FILE, source->native_c);
        const SourceText *header = project_inputs_content(&scaffold->frontend->inputs, PROJECT_INPUT_FILE, source->native_header);
        if ((source->native_c != NULL && body == NULL) || (source->native_header != NULL && header == NULL)) { return false; }
        if ((!first && !tool_json_raw(writer, ",")) || !tool_json_raw(writer, "{\"module\":")
            || !tool_json_string(writer, source->logical_path)
            || !tool_json_raw(writer, ",\"types\":") || !type_ids(writer, scaffold, source)
            || !tool_json_raw(writer, ",\"version\":")
            || (source->version == NULL ? !tool_json_raw(writer, "null") : !tool_json_string(writer, source->version->logical_path))
            || !tool_json_raw(writer, ",\"version_root\":")
            || (source->version == NULL ? !tool_json_raw(writer, "null")
                : !delivery_metadata_bytes(writer, (const unsigned char *)source->version->physical_path, strlen(source->version->physical_path)))
            || !tool_json_raw(writer, ",\"c\":") || !file(writer, source->native_c, body)
            || !tool_json_raw(writer, ",\"h\":") || !file(writer, source->native_header, header)
            || !tool_json_raw(writer, ",\"abi\":")
            || !file(writer, source->native_abi == NULL ? NULL : source_path(source->native_abi), source->native_abi)
            || !tool_json_raw(writer, ",\"configuration\":")
            || !file(writer, source->configuration_path, source->configuration)
            || !tool_json_raw(writer, ",\"resources\":") || !resources(writer, row->resources)
            || !tool_json_raw(writer, ",\"cmake\":") || !optional(writer, &row->cmake)
            || !tool_json_raw(writer, ",\"readme\":") || !optional(writer, row->guidance)
            || !tool_json_raw(writer, "}")) { return false; }
        first = false;
    }
    if (!tool_json_raw(writer, "],\"guidance\":[")) { return false; }
    first = true;
    for (const DeliveryVersionGuidance *row = scaffold->guidance; row != NULL; row = row->next) {
        const char *path = row->version->physical_path;
        if ((!first && !tool_json_raw(writer, ",")) || !tool_json_raw(writer, "{\"version\":")
            || !tool_json_string(writer, row->version->logical_path)
            || !tool_json_raw(writer, ",\"root\":")
            || !delivery_metadata_bytes(writer, (const unsigned char *)path, strlen(path))
            || !tool_json_raw(writer, ",\"readme\":") || !optional(writer, row->readme)
            || !tool_json_raw(writer, "}")) { return false; }
        first = false;
    }
    return tool_json_raw(writer, "]");
}
