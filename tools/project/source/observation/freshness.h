/** @file
 * @brief Compare the complete original observations needed by a prepared source operation.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * revalidates identities, original bytes and directory membership, including
 * newly added ignored names that do not change compiler reachability.
 */
#ifndef AMA_SOURCE_OBSERVATION_FRESHNESS_H
#define AMA_SOURCE_OBSERVATION_FRESHNESS_H
#include "../image/image.h"
/** Preserve root-domain, membership, observation-precision, identity and byte failures. */
typedef enum { SOURCE_FRESH_OK, SOURCE_FRESH_INVALID, SOURCE_FRESH_ROOTS,
    SOURCE_FRESH_MEMBERSHIP, SOURCE_FRESH_OBSERVATION,
    SOURCE_FRESH_IDENTITY, SOURCE_FRESH_CONTENT } SourceFreshnessStatus;
/** Borrow original/fresh rows and the offending key through caller consumption. */
typedef struct {
    const char *path;
    const SourceCatalogImageEntry *original, *fresh;
} SourceFreshnessIssue;
/** Compare two valid immutable ORIGINAL observation images over the same ordered
 * configured roots/exclusions and exact observation demands. Require identical
 * physical key sets, observation precision, known device/inode/mode and observed
 * content bytes. Complete directory rows plus identical key sets prove unchanged
 * immediate membership; an unobserved ignored entry remains unobserved, never
 * silently interpreted as absent or fetched here. Different text handles and row
 * order are allowed. Modification/access timestamps and other unused metadata do
 * not define identity or replace byte comparison.
 *
 * No allocation, I/O or mutation. Reset issue, then retain the first witness.
 * O(N squared + root/exclusion comparisons + observed bytes), O(1) scratch.
 * Caller rebuilds fresh observations with the same demands under exclusive project
 * access and retains that exclusion through publication. Comparing prepared final
 * images, comparing only a compiler graph, or releasing the exclusion before
 * publication does not establish TOOL-04 freshness or transaction safety.
 */
SourceFreshnessStatus source_observation_freshness(const SourceCatalogImage *,
    const SourceCatalogImage *, SourceFreshnessIssue *);
#endif
