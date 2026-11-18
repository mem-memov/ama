/**
 * @file
 * @brief Own static module inventories and expose exact source-scoped queries.
 *
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03) requires explicit
 * scratch ownership. [TPL-02](../../../spec/language/templates.md#tpl-02) and
 * [TPL-12](../../../spec/language/static-results.md#tpl-12) require exact module
 * namespaces and source-position alias visibility without runtime storage.
 */
#ifndef AMA_SEMANTIC_STATIC_INVENTORY_H
#define AMA_SEMANTIC_STATIC_INVENTORY_H

#include "model.h"

/** Failure prevents publication; partial arena records still have one owner.
 * Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03).
 */
typedef enum { STATIC_OK, STATIC_INVALID, STATIC_OUT_OF_MEMORY } StaticStatus;

/** Own all static scratch; borrow a valid immutable Project and allocator.
 * A valid flag certifies completed collection only, not concrete specialization
 * or generated execution. Related diagnostics borrow retained project sites.
 * Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03),
 * [TPL-02](../../../spec/language/templates.md#tpl-02).
 */
typedef struct {
    SourceArena arena;
    const Project *project;
    StaticModule *first;
    StaticModule *last;
    StaticStatus status;
    Diagnostic error;
    DiagnosticRelated related;
    bool attempted;
    bool valid;
} StaticInventory;

/** Initialize an empty owner; project and allocator outlive destruction.
 * No source is parsed and no allocation is performed.
 * Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03).
 */
void static_inventory_init(StaticInventory *, const Project *, SourceAllocator);

/** Collect once from an empty initialized owner. Publish local lexical validity
 * and known signature checks; explicit pending application/result obligations
 * still require later normalization and inference before code generation.
 * Component contract: [TPL-02](../../../spec/language/templates.md#tpl-02),
 * [TPL-12](../../../spec/language/static-results.md#tpl-12).
 */
StaticStatus static_inventory_collect(StaticInventory *);

/** Release all partial/complete records, invalidate views and zero the owner.
 * Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03).
 */
void static_inventory_destroy(StaticInventory *);

/** Find an already inserted module by exact loaded definition identity.
 * Component contract: [TPL-02](../../../spec/language/templates.md#tpl-02).
 * This query does not certify that the inventory has finished collection.
 */
const StaticModule *static_module_find(const StaticInventory *, const ProjectModule *);

/** Find an inserted complete name at an original source byte position.
 * Parameters/results are module-wide; aliases require position >= introduction.
 * Component contract: [TPL-12](../../../spec/language/static-results.md#tpl-12).
 * No foreign names, normalization, inference or filesystem lookup is added.
 */
const StaticSymbol *static_symbol_find(const StaticModule *, SourceSpan, size_t);

/** Select an original input/result by its dense role ordinal, independently of
 * allocation-chain order. Borrow a collected module with completed ordinals;
 * return null for absent modules, aliases, unsupported roles or out-of-range
 * positions. This query neither validates nor changes the inventory.
 * Contract: [TPL-02](../../../spec/language/templates.md#tpl-02),
 * [TPL-12](../../../spec/language/static-results.md#tpl-12).
 */
const StaticSymbol *static_symbol_at(const StaticModule *, StaticRole, size_t);
#endif
