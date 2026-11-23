/** @file
 * @brief Share observed final-root mode evidence for structural initialization.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * requires initialized directory modes to come from observed physical authority.
 */
#ifndef AMA_SOURCE_CONFIGURATION_INITIALIZATION_INTERNAL_H
#define AMA_SOURCE_CONFIGURATION_INITIALIZATION_INTERNAL_H
#include "../../image/image.h"
/** Borrow an observed final root mode, or closest containing physical boundary
 * mode for a wholly missing root. Unknown/non-directory rows refuse. */
const SourceCatalogImageEntry *source_configuration_root_mode(const SourceCatalogImage *, const ModuleRoot *);
#endif
