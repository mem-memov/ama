/** @file
 * @brief Encode complete specialization owner maps for exact generated-name registration.
 * [NAME-01](../../../../../spec/implementation/compiler.md#name-01) separates categories and roots.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) retains failed destination storage.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) separates keys from admission.
 * [TPL-07](../../../../../spec/language/templates.md#tpl-07) keeps ordinary constructor formals Type-valued.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) retains every original binding.
 * [NAME-02](../../../../../spec/implementation/compiler.md#name-02) requires full keys beside hashes.
 */
#ifndef AMA_STATIC_REGISTRY_KEY_H
#define AMA_STATIC_REGISTRY_KEY_H
#include "../../terms/store.h"
#include "source/identity/identity.h"

/** Encoding refuses open/foreign maps and checked size/allocation failures.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial preparation.
 */
typedef enum { STATIC_KEY_OK, STATIC_KEY_INVALID, STATIC_KEY_OUT_OF_MEMORY, STATIC_KEY_LIMIT, STATIC_KEY_COLLISION } StaticKeyStatus;

/** Descriptor/member storage belongs to the supplied arena; original module
 * path/span borrow the inventory. The descriptor includes every reachable
 * nominal identity once, using deterministic postorder references.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) preserves full metadata.
 */
typedef struct {
    const char *descriptor;
    size_t length;
    QualifiedIdentity qualified;
} StaticKeyEncoding;

/** Encode an already solver-normalized, published applied environment. Ground
 * representation alone is not a solver/admission proof: callers must normalize
 * through the guard before this operation. Every Type constructor capture is
 * Type-valued; producer inputs may additionally contain closed Templates.
 * Destination arena must be distinct from the term store arena.
 * Scratch is released on every return; partial destination allocations remain
 * arena-owned. Output changes only on success. No names, scopes or instances
 * are registered, and the borrowed term store is unchanged.
 * [NAME-01](../../../../../spec/implementation/compiler.md#name-01) separates roots/categories.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) keeps registration distinct from admission.
 */
StaticKeyStatus static_key_encode(const StaticTermStore *, const StaticTermEnvironment *,
    SourceArena *, StaticKeyEncoding *);
/** Compose full-key encoding and existing SHA-256 name registration. Temporary
 * descriptors expire before return; successful names and partial name-encoding
 * allocations belong to names. Its conflict pointer is cleared for each attempt
 * and identifies the retained unequal key on collision. Output changes only on
 * success. Source/terms remain borrowed and no execution is admitted.
 * [NAME-02](../../../../../spec/implementation/compiler.md#name-02) compares complete keys, never digest alone.
 */
StaticKeyStatus static_key_register(const StaticTermStore *, const StaticTermEnvironment *,
    IdentityRegistry *, const IdentityName **);
#endif
