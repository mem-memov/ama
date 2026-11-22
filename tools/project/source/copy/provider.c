/** @file
 * @brief Apply the explicit copy dependency mode without changing checked original edges.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * clones ordinary dependencies only in the selected mode; standard references
 * retain their configured namespace under
 * [SRC-01](../../../../spec/language/source-and-names.md#src-01).
 */
#include <string.h>
#include "internal.h"
/** Structural facts retain the original provider's diagnostics and ownership. */
static AcquisitionClosureStatus expand(void *context, const char *identity,
    AcquisitionExpansion *output, SourceIssue *issue) {
    SourceCopyClosure *copy = context;
    return source_expansion_prepare(&copy->expansion, identity, output, issue);
}
/** Admission is an exact independently cached compiler proof, never test execution. */
static AcquisitionClosureStatus admit(void *context, const DeliverySourceDefinition *definition, SourceIssue *issue) {
    SourceCopyClosure *copy = context;
    return source_expansion_admit(&copy->expansion, definition, issue);
}
/** Checked exported targets are canonical: // is the exact standard root tag,
 * not a physical-directory/name heuristic. Declared data activation bypasses this
 * semantic-edge policy in the shared controller.
 */
static bool follow(void *context, const DeliverySourceDefinition *owner, const DeliverySourceDependency *edge) {
    (void)owner; SourceCopyClosure *copy = context;
    return copy->mode == SOURCE_COPY_CLONE_DEPENDENCIES && strncmp(edge->target, "//", 2) != 0;
}
AcquisitionClosureProvider source_copy_closure_callbacks(SourceCopyClosure *copy) {
    return (AcquisitionClosureProvider){.context = copy, .expand = expand, .admit_test = admit, .follow_dependency = follow};
}
