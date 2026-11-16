/** @file
 * @brief Declare exact raw-control copy phases over checked compiler schemas.
 * [CONT-04](../../../../../../spec/language/continuations.md#cont-04) requires
 * all destination lists before activating copied cursors. Borrowed storage/map
 * pointers live only through one guarded object-copy operation.
 */
#ifndef AMA_FOREACH_COPY_INTERNAL_H
#define AMA_FOREACH_COPY_INTERNAL_H
#include "copy.h"
/** Require the context-local nominal type and an identical world, not merely
 * an equal context-local handle triple from another context.
 * Component contract: [CONT-04](../../../../../../spec/language/continuations.md#cont-04). */
AmaStatus ama_foreach_copy_type(void *, AmaWorld *, uint64_t, const AmaProgramType **);
/** Validate list ordinal/kind before deriving a borrowed source/destination list. Component contract: [CONT-04](../../../../../../spec/language/continuations.md#cont-04). */
AmaStatus ama_foreach_copy_list(const AmaProgramType *, const void *, size_t, const AmaHandleList **);
/** Translate only a fully validated live occurrence by its current position.
 * Missing/obsolete origins become reserved-generation dead identities, never
 * positional guesses or freshly revived memberships.
 * Component contract: [CONT-04](../../../../../../spec/language/continuations.md#cont-04). */
AmaStatus ama_foreach_copy_membership(const AmaObjectControlCopy *, const AmaProgramType *,
    AmaMembership, AmaMembership *);
/** Validate active collection identity/incarnation without invoking a native hook. Component contract: [CONT-04](../../../../../../spec/language/continuations.md#cont-04). */
AmaStatus ama_foreach_control_validate(AmaWorld *, AmaHandle, const void *, const AmaServiceState *, void *);
/** Copy namespace/boundary/origin values after all destination list shapes exist. Component contract: [CONT-04](../../../../../../spec/language/continuations.md#cont-04). */
AmaStatus ama_foreach_control_copy(const AmaObjectControlCopy *, void *);
/** Remap only reference values in already copied cells, including forest siblings. Component contract: [CONT-04](../../../../../../spec/language/continuations.md#cont-04). */
AmaStatus ama_foreach_control_references(AmaWorld *, uint64_t, void *, AmaObjectReferenceMap, const void *, void *);
#endif
