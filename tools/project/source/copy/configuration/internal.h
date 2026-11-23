/** @file
 * @brief Share the exact absence predicate for structural and permission initialization.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * gives both phases the same wholly-absent-version boundary.
 */
#ifndef AMA_SOURCE_CONFIGURATION_INITIALIZATION_INTERNAL_H
#define AMA_SOURCE_CONFIGURATION_INITIALIZATION_INTERNAL_H
#include "../../image/image.h"
/** Require a valid immutable image and normalized absolute proposed version key.
 * Existing or unknown keys refuse; only the closest complete directory ancestor
 * proves absence. No allocation/I/O; finite exact row/ancestor scans. */
bool source_configuration_version_absent(const SourceCatalogImage *, const char *);
/** Borrow an observed final root mode, or closest containing physical boundary
 * mode for a wholly missing root. Unknown/non-directory rows refuse. */
const SourceCatalogImageEntry *source_configuration_root_mode(const SourceCatalogImage *, const ModuleRoot *);
#endif
