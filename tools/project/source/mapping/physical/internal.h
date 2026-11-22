/** @file
 * @brief Validate a complete final-version main index before plus-topology projection.
 * [SRC-05](../../../../../spec/language/source-and-names.md#src-05) identifies
 * physical module ancestors by exact canonical identity, never by basename.
 */
#ifndef AMA_PROJECT_SOURCE_MAIN_PLACEMENT_INTERNAL_H
#define AMA_PROJECT_SOURCE_MAIN_PLACEMENT_INTERNAL_H
#include "main.h"
#include <stdbool.h>
/** Return false for malformed/mixed-version/duplicate inputs; no allocation or
 * caller output mutation. The selected version byte boundary is returned on
 * success, and physical normalization remains the explicit caller precondition.
 */
bool source_main_placement_valid(const SourceVersionPlacement *, const char *const *, size_t,
    const char *, size_t *version_length);
#endif
