/** @file
 * @brief Keep structural captures alive independently of original compiler proofs.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) requires
 * deterministic reverse cleanup of complete and failed preparation.
 */
#include <string.h>
#include "expansion.h"
void source_expansion_init(SourceExpansion *expansion, SourceAllocator allocator,
    const SourceRootCapture *capture, SourceAnalysis *analysis, const SourceDeclaredGroup *declared) {
    memset(expansion, 0, sizeof(*expansion));
    source_arena_init(&expansion->arena, allocator);
    source_manager_init(&expansion->files, allocator);
    expansion->capture = capture;
    expansion->analysis = analysis;
    expansion->declared = declared;
}
void source_expansion_destroy(SourceExpansion *expansion) {
    source_arena_destroy(&expansion->arena);
    source_manager_destroy(&expansion->files);
    memset(expansion, 0, sizeof(*expansion));
}
AcquisitionClosureStatus source_expansion_admit(void *context,
    const DeliverySourceDefinition *definition, SourceIssue *issue) {
    SourceExpansion *expansion = context;
    if (!expansion) {
        if (issue) { *issue = (SourceIssue){.message = "Independent admission lacks its structural provider owner"}; }
        return ACQUISITION_CLOSURE_INVALID;
    }
    return source_analysis_admit_test(expansion->analysis, definition, issue);
}
