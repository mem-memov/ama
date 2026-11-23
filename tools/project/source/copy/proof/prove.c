/** @file
 * @brief Prove exact final physical membership before context and graph-site admission.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * protects originals even without graph edges; [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires the compiler response to match the actual prepared input set.
 */
#include "internal.h"
SourceCopyProofStatus source_copy_prove(SourceArena *arena, const SourceAnalysis *original,
    const SourceCloneMap *map, const SourceFileMap *files, const SourceGraphPreparedText *prepared,
    const SourceCopyContextOmissions *contexts, const SourceImageCapture *final, const SourceCompilerClient *client,
    const char *const *removed, size_t count, SourceCopyProofIssue *issue) {
    if (!issue) { return SOURCE_COPY_PROOF_INVALID; } *issue = (SourceCopyProofIssue){0};
    if (!arena || !original || !original->valid || !map || !map->ready || !map->capture || !map->capture->valid
        || !files || !files->ready || !contexts || !contexts->ready || contexts->mains != map
        || !contexts->copy || contexts->copy->expansion.analysis != original || contexts->copy->tree->capture != map->capture
        || !final || !final->valid || !client || !client->valid || (count && !removed)) { return SOURCE_COPY_PROOF_INVALID; }
    for (size_t i = 0; i < count; i++) { if (!removed[i]) { return SOURCE_COPY_PROOF_INVALID; } }
    if (!delivery_source_response_matches(&original->original.response, &map->capture->view)) { return SOURCE_COPY_PROOF_INVALID; }
    if (!delivery_source_response_matches(&client->response, &final->view)) { return SOURCE_COPY_PROOF_INPUTS; }
    if (source_mapping_correspondence(map, files, &issue->correspondence) != SOURCE_CORRESPONDENCE_OK) { return SOURCE_COPY_PROOF_CORRESPONDENCE; }
    SourceCopyMaterial material;
    issue->material_status = source_copy_material_prepare(arena, files, prepared, &material, &issue->material);
    if (issue->material_status != SOURCE_MATERIAL_OK || !source_copy_proof_material(&material, final->image, &issue->path)) { return SOURCE_COPY_PROOF_MATERIAL; }
    SourceCopyProofStatus units = source_copy_proof_units(map, files, final, removed, count, issue);
    if (units != SOURCE_COPY_PROOF_OK) { return units; }
    if (source_copy_context_omissions_check(contexts, final, removed, count, &issue->context) != SOURCE_COPY_CONTEXT_OK) { return SOURCE_COPY_PROOF_CONTEXT; }
    if (source_graph_sites(original->original.response.definitions, client->response.definitions, map, removed, count, prepared, &issue->sites) != SOURCE_SITES_OK) { return SOURCE_COPY_PROOF_SITES; }
    *issue = (SourceCopyProofIssue){0}; return SOURCE_COPY_PROOF_OK;
}
