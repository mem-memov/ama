/** @file
 * @brief Check exact original/prepared occurrence sites after binding-law admission.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves each resolved occurrence through parsed-span edits;
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires final checked captured copies, independently of physical publication.
 */
#ifndef AMA_PROJECT_SOURCE_GRAPH_SITES_H
#define AMA_PROJECT_SOURCE_GRAPH_SITES_H
#include "bindings.h"
#include "../edits/edits.h"
/** Borrow one complete edit result whose original is the exact original-export
 * occurrence source handle, not merely another equal-byte capture. One row per
 * original handle is allowed; unchanged copied text still has an applied no-edit
 * result with the explicit final physical source path.
 */
typedef struct SourceGraphPreparedText {
    struct SourceGraphPreparedText *next;
    const SourceTextEdits *edits;
} SourceGraphPreparedText;
/** Binding, missing provenance and incorrect final sites are distinct refusals. */
typedef enum { SOURCE_SITES_OK, SOURCE_SITES_INVALID, SOURCE_SITES_BINDINGS,
    SOURCE_SITES_MISSING, SOURCE_SITES_CHANGED } SourceSitesStatus;
/** First borrowed exact witness; binding details also describe the owning definition. */
typedef struct {
    SourceBindingsStatus binding_status;
    SourceBindingsIssue binding;
    const SourceGraphPreparedText *prepared;
    const DeliverySourceDependency *original_edge, *final_edge;
} SourceSitesIssue;
/** First check complete binding projection with independent physical-removal
 * and selected-registry retirement facts, then require unchanged path/offset
 * sites for surviving originals and exact edit-projected path/offset sites for
 * copies. Match immutable export source handles exactly; compare final projected
 * paths/offsets rather than source pointers because the admitted final response
 * deep-owns its capture. Copied occurrences require explicit prepared provenance;
 * never infer shifted offsets by searching text. Duplicate/unready preparation
 * rows refuse. No allocation or I/O. Output issue is reset then retains the first
 * failure. Source/result bytes and final package/topology/transaction authority
 * remain separate obligations; final client admission must match prepared bytes.
 * Beyond binding checking, O(P squared + F*(N+C) + E*(P+A)) work and O(1)
 * scratch, where P is prepared texts and A bounds edits per text.
 */
SourceSitesStatus source_graph_sites(const DeliverySourceDefinition *original,
    const DeliverySourceDefinition *final, const SourceCloneMap *,
    const char *const *removed, size_t removed_count,
    SourceRegistryRetirement, const SourceGraphPreparedText *, SourceSitesIssue *);
#endif
