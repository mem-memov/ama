/**
 * @file
 * @brief Declare scoped generated-output ownership and manifest integrity records.
 *
 * [COMP-04](../../../../../spec/implementation/compiler.md#comp-04)
 * Authored files are not inferred as generated from their basename; owned output requires explicit scoped metadata.
 */
#ifndef AMA_SCAFFOLD_OWNERSHIP_H
#define AMA_SCAFFOLD_OWNERSHIP_H

#include "native/scaffold/internal.h"

/** Validated scoped manifest name/digest record owned by scaffold arena; it grants authority only after current-content verification. */
typedef struct ScaffoldOwned {
    struct ScaffoldOwned *next;
    const char *name;
    unsigned char digest[32];
} ScaffoldOwned;

/** Hash the exact absolute selected source path for this explicit output scope; output supplies 32 writable bytes.
 */
void scaffold_scope(const NativeScaffold *, unsigned char digest[32]);
/** Check SHA input length before hashing complete immutable artifact bytes; return false with first diagnostic on unrepresentable length.
 */
bool scaffold_digest(NativeScaffold *, const SourceText *, unsigned char digest[32]);
/** Read/validate one regular scoped manifest or accept its absence. Exact records/paths/digests stay scaffold-owned; malformed scratch is never publication authority.
 */
bool scaffold_manifest_read(NativeScaffold *);
/** Borrow the component after the last slash, or the complete path, without modifying/canonicalizing caller storage.
 */
const char *scaffold_basename(const char *path);

#endif
