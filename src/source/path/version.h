/**
 * @file
 * @brief Recognize one exact bounded package-version component without integer conversion.
 *
 * [PKG-01](../../../spec/project/packages.md#pkg-01) makes a complete version
 * component the unambiguous package boundary. Its unbounded decimal strings
 * must not depend on the host integer range or a locale's character classes.
 */
#ifndef AMA_SOURCE_PATH_VERSION_H
#define AMA_SOURCE_PATH_VERSION_H

#include <stdbool.h>
#include <stddef.h>

/** Recognize the complete vMAJOR.MINOR.PATCH[-VARIANT] byte view.
 * @param[in] bytes Borrowed bounded bytes, never modified or retained; NULL fails.
 * @param[in] length Exact readable length; no terminator is required or consumed.
 * @return True exactly for the version production, including a nonempty Unicode
 * name-word variant when present. Empty, malformed UTF-8 and embedded NUL fail.
 * This allocation-free predicate has no side effects and runs in O(length) time
 * and O(1) auxiliary space. [PKG-01](../../../spec/project/packages.md#pkg-01)
 * requires lexical decimal recognition rather than conversion or normalization.
 */
bool module_version_component(const unsigned char *bytes, size_t length);

#endif
