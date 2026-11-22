/** @file
 * @brief Export checked definition metadata through the compiler/project-tool boundary.
 * [PROJECT-01](../../../spec/implementation/project-tools.md#project-01) separates
 * compiler owners from project operations; [PKG-08](../../../spec/project/packages.md#pkg-08)
 * requires exact compiler-resolved dependencies and semantic test classification.
 * Export original resolved targets rather than reinterpreting source operands.
 * [DEP-03](../../../spec/implementation/compiler.md#dep-03)
 * Open definitions remain metadata until independently concrete use.
 * [TPL-06](../../../spec/language/templates.md#tpl-06)
 */
#ifndef AMA_DELIVERY_SOURCE_EXPORT_H
#define AMA_DELIVERY_SOURCE_EXPORT_H
#include "records.h"
#include "semantic/static/inventory.h"
/** Copy plain byte-sorted definitions and original ordered dependency edges from
 * a valid inventory/project into arena. The caller completes ordinary entry-free
 * frontend checking first; failed diagnostics cannot be exported as helper roles.
 * Copied names survive compiler teardown, but original SourceSpan byte owners must
 * outlive their consumption. Output changes only on success; failed scratch stays
 * arena-owned. No I/O, execution, source acquisition or test orchestration occurs.
 * O(N squared + E + name bytes) time and O(N+E+name bytes) owned space for N
 * definitions/E edges. [DEP-03](../../../spec/implementation/compiler.md#dep-03)
 * supplies targets; [TPL-06](../../../spec/language/templates.md#tpl-06) preserves
 * open definitions instead of inventing substitutions.
 */
SourceStatus delivery_source_export(SourceArena *, const StaticInventory *, const DeliverySourceDefinition **);
#endif
