/** @file
 * @brief Copy explicit root settings and final entry identity without deriving a namespace from directory names.
 * [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * returns changed root settings with the operation's result.
 */
#include <stdint.h>
#include "internal.h"

/** Every root's exclusions remain literal settings from the validated plan, not guessed physical references. */
static bool roots(RelocationReport *report, const ModuleRoot *input, size_t count, const ModuleRoot **output) {
    if (count > SIZE_MAX/sizeof(ModuleRoot)) { return relocation_report_memory(report); }
    ModuleRoot *copy = source_arena_allocate(&report->arena,count*sizeof(*copy));
    if (copy == NULL) { return relocation_report_memory(report); }
    for (size_t index = 0; index < count; index++) {
        copy[index] = (ModuleRoot){.kind=input[index].kind,.excluded_count=input[index].excluded_count};
        copy[index].path = source_arena_copy(&report->arena,input[index].path);
        size_t excluded_count = input[index].excluded_count;
        if (copy[index].path == NULL || excluded_count > SIZE_MAX/sizeof(char *)) { return relocation_report_memory(report); }
        const char **excluded = source_arena_allocate(&report->arena,excluded_count*sizeof(*excluded));
        if (excluded == NULL) { return relocation_report_memory(report); }
        copy[index].excluded = excluded;
        for (size_t item = 0; item < excluded_count; item++) {
            excluded[item] = source_arena_copy(&report->arena,input[index].excluded[item]);
            if (excluded[item] == NULL) { return relocation_report_memory(report); }
        }
    }
    *output = copy;
    return true;
}
/** Copy even unchanged paths so the report does not extend the lifetime of a compiler snapshot. */
bool relocation_report_settings(RelocationReport *report, const RelocationStaging *staging) {
    const RelocationPreview *preview = staging->references->preview;
    report->source = source_arena_copy(&report->arena,preview->source);
    report->destination = source_arena_copy(&report->arena,preview->destination);
    report->entry = staging->entry == NULL ? NULL : source_arena_copy(&report->arena,staging->entry);
    report->before_root_count = preview->before_root_count;
    report->after_root_count = preview->after_root_count;
    if (report->source == NULL || report->destination == NULL || (staging->entry != NULL && report->entry == NULL)) {
        return relocation_report_memory(report);
    }
    return roots(report,preview->before_roots,preview->before_root_count,&report->before_roots)
        && roots(report,preview->after_roots,preview->after_root_count,&report->after_roots);
}
