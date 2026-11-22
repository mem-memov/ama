/** @file
 * @brief Separate active copy units, version demands and final logical naming.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * gives explicit, additional and associated-test placements distinct authorities.
 */
#ifndef AMA_PROJECT_SOURCE_COPY_DESTINATIONS_INTERNAL_H
#define AMA_PROJECT_SOURCE_COPY_DESTINATIONS_INTERNAL_H
#include "destinations.h"
#include "../../mapping/tests.h"
/** Private construction arrays remain arena-owned after failure. Association
 * names exist only for actual checked active src definitions, never draft bodies.
 */
typedef struct {
    SourceCopyDestination *entries;
    SourceVersionUnit *units;
    const char **associations;
    SourceVersionFlow *flows;
    size_t flow_count;
    const SourceVersionAssignment *assignments;
    SourceTestDirectoryMap *tests;
    size_t test_count;
} SourceCopyDestinationWork;
/** Validate explicit selection coverage and collect active ordinary originals. */
SourceCopyDestinationsStatus source_copy_destination_units(SourceCopyDestinations *, SourceCopyDestinationWork *);
/** Construct finite exact requester demands, respecting deepest test ownership. */
SourceCopyDestinationsStatus source_copy_destination_flows(SourceCopyDestinations *, SourceCopyDestinationWork *);
/** Name source copies first, then project associated tests and outside helpers. */
SourceCopyDestinationsStatus source_copy_destination_names(SourceCopyDestinations *, SourceCopyDestinationWork *);
/** Find an explicit proposal by exact original handle; NULL means additional. */
const SourceCloneMapping *source_copy_destination_explicit(const SourceCopyDestinations *, const ModuleSource *);
#endif
