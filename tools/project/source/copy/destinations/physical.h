/** @file
 * @brief Join admitted ordinary logical copy candidates with exact final main topology.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires agreement between explicit P and C; [SRC-05](../../../../../spec/language/source-and-names.md#src-05)
 * derives plus storage from final module ancestors.
 */
#ifndef AMA_SOURCE_COPY_PHYSICAL_MAINS_H
#define AMA_SOURCE_COPY_PHYSICAL_MAINS_H
#include "destinations.h"
#include "../../mapping/physical/main.h"
/** Preserve topology, explicit-placement and injective-relation refusal stages. */
typedef enum { SOURCE_COPY_MAINS_OK, SOURCE_COPY_MAINS_INVALID,
    SOURCE_COPY_MAINS_TOPOLOGY, SOURCE_COPY_MAINS_EXPLICIT,
    SOURCE_COPY_MAINS_MEMORY, SOURCE_COPY_MAINS_MAP } SourceCopyMainsStatus;
/** Borrow offending original/explicit witnesses; expected path belongs to map arena. */
typedef struct {
    const ModuleSource *original;
    const SourceCloneMapping *explicit_row;
    const char *expected;
    SourceMainPathStatus path_status;
} SourceCopyMainsIssue;
/** Add explicit and active ordinary candidates belonging to one destination
 * version into a distinct open clone map over the same original capture. Derive
 * each path from the complete supplied final same-version main index, including
 * surviving/context/draft mains. Explicit placements must agree byte for byte.
 * Other destination versions await their own call. Existing map rows are joined
 * through its normal injectivity rules. No sealing, I/O or context/opaque mapping.
 * Caller owns index completeness, designated-root selection and normalized physical
 * keys. Failure keeps owned scratch/visible prefix but poisons sealing. Refuse
 * sealed/failed maps, borrowed input-map aliasing and missing issue unchanged.
 * Borrow inputs through inspection. Complexity is
 * the sum of exact topology construction and map insertion for the finite rows;
 * scratch paths stay map-arena-owned. An unused version adds no rows and grants
 * no topology authority. This stage alone proves no final catalog/operation safety.
 */
SourceCopyMainsStatus source_copy_place_version_mains(SourceCloneMap *, const SourceCopyDestinations *,
    const SourceVersionPlacement *, const char *const *, size_t, SourceCopyMainsIssue *);
#endif
