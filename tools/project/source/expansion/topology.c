/** @file
 * @brief Derive context and descendant membership from exact original logical ancestry.
 * [PKG-07](../../../../spec/project/packages.md#pkg-07) retains ancestor mains
 * without activating their siblings. Physical plus directories never enter identity.
 */
#include <stdint.h>
#include <string.h>
#include "internal.h"
/** Test a component boundary, excluding equality and lookalike prefixes. */
static bool below(const char *child, const char *parent) {
    size_t length = strlen(parent);
    return !strncmp(child, parent, length) && child[length] == '/';
}
AcquisitionClosureStatus source_expansion_topology(SourceExpansion *expansion,
    const ModuleSource *source, AcquisitionExpansion *fact) {
    size_t count = 0;
    for (const SourceCapturedUnit *unit = expansion->capture->units; unit; unit = unit->next) {
        if (below(unit->source->logical_path, source->logical_path)) {
            if (count == SIZE_MAX) { return ACQUISITION_CLOSURE_OUT_OF_MEMORY; }
            count++;
        }
    }
    if (count > SIZE_MAX / sizeof(const char *)) { return ACQUISITION_CLOSURE_OUT_OF_MEMORY; }
    const char **children = count ? source_arena_allocate(&expansion->arena, count * sizeof(*children)) : NULL;
    if (count && !children) { return ACQUISITION_CLOSURE_OUT_OF_MEMORY; }
    size_t index = 0;
    AcquisitionContextFact *contexts = NULL;
    for (const SourceCapturedUnit *unit = expansion->capture->units; unit; unit = unit->next) {
        if (below(unit->source->logical_path, source->logical_path)) { children[index++] = unit->source->logical_path; }
        if (!below(source->logical_path, unit->source->logical_path)) { continue; }
        AcquisitionContextFact *context = source_arena_allocate(&expansion->arena, sizeof(*context));
        if (!context) { return ACQUISITION_CLOSURE_OUT_OF_MEMORY; }
        *context = (AcquisitionContextFact){contexts, unit->source->logical_path, unit->files};
        contexts = context;
    }
    fact->contexts = contexts;
    fact->submodules = children;
    fact->submodule_count = count;
    return ACQUISITION_CLOSURE_OK;
}
