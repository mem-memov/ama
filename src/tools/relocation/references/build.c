/** @file
 * @brief Build one complete original binding graph before selecting any rewritten module bytes.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves exact resolved identities, including cycles and external importers.
 */
#include "internal.h"
/** A prepared preview fixes physical identities; source/semantic validity must also succeed before edits are consumable. Component contract: [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02). */
bool relocation_references_build(RelocationReferences *references, const RelocationPreview *preview) {
    if (references->attempted) { return false; }
    references->attempted = true;
    if (preview == NULL || !preview->ready) {
        return relocation_references_fail(references,RELOCATION_INVALID_PATH,"A complete relocation preview is required",(SourceSpan){0});
    }
    references->preview = preview;
    if (!relocation_references_domain(references) || !relocation_references_graph(references)) { return false; }
    references->ready = true;
    return true;
}
