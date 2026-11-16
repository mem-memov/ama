/** @file
 * @brief Apply module-prefix edits once while copying all other source bytes exactly.
 * [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * preserves comments, literals, layout, drafts and optional final newlines.
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../internal.h"

/** Original source order determines output order, independently of graph traversal or root order. */
static int compare_edits(const void *left, const void *right) {
    const RelocationPathEdit *a = *(const RelocationPathEdit *const *)left;
    const RelocationPathEdit *b = *(const RelocationPathEdit *const *)right;
    return a->original.start < b->original.start ? -1 : a->original.start > b->original.start;
}
/** Select only edits backed by this exact captured physical source and unchanged original bytes. */
static bool select_edits(RelocationStaging *staging, RelocationStagedSource *source) {
    size_t count = 0;
    for (const RelocationPathEdit *edit = staging->references->edits; edit != NULL; edit = edit->next) {
        if (strcmp(source_path(edit->original.source),source->module->before_path) == 0) { count++; }
    }
    if (count > SIZE_MAX/sizeof(*source->edits)) {
        return relocation_staging_fail(staging,RELOCATION_OUT_OF_MEMORY,"Too many source replacements",(SourceSpan){0});
    }
    source->edits = source_arena_allocate(&staging->arena,count*sizeof(*source->edits));
    if (source->edits == NULL) {
        return relocation_staging_fail(staging,RELOCATION_OUT_OF_MEMORY,"Source replacement allocation failed",(SourceSpan){0});
    }
    for (const RelocationPathEdit *edit = staging->references->edits; edit != NULL; edit = edit->next) {
        if (strcmp(source_path(edit->original.source),source->module->before_path) != 0) { continue; }
        if (!source->module->active || !source_span_valid(edit->original)
            || edit->original.start == edit->original.end
            || source_length(edit->original.source) != source_length(source->module->text)
            || memcmp(source_bytes(edit->original.source),source_bytes(source->module->text),source_length(source->module->text)) != 0) {
            return relocation_staging_fail(staging,RELOCATION_REFERENCE_CONFLICT,"A replacement has no exact active source snapshot",edit->original);
        }
        source->edits[source->edit_count++] = edit;
    }
    qsort(source->edits,source->edit_count,sizeof(*source->edits),compare_edits);
    return true;
}
/** Link only initialized records, retaining each failed prefix for one uniform teardown path. */
bool relocation_staging_texts(RelocationStaging *staging) {
    for (const RelocationModule *module = staging->references->preview->modules; module != NULL; module = module->next) {
        RelocationStagedSource *source = source_arena_allocate(&staging->arena,sizeof(*source));
        if (source == NULL) {
            return relocation_staging_fail(staging,RELOCATION_OUT_OF_MEMORY,"Prepared source record allocation failed",(SourceSpan){0});
        }
        *source = (RelocationStagedSource){.module=module};
        if (staging->last == NULL) { staging->first = source; }
        else { staging->last->next = source; }
        staging->last = source;
        if (!select_edits(staging,source) || !relocation_staging_rewrite(staging,module->text,module->after_path,source->edits,source->edit_count,&source->text)) { return false; }
    }
    return true;
}
