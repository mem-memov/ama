/**
 * @file
 * @brief Validate canonical logical module identities from bounded source byte views.
 *
 * [SRC-01](../../../spec/language/source-and-names.md#src-01) separates ordinary
 * versioned package identities from standard-library identities. This interface
 * validates an already canonical endpoint; normalization and filesystem lookup
 * are separate consumers under SRC-06 and SRC-02.
 */
#ifndef AMA_SOURCE_PATH_LOGICAL_H
#define AMA_SOURCE_PATH_LOGICAL_H

#include <stdbool.h>
#include <stddef.h>

/** Validate one complete canonical absolute module identity without allocation.
 * @param[in] bytes Borrowed bounded spelling; NULL is invalid and never read.
 * @param[in] length Exact readable length, excluding any C terminator.
 * @return True exactly for a complete ordinary vendor/package/version/src-or-tests
 * module identity or a nonempty standard-library module identity. Relative,
 * aliased, padded, empty or unnormalized components fail; bytes stay unchanged.
 * [PKG-01](../../../spec/project/packages.md#pkg-01) determines the unique version
 * boundary, and [SRC-03](../../../spec/language/source-and-names.md#src-03) applies
 * to all ordinary names. Time is O(length), auxiliary space O(1).
 */
bool module_path_canonical(const unsigned char *bytes, size_t length);

#endif
