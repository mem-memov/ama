/**
 * @file
 * @brief Compare finite actual constructor metadata with explicit scratch ownership.
 *
 * [TPL-07](../../../../spec/language/templates.md#tpl-07) matches exact name
 * sets and recursive categories, not source order. Deferred metadata remains
 * pending; [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * retains ownership on allocation failure without recursive C activation.
 */
#ifndef AMA_STATIC_SIGNATURE_H
#define AMA_STATIC_SIGNATURE_H
#include "../inventory.h"

/** Pending is a real unresolved obligation; it never certifies equality.
 * Contract: [TPL-07](../../../../spec/language/templates.md#tpl-07).
 */
typedef enum { STATIC_SIGNATURE_EQUAL, STATIC_SIGNATURE_DIFFERENT,
    STATIC_SIGNATURE_PENDING } StaticSignatureComparison;

/** Compare accepted finite signatures using independent arena-owned scratch.
 * Complete name sets and recursive categories must agree; source order is irrelevant.
 * A known mismatch wins over deferred entries. Borrow all inputs and publish output only on
 * success. False means scratch allocation refusal; input metadata is unchanged.
 * The caller owns failure diagnostics and destroys all partial arena storage.
 * Contract: [TPL-07](../../../../spec/language/templates.md#tpl-07),
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
bool static_signature_compare_arena(SourceArena *, const StaticSignature *,
    const StaticSignature *, StaticSignatureComparison *);
#endif
