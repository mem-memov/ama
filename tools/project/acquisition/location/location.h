/** @file
 * @brief Map one selected route to a contained checkout-relative version directory.
 * [PKG-06](../../../../spec/project/packages.md#pkg-06) defines the physical
 * version directory as part of coherent source identity. Filesystem/Git-tree
 * containment remains an explicit transport obligation under
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01).
 */
#ifndef AMA_PROJECT_ACQUISITION_LOCATION_H
#define AMA_PROJECT_ACQUISITION_LOCATION_H
#include "../routing/route.h"
/** Checked size/allocation refusal is distinct from invalid metadata. */
typedef enum { ACQUISITION_MAP_OK, ACQUISITION_MAP_INVALID,
    ACQUISITION_MAP_LIMIT, ACQUISITION_MAP_OUT_OF_MEMORY } AcquisitionMapStatus;
/** Immutable arena-owned mapping, borrowing the selected route's owners.
 * version is the canonical logical version identity, with no src/tests suffix.
 * directory is a nonempty normalized relative physical directory, with no
 * empty/dot/parent components. It does not assert filesystem availability. */
typedef struct {
    AcquisitionRoute route;
    const char *version;
    const char *directory;
    size_t version_length;
    size_t directory_length;
} AcquisitionLocation;
/** Map a successful route without I/O or rerouting. Output changes only on OK;
 * failed scratch stays arena-owned. Source/normalized path/configuration owners
 * outlive the returned record. A later transport must inspect the selected
 * snapshot and reject actual aliases/escapes before reading files. */
AcquisitionMapStatus acquisition_location(SourceArena *, const AcquisitionRoute *,
    const AcquisitionLocation **output, SourceIssue *issue);
#endif
