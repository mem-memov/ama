/** @file
 * @brief Separate final physical copy laws from their semantic-proof orchestration.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires both material and nominal preservation; neither replaces the other.
 */
#ifndef AMA_SOURCE_COPY_PROOF_INTERNAL_H
#define AMA_SOURCE_COPY_PROOF_INTERNAL_H
#include "proof.h"
#include "../../capture/preservation.h"
/** Compare finite prepared P observations with actual final image rows; no allocation. */
bool source_copy_proof_material(const SourceCopyMaterial *, const SourceCatalogImage *, const char **);
/** Permit main-file edits only inside authored module operands; final site proof
 * must still match the entire projected operand, preventing inserted code escape.
 * With T text owners, C clones, D definitions, E edits and H maximum operand
 * edges per definition, use O(T(C+D) + EH) comparisons and O(1) scratch. */
bool source_copy_proof_edits(const DeliverySourceDefinition *, const SourceCloneMap *,
    const SourceGraphPreparedText *, SourceCopyProofIssue *);
/** Prove retained originals, C placement, complete companion membership and final
 * main accounting over validated immutable inputs; no allocation or publication. */
SourceCopyProofStatus source_copy_proof_units(const SourceCloneMap *, const SourceFileMap *,
    const SourceImageCapture *, const char *const *, size_t, SourceRegistryRetirement, const SourceCopyIntroduction *, SourceCopyProofIssue *);
#endif
