/** @file
 * @brief Collect authoritative active ordinary roles and exact selected-tree destination seeds.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * keeps explicit placement distinct from dependency placement;
 * [PKG-08](../../../../../spec/project/packages.md#pkg-08) associates tests with
 * checked source definitions, without inventing tests for inactive draft mains.
 */
#include "internal.h"
#include "source/path/logical.h"
#include <stdint.h>
#include <string.h>
const SourceCloneMapping *source_copy_destination_explicit(const SourceCopyDestinations *result,
    const ModuleSource *source) {
    for (const SourceCloneMapping *row = result->explicit_map->first; row; row = row->next) {
        if (row->original == source) { return row; }
    }
    return NULL;
}
/** The supplied relation contains exactly the actually selected indexed mains. */
static bool explicit_coverage(const SourceCopyDestinations *result) {
    const SourceTreeSelection *tree = result->copy->tree;
    for (const SourceCloneMapping *row = result->explicit_map->first; row; row = row->next) {
        bool present = false;
        for (size_t i = 0; i < tree->count; i++) { present |= tree->units[i]->source == row->original; }
        if (!present) { return false; }
    }
    for (size_t i = 0; i < tree->count; i++) {
        if (!source_copy_destination_explicit(result, tree->units[i]->source)) { return false; }
    }
    return true;
}
/** Checked whole-domain absence is the admitted draft distinction, not a guess
 * from file contents or a failed partial semantic compilation. */
static bool checked_source(const SourceCopyDestinations *result, const char *identity) {
    const DeliverySourceDefinition *definition = result->copy->expansion.analysis->original.response.definitions;
    for (; definition; definition = definition->next) {
        if (!strcmp(definition->identity, identity)) { return true; }
    }
    return false;
}
/** Copy a canonical final version prefix from an explicit ordinary module name. */
static SourceCopyDestinationsStatus fixed_version(SourceCopyDestinations *result,
    const SourceCloneMapping *row, const char **output) {
    if (!row) { *output = NULL; return SOURCE_COPY_DESTINATIONS_OK; }
    ModulePathShape shape;
    if (!module_path_describe((const unsigned char *)row->identity, strlen(row->identity), &shape) ||
        shape.extent != MODULE_PATH_MODULE || shape.root != MODULE_ROOT_PROJECT) {
        return SOURCE_COPY_DESTINATIONS_NAMESPACE;
    }
    char *version = source_arena_allocate(&result->arena, shape.version_length + 1);
    if (!version) { return SOURCE_COPY_DESTINATIONS_MEMORY; }
    memcpy(version, row->identity, shape.version_length); version[shape.version_length] = 0;
    *output = version; return SOURCE_COPY_DESTINATIONS_OK;
}
SourceCopyDestinationsStatus source_copy_destination_units(SourceCopyDestinations *result,
    SourceCopyDestinationWork *work) {
    if (!explicit_coverage(result)) { return SOURCE_COPY_DESTINATIONS_INVALID; }
    for (const AcquisitionRoleRecord *role = result->copy->closure.roles.records; role; role = role->next) {
        if (role->role == ACQUISITION_ACTIVE && strncmp(role->identity, "//", 2)) { result->count++; }
    }
    size_t count = result->count;
    if (!count) { return SOURCE_COPY_DESTINATIONS_OK; }
    if (count > SIZE_MAX / sizeof(*work->entries) || count > SIZE_MAX / sizeof(*work->units) ||
        count > SIZE_MAX / sizeof(*work->associations)) { return SOURCE_COPY_DESTINATIONS_MEMORY; }
    work->entries = source_arena_allocate(&result->arena, count * sizeof(*work->entries));
    work->units = source_arena_allocate(&result->arena, count * sizeof(*work->units));
    work->associations = source_arena_allocate(&result->arena, count * sizeof(*work->associations));
    if (!work->entries || !work->units || !work->associations) { return SOURCE_COPY_DESTINATIONS_MEMORY; }
    size_t index = 0;
    for (const AcquisitionRoleRecord *role = result->copy->closure.roles.records; role; role = role->next) {
        if (role->role != ACQUISITION_ACTIVE || !strncmp(role->identity, "//", 2)) { continue; }
        result->failed_role = role;
        const ModuleSource *source = NULL;
        if (module_catalog_find(&result->copy->expansion.capture->catalog, role->identity, &source) != MODULE_OK) {
            return SOURCE_COPY_DESTINATIONS_INVALID;
        }
        work->entries[index] = (SourceCopyDestination){.source = source, .role = role};
        work->units[index] = (SourceVersionUnit){.original = source}; work->associations[index] = NULL;
        SourceCopyDestinationsStatus status = fixed_version(result,
            source_copy_destination_explicit(result, source), &work->units[index].fixed_version);
        if (status != SOURCE_COPY_DESTINATIONS_OK) { return status; }
        ModulePathShape shape;
        if (!module_path_describe((const unsigned char *)role->identity, strlen(role->identity), &shape) ||
            shape.extent != MODULE_PATH_MODULE) { return SOURCE_COPY_DESTINATIONS_INVALID; }
        if (shape.area == MODULE_PATH_SRC && checked_source(result, role->identity)) {
            SourceStatus association = acquisition_test_namespace(&result->arena, role->identity, &work->associations[index]);
            if (association != SOURCE_OK) {
                return association == SOURCE_OUT_OF_MEMORY ? SOURCE_COPY_DESTINATIONS_MEMORY : SOURCE_COPY_DESTINATIONS_INVALID;
            }
        }
        index++;
    }
    return SOURCE_COPY_DESTINATIONS_OK;
}
