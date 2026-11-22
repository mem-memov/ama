/** @file
 * @brief Join explicit original input ownership to structural facts and indexed data-main activation.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * retains complete module units even when selected as data;
 * [PKG-07](../../../../spec/project/packages.md#pkg-07) requires declared opaque
 * inputs rather than C/CMake scans or inferred neighboring resources.
 */
#include <string.h>
#include "internal.h"
AcquisitionClosureStatus source_expansion_declared(SourceExpansion *expansion,
    const ModuleSource *source, AcquisitionExpansion *fact) {
    AcquisitionDeclaredInputs *inputs = NULL;
    AcquisitionDeclaredModule *modules = NULL;
    for (const SourceDeclaredGroup *group = expansion->declared; group; group = group->next) {
        if (group->requester->source != source) { continue; }
        AcquisitionExpansion origin = {0};
        AcquisitionClosureStatus status = source_expansion_version(expansion, group->version, &origin);
        if (status != ACQUISITION_CLOSURE_OK) { return status; }
        AcquisitionDeclaredInputs *input = source_arena_allocate(&expansion->arena, sizeof(*input));
        if (!input) { return ACQUISITION_CLOSURE_OUT_OF_MEMORY; }
        *input = (AcquisitionDeclaredInputs){.next = inputs,
            .source = origin.source, .files = group->files};
        inputs = input;
        for (const AcquisitionUnitEntry *file = group->files; file; file = file->next) {
            for (const SourceCapturedUnit *unit = expansion->capture->units; unit; unit = unit->next) {
                if (strcmp(file->file.path, source_path(unit->source->text))) { continue; }
                AcquisitionDeclaredModule *module = source_arena_allocate(&expansion->arena, sizeof(*module));
                if (!module) { return ACQUISITION_CLOSURE_OUT_OF_MEMORY; }
                /* This is the canonical original data-file observation, not an
                 * invented authored path occurrence. Equal repeated captures
                 * therefore retain one exact role cause. */
                *module = (AcquisitionDeclaredModule){.next = modules,
                    .identity = unit->source->logical_path, .origin = {unit->source->text, 0, 0}};
                modules = module;
            }
        }
    }
    fact->declared_inputs = inputs;
    fact->declared_modules = modules;
    return ACQUISITION_CLOSURE_OK;
}
