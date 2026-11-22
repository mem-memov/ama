/** @file
 * @brief Validate original candidate ownership and intern each independent admission once.
 * [PKG-08](../../../../spec/project/packages.md#pkg-08) distinguishes test roots from support;
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) retains cached failed proofs.
 */
#include <string.h>
#include "internal.h"
/** A candidate witness identifies who requested proof; actual failure diagnostics
 * remain the compiler's retained raw stream under
 * [PKG-08](../../../../spec/project/packages.md#pkg-08).
 */
static SourceIssue witness(const SourceAnalysis *analysis, const DeliverySourceDefinition *definition) {
    SourceIssue issue = {.expected = "See the retained compiler diagnostic stream for the actual failure"};
    for (size_t index = 0; index < analysis->view.count; index++) {
        const ModuleSnapshotEntry *entry = &analysis->view.entries[index];
        if (!strcmp(entry->logical_path, definition->identity)) { issue.span = (SourceSpan){entry->text, 0, 0}; break; }
    }
    return issue;
}
AcquisitionClosureStatus source_analysis_admit_test(void *context, const DeliverySourceDefinition *definition, SourceIssue *issue) {
    SourceAnalysis *analysis = context;
    if (!analysis || !issue) { return ACQUISITION_CLOSURE_INVALID; }
    analysis->last_probe = NULL;
    *issue = (SourceIssue){.message = "Independent test candidate does not belong to the admitted original domain"};
    if (!analysis->valid) { return ACQUISITION_CLOSURE_PREPARATION_FAILED; }
    const DeliverySourceDefinition *owned = analysis->original.response.definitions;
    while (owned && owned != definition) { owned = owned->next; }
    if (!owned || (owned->kind != DELIVERY_SOURCE_CONCRETE && owned->kind != DELIVERY_SOURCE_PARAMETERIZED)) {
        return ACQUISITION_CLOSURE_INVALID;
    }
    for (SourceAnalysisProbe *probe = analysis->probes; probe; probe = probe->next) {
        if (probe->definition != owned) { continue; }
        analysis->last_probe = probe; *issue = probe->issue; return probe->status;
    }
    SourceAnalysisProbe *probe = source_arena_allocate(&analysis->arena, sizeof(*probe));
    if (!probe) {
        *issue = witness(analysis, owned); issue->message = "Cannot retain independent candidate proof";
        return ACQUISITION_CLOSURE_OUT_OF_MEMORY;
    }
    *probe = (SourceAnalysisProbe){.next = analysis->probes, .definition = owned, .issue = witness(analysis, owned)};
    source_compiler_client_init(&probe->compiler, analysis->arena.allocator);
    analysis->probes = probe; analysis->last_probe = probe;
    source_analysis_probe(analysis, probe); *issue = probe->issue; return probe->status;
}
