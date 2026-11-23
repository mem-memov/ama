/** @file
 * @brief Derive complete final permission expectations only from actual immutable scope authority.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves original finite selections plus explicit additions; final parsing
 * alone is not authority for a caller-invented permission set.
 */
#include "internal.h"
#include <stdint.h>
#include <string.h>
/** Byte equality is independent of source-owner identity and allocation. */
static bool same_text(const SourceText *left, const SourceText *right) {
    return left && right && source_length(left) == source_length(right)
        && !memcmp(source_bytes(left), source_bytes(right), source_length(left));
}
/** Final scope keys include physical version, not merely equal nominal names. */
static const SourcePackageScope *original_scope(const SourceOperationComplete *owner, const SourcePackageScope *actual) {
    for (const SourcePackageScope *old = owner->original_environment.first; old; old = old->next) {
        if (!strcmp(old->version->logical_path, actual->version->logical_path)
            && !strcmp(old->version->physical_path, actual->version->physical_path)) { return old; }
    }
    return NULL;
}
/** An unchanged final supplier must already have existed, suppressed only by
 * whole-version precedence in U. Final complete source/J proof remains required. */
static bool supplier(const SourceOperationComplete *owner, const SourcePackageScope *actual) {
    const SourceObservedImage *observed = owner->writes.inputs.observed;
    const ModuleVersion *old = observed->original->catalog.versions;
    while (old && strcmp(old->logical_path, actual->version->logical_path)) { old = old->next; }
    if (!old || !strcmp(old->physical_path, actual->version->physical_path)) { return false; }
    const char *path = source_path(actual->document.source);
    for (size_t i = 0; i < observed->image.count; i++) {
        const SourceCatalogImageEntry *entry = &observed->image.entries[i];
        if (!strcmp(entry->path, path)) {
            return entry->metadata_captured && S_ISREG(entry->metadata.st_mode) && same_text(entry->content, actual->document.source);
        }
    }
    return false;
}
/** Derive a no-new-permissions selection from its actual baseline. Missing old
 * matches disappear under PKG-05; new wildcard matches cannot enter this set. */
static SourceOperationCompleteStatus preserved(SourceOperationComplete *owner, size_t index,
    const SourcePackageScope *actual, const SourcePackageScope *before, SourceOperationScopeOrigin kind) {
    SourceOperationConfigurationExpectation *expected = &owner->configurations[index];
    expected->environment = &owner->plan.environment; expected->scope = actual;
    owner->selection_status = source_copy_selected_roots(&owner->arena, before->selection, owner->plan.environment.view,
        NULL, false, NULL, 0, &expected->roots, &owner->selection_issue);
    if (owner->selection_status != SOURCE_SELECTION_ROOTS_OK) {
        return owner->selection_status == SOURCE_SELECTION_ROOTS_MEMORY ? SOURCE_OPERATION_COMPLETE_MEMORY : SOURCE_OPERATION_COMPLETE_CONFIGURATION;
    }
    owner->authorities[index] = (SourceOperationScopeAuthority){.kind = kind, .baseline = before};
    return SOURCE_OPERATION_COMPLETE_OK;
}
/** First route actual owned writes; no carried config can masquerade as untouched. */
static SourceOperationCompleteStatus scope(SourceOperationComplete *owner, size_t index, const SourcePackageScope *actual) {
    const SourceOperationWritesInputs *inputs = &owner->writes.inputs;
    const char *path = source_path(actual->document.source); owner->failed_path = path;
    for (size_t i = 0; i < inputs->configuration_count; i++) {
        const SourceOperationConfigurationWrite *writer = inputs->configurations[i];
        if (strcmp(source_path(writer->result), path)) { continue; }
        if (!same_text(writer->result, actual->document.source)) { return SOURCE_OPERATION_COMPLETE_CONFIGURATION; }
        owner->configurations[index] = (SourceOperationConfigurationExpectation){writer->inputs.preview, writer->inputs.after, writer->roots};
        owner->authorities[index] = (SourceOperationScopeAuthority){.kind = SOURCE_OPERATION_SCOPE_WRITER, .writer = writer};
        return SOURCE_OPERATION_COMPLETE_OK;
    }
    for (const SourceFileMapping *file = inputs->files->first; file; file = file->next) {
        if (!strcmp(file->destination, path)) { return SOURCE_OPERATION_COMPLETE_CONFIGURATION; }
    }
    const SourcePackageScope *before = original_scope(owner, actual);
    if (before) {
        if (!same_text(before->document.source, actual->document.source)) { return SOURCE_OPERATION_COMPLETE_CONFIGURATION; }
        return preserved(owner, index, actual, before, SOURCE_OPERATION_SCOPE_PRESERVED);
    }
    for (size_t i = 0; i < inputs->initialization_count; i++) {
        const SourceVersionInitialization *initialization = inputs->initializations[i];
        if (strcmp(source_path(initialization->configuration), path)) { continue; }
        if (source_length(actual->document.source)) { return SOURCE_OPERATION_COMPLETE_CONFIGURATION; }
        owner->configurations[index] = (SourceOperationConfigurationExpectation){.environment = &owner->plan.environment, .scope = actual};
        owner->authorities[index] = (SourceOperationScopeAuthority){.kind = SOURCE_OPERATION_SCOPE_INITIALIZED, .initialization = initialization};
        return SOURCE_OPERATION_COMPLETE_OK;
    }
    if (supplier(owner, actual)) { return preserved(owner, index, actual, actual, SOURCE_OPERATION_SCOPE_SUPPLIER); }
    return SOURCE_OPERATION_COMPLETE_CONFIGURATION;
}
SourceOperationCompleteStatus source_operation_complete_configurations(SourceOperationComplete *owner) {
    size_t count = 0;
    for (const SourcePackageScope *actual = owner->plan.environment.first; actual; actual = actual->next) {
        if (count == SIZE_MAX / sizeof(*owner->configurations) || count == SIZE_MAX / sizeof(*owner->authorities)) { return SOURCE_OPERATION_COMPLETE_MEMORY; }
        count++;
    }
    owner->configuration_count = count;
    owner->configurations = count ? source_arena_allocate(&owner->arena, count * sizeof(*owner->configurations)) : NULL;
    owner->authorities = count ? source_arena_allocate(&owner->arena, count * sizeof(*owner->authorities)) : NULL;
    if (count && (!owner->configurations || !owner->authorities)) { return SOURCE_OPERATION_COMPLETE_MEMORY; }
    size_t index = 0;
    for (const SourcePackageScope *actual = owner->plan.environment.first; actual; actual = actual->next) {
        SourceOperationCompleteStatus status = scope(owner, index++, actual);
        if (status != SOURCE_OPERATION_COMPLETE_OK) { return status; }
    }
    owner->failed_path = NULL; return SOURCE_OPERATION_COMPLETE_OK;
}
