/** @file
 * @brief Join exact original catalog identities to compiler-checked structural closure facts.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires one original domain; [PKG-07](../../../../spec/project/packages.md#pkg-07)
 * preserves drafts and full units; [PKG-08](../../../../spec/project/packages.md#pkg-08)
 * associates experiments only with active ordinary source modules.
 */
#include <string.h>
#include "internal.h"
#include "source/path/logical.h"
/** Active source associations are required even if no test file is executable;
 * exact eligibility and missing concrete experiments remain the closure's job. */
static AcquisitionClosureStatus tests(SourceExpansion *expansion, AcquisitionExpansion *fact) {
    AcquisitionTestDirectory *directory = source_arena_allocate(&expansion->arena, sizeof(*directory));
    if (!directory) { return ACQUISITION_CLOSURE_OUT_OF_MEMORY; }
    SourceStatus status = acquisition_test_directory(&expansion->arena, fact->module, directory);
    if (status != SOURCE_OK) {
        return status == SOURCE_OUT_OF_MEMORY || status == SOURCE_LIMIT
            ? ACQUISITION_CLOSURE_OUT_OF_MEMORY : ACQUISITION_CLOSURE_INVALID;
    }
    const char *slash = strrchr(directory->physical, '/');
    if (!slash) { return ACQUISITION_CLOSURE_INVALID; }
    char *parent = source_arena_copy(&expansion->arena, directory->physical);
    if (!parent) { return ACQUISITION_CLOSURE_OUT_OF_MEMORY; }
    parent[slash - directory->physical] = 0;
    expansion->file_status = acquisition_directory_capture(&expansion->arena, &expansion->files,
        *parent ? parent : "/", slash + 1, &fact->test_files, &expansion->failed_path);
    if (expansion->file_status != SOURCE_OK) {
        return expansion->file_status == SOURCE_OUT_OF_MEMORY || expansion->file_status == SOURCE_LIMIT
            ? ACQUISITION_CLOSURE_OUT_OF_MEMORY : ACQUISITION_CLOSURE_PREPARATION_FAILED;
    }
    fact->test_directory = directory;
    return ACQUISITION_CLOSURE_OK;
}
AcquisitionClosureStatus source_expansion_prepare(void *context, const char *identity,
    AcquisitionExpansion *output, SourceIssue *issue) {
    SourceExpansion *expansion = context;
    if (!issue) { return ACQUISITION_CLOSURE_INVALID; }
    if (!identity || !output) {
        *issue = (SourceIssue){.message = "Structural expansion requires an identity and output"};
        return ACQUISITION_CLOSURE_INVALID;
    }
    AcquisitionClosureStatus admitted = source_expansion_validate(expansion, issue);
    if (admitted != ACQUISITION_CLOSURE_OK) { return admitted; }
    ModulePathShape shape;
    if (!module_path_describe((const unsigned char *)identity, strlen(identity), &shape)) {
        issue->message = "Structural expansion requires a canonical module or selected version identity";
        return ACQUISITION_CLOSURE_INVALID;
    }
    if (shape.extent == MODULE_PATH_VERSION) {
        const ModuleVersion *version = expansion->capture->catalog.versions;
        while (version && strcmp(version->logical_path, identity)) { version = version->next; }
        if (!version) {
            issue->message = "Structural expansion version is not originally selected";
            return ACQUISITION_CLOSURE_INVALID;
        }
        AcquisitionExpansion fact = {.definitions = expansion->analysis->original.response.definitions};
        AcquisitionClosureStatus status = source_expansion_scope(expansion, version, &fact);
        if (status != ACQUISITION_CLOSURE_OK) {
            issue->message = "Original whole-version structural expansion failed";
            return status;
        }
        *output = fact;
        *issue = (SourceIssue){0};
        return ACQUISITION_CLOSURE_OK;
    }
    const ModuleSource *source = NULL;
    if (module_catalog_find(&expansion->capture->catalog, identity, &source) != MODULE_OK) {
        issue->message = "Structural expansion identity is not an original selected module";
        return ACQUISITION_CLOSURE_INVALID;
    }
    AcquisitionExpansion fact = {.module = source, .definitions = expansion->analysis->original.response.definitions};
    const DeliverySourceDefinition *definition = fact.definitions;
    while (definition && strcmp(definition->identity, identity)) { definition = definition->next; }
    fact.draft = !definition;
    AcquisitionClosureStatus status = source_expansion_files(expansion, source, &fact.files);
    if (status == ACQUISITION_CLOSURE_OK) { status = source_expansion_version(expansion, source->version, &fact); }
    if (status == ACQUISITION_CLOSURE_OK) { status = source_expansion_topology(expansion, source, &fact); }
    if (status == ACQUISITION_CLOSURE_OK && definition && shape.area == MODULE_PATH_SRC) {
        status = tests(expansion, &fact);
    }
    if (status == ACQUISITION_CLOSURE_OK) { status = source_expansion_declared(expansion, source, &fact); }
    if (status != ACQUISITION_CLOSURE_OK) {
        issue->span = (SourceSpan){source->text, 0, 0};
        issue->message = status == ACQUISITION_CLOSURE_CONFLICT
            ? "Structural observation changed an original captured unit" : "Original structural expansion failed";
        return status;
    }
    *output = fact;
    *issue = (SourceIssue){0};
    return ACQUISITION_CLOSURE_OK;
}
