/** @file
 * @brief Derive a closed introduction set from actual final units outside conserved originals and C.
 * [TOOL-02](../../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * permits independent final whole-version selection but no arbitrary new code.
 */
#include "internal.h"
#include <stdint.h>
#include <string.h>
/** An explicitly mapped copy or retained original remains under the existing
 * conservation proof. Only other final units need supplier provenance. */
static bool candidate(const SourceCloneMap *map, SourceRegistryRetirement retired, const SourceCapturedUnit *unit) {
    const char *name = unit->source->logical_path;
    if (source_copy_introduction_clone(map, name, false)) { return false; }
    const SourceCapturedUnit *old = map->capture->units;
    while (old && strcmp(old->source->logical_path, name)) { old = old->next; }
    if (!old) { return true; }
    for (size_t i = 0; i < retired.count; i++) { if (!strcmp(retired.names[i], name)) { return true; } }
    return false;
}
SourceIntroductionStatus source_copy_introduction_prepare(SourceArena *arena, const SourceObservedImage *observed,
    const SourceImageCapture *final, const SourceCloneMap *map, SourceRegistryRetirement retired,
    const DeliverySourceDefinition *original, const DeliverySourceDefinition *definitions,
    SourceCopyIntroduction *output, SourceIntroductionIssue *issue) {
    if (!issue) { return SOURCE_INTRODUCTION_INVALID; } *issue = (SourceIntroductionIssue){0};
    if (!arena || !observed || !observed->valid || !observed->sealed || !final || !final->valid
        || !map || !map->ready || observed->original != map->capture || !output
        || (retired.count && !retired.names)) { return SOURCE_INTRODUCTION_INVALID; }
    for (size_t i = 0; i < retired.count; i++) { if (!retired.names[i]) { return SOURCE_INTRODUCTION_INVALID; } }
    size_t count = 0;
    for (const SourceCapturedUnit *unit = final->units; unit; unit = unit->next) {
        if (candidate(map, retired, unit)) { if (count == SIZE_MAX) { return SOURCE_INTRODUCTION_MEMORY; } count++; }
    }
    if (count > SIZE_MAX / sizeof(const SourceCapturedUnit *)) { return SOURCE_INTRODUCTION_MEMORY; }
    SourceCopyIntroduction result = {.count = count};
    result.units = count ? source_arena_allocate(arena, count * sizeof(*result.units)) : NULL;
    if (count && !result.units) { return SOURCE_INTRODUCTION_MEMORY; }
    size_t index = 0;
    for (const SourceCapturedUnit *unit = final->units; unit; unit = unit->next) {
        if (!candidate(map, retired, unit)) { continue; }
        SourceIntroductionStatus status = source_copy_introduction_origin(arena, observed, unit, issue);
        if (status != SOURCE_INTRODUCTION_OK) { return status; }
        result.units[index++] = unit;
    }
    SourceIntroductionStatus status = source_copy_introduction_graph(arena, &result, map, original, definitions,
        &result.conserved, issue);
    if (status != SOURCE_INTRODUCTION_OK) { return status; }
    *output = result; *issue = (SourceIntroductionIssue){0}; return SOURCE_INTRODUCTION_OK;
}
