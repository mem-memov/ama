/**
 * @file
 * @brief Declare static inventory construction and allocation-free failure boundaries.
 *
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03) keeps failed
 * construction owned. [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * preserves original diagnostics. [TPL-02](../../../spec/language/templates.md#tpl-02)
 * requires complete local name identity, not first-actual classification.
 * [TPL-07](../../../spec/language/templates.md#tpl-07) fixes finite named signatures.
 */
#ifndef AMA_SEMANTIC_STATIC_INTERNAL_H
#define AMA_SEMANTIC_STATIC_INTERNAL_H
#include "inventory.h"

/** Allocate zeroed inventory-owned storage; retain the first failure, with no
 * caller-owned partial record. Contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03).
 */
void *static_allocate(StaticInventory *, size_t);
/** Record allocation failure without allocating or publishing validity.
 * Contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04).
 */
bool static_memory_failure(StaticInventory *);
/** Record the first invalid source and optional prior declaration, borrowing
 * all sites/messages. Contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04).
 */
bool static_invalid(StaticInventory *, SourceSpan, const char *, const StaticSymbol *);
/** Compare valid complete names by exact byte sequences, even across sources.
 * Contract: [TPL-02](../../../spec/language/templates.md#tpl-02).
 */
bool static_same_name(SourceSpan, SourceSpan);
/** Reuse one exact module record or append a zero-symbol owner; null on failure.
 * The definition belongs to the inventory's borrowed Project.
 * Contract: [TPL-02](../../../spec/language/templates.md#tpl-02).
 */
StaticModule *static_module_add(StaticInventory *, const ProjectModule *);
/** Append one new local name; duplicate names fail regardless of declaration role.
 * Ordinals stay unassigned until collection finishes. Caller supplies valid
 * source spans from this module, an inventory-owned module, a valid role and
 * a category appropriate for this construction phase.
 * Contract: [TPL-02](../../../spec/language/templates.md#tpl-02).
 */
StaticSymbol *static_symbol_add(StaticInventory *, StaticModule *, SourceSpan,
    SourceSpan, StaticRole, StaticCategory);
/** Collect expression heads and nonempty static receivers from the module's
 * owning AST edges. Keep original sites/context; do not bind names or publish
 * inventory validity. Contract: [TPL-02](../../../spec/language/templates.md#tpl-02),
 * [TPL-12](../../../spec/language/static-results.md#tpl-12).
 */
bool static_collect_occurrences(StaticInventory *, StaticModule *);
/** Stably order a finite occurrence chain by original byte offset without
 * allocation. Contract: [TPL-12](../../../spec/language/static-results.md#tpl-12)
 * makes alias classification depend on lexical position rather than AST links.
 */
void static_sort_occurrences(StaticModule *);
/** Collect public static graph pair names before free-input classification
 * or uses. Exact defining expressions are retained for the next phase.
 * Contract: [TPL-02](../../../spec/language/templates.md#tpl-02),
 * [TPL-12](../../../spec/language/static-results.md#tpl-12).
 */
bool static_collect_explicit(StaticInventory *, StaticModule *);
/** Collect source-ordered first receipts after pair collection; later receipts
 * reuse the pair/first-alias identity. Borrow valid graph syntax; partial records
 * remain inventory-owned on failure. [TPL-12](../../../spec/language/static-results.md#tpl-12).
 */
bool static_collect_receivers(StaticInventory *, StaticModule *);
/** Assign dense internal indices for a complete parameter or public output
 * group from distinct defining offsets, without allocation or relinking.
 * [TPL-02](../../../spec/language/templates.md#tpl-02),
 * [TPL-12](../../../spec/language/static-results.md#tpl-12) retain deterministic identities.
 */
void static_order_symbols(StaticModule *, StaticRole);
/** Bind every collected local use to its exact visible symbol or loaded path
 * occurrence. Unknown names never introduce body parameters.
 * Contract: [TPL-02](../../../spec/language/templates.md#tpl-02),
 * [TPL-12](../../../spec/language/static-results.md#tpl-12).
 */
bool static_bind_occurrences(StaticInventory *, StaticModule *);
/** Bind whole qualified operands to already normalized explicit dependency edges.
 * Descriptor and namespace outputs borrow project ownership; a missing original
 * edge is an inventory error, never a same-text lookup fallback.
 * [CONST-05](../../../spec/language/declarations.md#const-05) and
 * [DEP-03](../../../spec/implementation/compiler.md#dep-03) preserve occurrence identity.
 */
bool static_bind_constant_occurrences(StaticInventory *, StaticModule *);
/** Resolve only an exact original nonimplicit loaded request span; no textual
 * path search or new loading. Null means no matching dependency occurrence.
 * [DEP-03](../../../spec/implementation/compiler.md#dep-03) retains request provenance.
 */
const ProjectModule *static_loaded_origin(const StaticModule *, SourceSpan);
/** Build signature skeletons over already classified and bound modules; retain
 * pending expression metadata instead of inferring categories from actuals.
 * Contract: [TPL-07](../../../spec/language/templates.md#tpl-07).
 */
bool static_prepare_signatures(StaticInventory *);
/** Resolve known metadata and reject finite-signature cycles or known Type
 * applications; deferred applications/projections remain explicit obligations.
 * Contract: [TPL-07](../../../spec/language/templates.md#tpl-07).
 */
bool static_check_signatures(StaticInventory *);
/** Reject cycles among known nested signature edges with explicit traversal
 * frames. Deferred edges must be rechecked when later metadata resolves them.
 * Contract: [TPL-02](../../../spec/language/templates.md#tpl-02).
 */
bool static_signatures_acyclic(StaticInventory *);
#endif
