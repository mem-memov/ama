/** @file
 * @brief Deduplicate identical original replacements and reject any conflicting overlap before staged rewriting.
 * [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * preserves every byte outside parsed module-prefix spans, including constant member names and suffixes.
 */
#include <string.h>
#include "../internal.h"

/** Half-open bounds make overlap checks overflow-free; repeated structural aliases may request the same exact edit. Component contract: [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03). */
bool relocation_reference_edit(RelocationReferences *references, SourceSpan site, const RelocationModule *target) {
    for (const RelocationPathEdit *other = references->edits; other != NULL; other = other->next) {
        if (other->original.source != site.source || other->original.start >= site.end || site.start >= other->original.end) { continue; }
        if (other->original.start == site.start && other->original.end == site.end
            && strcmp(other->replacement,target->after_identity) == 0) { return true; }
        relocation_references_fail(references,RELOCATION_REFERENCE_CONFLICT,"Original module-path edits overlap",site);
        references->related = (DiagnosticRelated){.location={other->original,NULL},.role="conflicting path replacement"};
        references->error.related = &references->related;
        references->error.related_count = 1;
        return false;
    }
    RelocationPathEdit *edit = source_arena_allocate(&references->arena,sizeof(*edit));
    if (edit == NULL) { return relocation_references_fail(references,RELOCATION_OUT_OF_MEMORY,"Path edit allocation failed",site); }
    *edit = (RelocationPathEdit){.original=site,.replacement=target->after_identity,.target=target};
    if (references->last_edit == NULL) { references->edits = edit; }
    else { references->last_edit->next = edit; }
    references->last_edit = edit;
    return true;
}
