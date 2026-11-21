/**
 * @file
 * @brief Separate strict INI encoding checks, scalar spacing and line publication.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) requires exact original
 * byte checks; none of these operations interprets values or resolves a path.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves the original failure interval when scanning closes.
 */
#ifndef AMA_SOURCE_CONFIGURATION_LINES_INTERNAL_H
#define AMA_SOURCE_CONFIGURATION_LINES_INTERNAL_H
#include <stdint.h>
#include "lines.h"

/** Classify pinned Unicode 17 White_Space without host locale dependencies.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) requires boundary whitespace checks independent of the host locale.
 */
bool configuration_space(uint32_t scalar);
/** Record one bounded original byte site, close the cursor and return false.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) requires the original byte site even after scanning stops.
 */
bool configuration_line_failure(ConfigurationLines *lines, size_t offset,
    const char *message, const char *expected);
#endif
