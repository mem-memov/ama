/**
 * @file
 * @brief Separate exact identity-size proof from versioned binary component serialization.
 *
 * [NAME-01](../../../../spec/implementation/compiler.md#name-01) includes category,
 * root kind and complete original components. [NAME-02](../../../../spec/implementation/compiler.md#name-02)
 * hashes exactly this complete key while collision checks retain the original bytes.
 */
#ifndef AMA_IDENTITY_ENCODING_INTERNAL_H
#define AMA_IDENTITY_ENCODING_INTERNAL_H
#include "../internal.h"
#include "source/module/catalog.h"

#define IDENTITY_ENCODING_FIXED_BYTES 26

/** Checked binary shape. Fixed bytes comprise magic/version, category/root and
 * two 64-bit component counts; each component adds one 64-bit length and its bytes.
 * root_width counts only the source root marker, never an empty path component.
 */
typedef struct {
    size_t length;
    size_t display_length;
    size_t module_count;
    size_t root_width;
    ModuleRootKind root_kind;
} IdentityEncodingShape;

/** Add one 64-bit length and its byte count within both size_t and SHA bit limits.
 * False preserves total; valid writable scalar ownership is supplied by the caller.
 * This pure arithmetic boundary permits complete-domain limit tests without huge arrays.
 */
bool identity_encoding_part_size(size_t *, size_t);

/** Validate/measure borrowed source metadata without allocation. Output changes only on success;
 * the complete encoded length fits both size_t and the SHA-256 64-bit bit-count domain.
 */
IdentityStatus identity_encoding_measure(QualifiedIdentity, IdentityEncodingShape *);
/** Allocate/fill the exact original display string once; return arena-owned text or NULL.
 * Valid premeasured shape/input stay immutable through the synchronous copy.
 */
const char *identity_encoding_display(SourceArena *, QualifiedIdentity, const IdentityEncodingShape *);

/** Serialize counted module/member parts into premeasured caller-owned capacity.
 * Shape and input are already validated and immutable; no allocation or failure occurs.
 */
void identity_encoding_parts(unsigned char *, QualifiedIdentity, const IdentityEncodingShape *);
#endif
