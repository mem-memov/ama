/**
 * @file
 * @brief Validate canonical logical module identities from bounded source byte views.
 *
 * [SRC-01](../../../spec/language/source-and-names.md#src-01) separates ordinary
 * versioned package identities from standard-library identities. This interface
 * validates already canonical prefixes/endpoints; normalization and filesystem
 * lookup remain separate consumers. [PKG-01](../../../spec/project/packages.md#pkg-01)
 * supplies explicit version and source-area boundaries without probe modules.
 * [SRC-03](../../../spec/language/source-and-names.md#src-03) keeps complete
 * original component names independent of declarations and locale.
 */
#ifndef AMA_SOURCE_PATH_LOGICAL_H
#define AMA_SOURCE_PATH_LOGICAL_H

#include <stdbool.h>
#include <stddef.h>
#include "root.h"

/** How far a valid canonical prefix has reached; no missing component is invented.
 * [SRC-01](../../../spec/language/source-and-names.md#src-01) distinguishes a
 * complete module from vendor/package/version/area namespace prefixes.
 */
typedef enum {
    MODULE_PATH_NAMESPACE,
    MODULE_PATH_VERSION,
    MODULE_PATH_AREA,
    MODULE_PATH_MODULE
} ModulePathExtent;

/** Structural source area, independent of similarly spelled ordinary module names.
 * [PKG-01](../../../spec/project/packages.md#pkg-01) reserves these positions
 * after the version; standard paths have no package area.
 */
typedef enum {
    MODULE_PATH_NO_AREA,
    MODULE_PATH_SRC,
    MODULE_PATH_TESTS
} ModulePathArea;

/** Numeric description of one validated bounded canonical spelling.
 * version_length is the original exclusive byte offset after the version;
 * version_components counts through that component. Both are zero without a
 * version. No pointers are retained. [PKG-01](../../../spec/project/packages.md#pkg-01)
 * gives one exact version boundary, never a guessed substring or host integer.
 */
typedef struct {
    ModuleRootKind root;
    ModulePathExtent extent;
    ModulePathArea area;
    size_t components;
    size_t version_components;
    size_t version_length;
} ModulePathShape;

/** Describe a complete canonical module or extendable canonical namespace prefix.
 * Root-only / and // are valid prefixes. Empty/trailing/repeated components,
 * dot/parent/alias spellings, malformed names and misplaced versions refuse.
 * Output changes only on success; NULL output refuses. Input is borrowed only for this call.
 * [SRC-01](../../../spec/language/source-and-names.md#src-01) determines the
 * endpoint shape; [PKG-01](../../../spec/project/packages.md#pkg-01) identifies
 * version/area positions. O(length) time and O(1) auxiliary storage.
 */
bool module_path_describe(const unsigned char *bytes, size_t length, ModulePathShape *output);

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
