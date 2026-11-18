/**
 * @file
 * @brief Build and compare finite named constructor signatures independently of local scope.
 *
 * [TPL-07](../../../../spec/language/templates.md#tpl-07) matches exact name
 * sets and recursive categories, not source order.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) allows omitted
 * producer inputs when an output phase requests later closure. Explicitly deferred metadata
 * remains pending; [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
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

/** Exact compares full sets; provided compares a supplied left subset at the
 * root only. Nested Template signatures always require complete equality.
 * Contract: [TPL-07](../../../../spec/language/templates.md#tpl-07),
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12).
 */
typedef enum { STATIC_SIGNATURE_EXACT, STATIC_SIGNATURE_PROVIDED } StaticSignatureMatch;

/** Build a nonempty explicit signature from immutable sibling formals. Reject
 * same-level duplicate complete names. Publish output only on full success;
 * inventory owns every partial allocation. Input must be an acyclic AST tree.
 * Contract: [TPL-07](../../../../spec/language/templates.md#tpl-07).
 */
bool static_signature_build(StaticInventory *, const AstStaticFormal *, StaticSignature **);

/** Compare finite acyclic signatures with distinct names per level. Visit all
 * known constraints: a known mismatch wins over deferred entries. Pending is
 * reported only if no known mismatch exists. Input/output borrow caller storage;
 * scratch belongs to inventory, and output is untouched on allocation failure.
 * PROVIDED checks supplied left names against right; omitted right slots are
 * allowed only at the root. TPL-12 resolves them at the later producer closure.
 * Contract: [TPL-07](../../../../spec/language/templates.md#tpl-07).
 */
bool static_signature_compare(StaticInventory *, const StaticSignature *,
    const StaticSignature *, StaticSignatureMatch, StaticSignatureComparison *);
/** Compare accepted finite signatures using independent arena-owned scratch.
 * Borrow all inputs, retain no temporary caller state and publish output only on
 * success. False means scratch allocation refusal; no inventory is mutated.
 * The caller owns failure diagnostics and destroys all partial arena storage.
 * Contract: [TPL-07](../../../../spec/language/templates.md#tpl-07),
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
bool static_signature_compare_arena(SourceArena *, const StaticSignature *,
    const StaticSignature *, StaticSignatureMatch, StaticSignatureComparison *);
#endif
