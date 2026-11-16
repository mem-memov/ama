/** @file
 * @brief Prove the final resolved graph is exactly the original graph under the planned identity map.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves every original binding; [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires an independent final check before publication.
 */
#include <string.h>
#include "../internal.h"

/** Compare one edge's target, physical source, implicit kind and translated original requiring boundaries. Component contract: [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02), [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static bool same_edge(const RelocationStagedSource *source, const RelocationReference *original, const ProjectDependency *final) {
    size_t start, end;
    return final != NULL && source_span_valid(final->requirement)
        && strcmp(source_path(final->requirement.source),source_path(source->text)) == 0
        && relocation_staging_offset(source->module->text,source->edits,source->edit_count,original->site.start,&start)
        && relocation_staging_offset(source->module->text,source->edits,source->edit_count,original->site.end,&end)
        && final->requirement.start == start && final->requirement.end == end
        && final->implicit == original->implicit
        && strcmp(final->target->source->logical_path,original->target->after_identity) == 0
        && strcmp(source_path(final->target->source->text),original->target->after_path) == 0;
}
/** Stable source-position ordering preserves multiplicities, including duplicate paths and equal spans. Component contract: [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02), [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static bool module_edges(RelocationStaging *staging, const RelocationStagedSource *source) {
    const ProjectModule *module = project_find(&staging->project,source->module->after_identity);
    if (module == NULL || strcmp(source_path(module->source->text),source_path(source->text)) != 0) {
        return relocation_staging_fail(staging,RELOCATION_INVALID_REFERENCE,"A prepared module has no exact final owner",(SourceSpan){0});
    }
    const ProjectDependency *final = module->first;
    for (const RelocationReference *edge = staging->references->first; edge != NULL; edge = edge->next) {
        if (edge->owner != source->module) { continue; }
        if (!same_edge(source,edge,final)) {
            return relocation_staging_fail(staging,RELOCATION_INVALID_REFERENCE,"The final reference graph does not preserve an original binding",edge->site);
        }
        final = final->next;
    }
    if (final != NULL) {
        return relocation_staging_fail(staging,RELOCATION_INVALID_REFERENCE,"The prepared module introduces an unexpected reference",final->requirement);
    }
    return true;
}
/** The required set is exactly the active preview domain; edge equality proves no binding was lost, added or redirected. Component contract: [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02), [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_staging_graph(RelocationStaging *staging) {
    size_t expected = 0, actual = 0;
    for (const RelocationStagedSource *source = staging->first; source != NULL; source = source->next) {
        if (source->module->active) {
            expected++;
            if (!module_edges(staging,source)) { return false; }
        }
    }
    for (const ProjectModule *module = staging->project.first; module != NULL; module = module->next) { actual++; }
    return expected == actual || relocation_staging_fail(staging,RELOCATION_INVALID_REFERENCE,
        "The prepared project changes the active module domain",(SourceSpan){0});
}
