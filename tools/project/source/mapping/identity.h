/** @file
 * @brief Decode candidate logical names from explicit prepared physical placements.
 * [SRC-01](../../../../spec/language/source-and-names.md#src-01) fixes root kinds;
 * [PKG-01](../../../../spec/project/packages.md#pkg-01) fixes ordinary layout;
 * [SRC-05](../../../../spec/language/source-and-names.md#src-05) makes plus physical
 * storage only. Final common catalog discovery must still confirm every candidate.
 */
#ifndef AMA_PROJECT_SOURCE_PLACEMENT_IDENTITY_H
#define AMA_PROJECT_SOURCE_PLACEMENT_IDENTITY_H
#include "source/module/catalog.h"
/** Structural interpretation is distinct from final selected-source authority. */
typedef enum { SOURCE_IDENTITY_OK, SOURCE_IDENTITY_INVALID, SOURCE_IDENTITY_OUTSIDE,
    SOURCE_IDENTITY_NOT_MODULE, SOURCE_IDENTITY_AMBIGUOUS, SOURCE_IDENTITY_MEMORY } SourceIdentityStatus;
/** Arena-owned candidate name and exact explicit root index; no filesystem lease. */
typedef struct { const char *identity; size_t root; } SourcePlacedIdentity;
/** Interpret a previously normalized absolute physical main under explicit final
 * roots. Root-local exclusions apply; ordinary roots require packages/dependencies,
 * vendor/package/version and src/tests placement. Strip trailing plus only from
 * module-area directory components, never from package/version/area or a main's
 * basename. Validate with the shared canonical logical path predicate. Multiple
 * eligible roots refuse rather than select by input order. Output changes only on
 * unique success; failed scratch stays arena-owned. No I/O or parsing of Ama
 * bodies/configuration occurs. A candidate does not certify version precedence,
 * physical aliases, required plus anchors or destination existence.
 */
SourceIdentityStatus source_placement_identity(SourceArena *, const ModuleRoot *, size_t,
    const char *path, SourcePlacedIdentity *output);
#endif
