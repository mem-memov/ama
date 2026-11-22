/** @file
 * @brief Admit exact final compiler inputs before proving surviving removal graph and sites.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves original binding/site identities; [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires final semantic evidence to describe the actual prepared input set.
 */
#include "image.h"
SourceRemovalProofStatus source_removal_image_prove(const SourceRemovalImage *image,
    const SourceCompilerClient *client, SourceSitesIssue *issue) {
    if (!issue) { return SOURCE_REMOVAL_PROOF_INVALID; }
    *issue = (SourceSitesIssue){0};
    if (!image || !image->valid || !client || !client->valid) {
        return SOURCE_REMOVAL_PROOF_INVALID;
    }
    if (!delivery_source_response_matches(&client->response, &image->capture.view)) { return SOURCE_REMOVAL_PROOF_INPUTS; }
    SourceCloneMap map;
    source_clone_map_init(&map, image->arena.allocator, image->selection->tree->capture);
    source_clone_map_seal(&map);
    SourceSitesStatus status = source_graph_sites(image->selection->analysis->original.response.definitions,
        client->response.definitions, &map, image->selection->identities, image->selection->identity_count, NULL, issue);
    source_clone_map_destroy(&map);
    return status == SOURCE_SITES_OK ? SOURCE_REMOVAL_PROOF_OK : SOURCE_REMOVAL_PROOF_SITES;
}
