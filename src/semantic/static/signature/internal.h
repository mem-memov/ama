/**
 * @file
 * @brief Share known metadata pairs between finite propagation and signature checking.
 *
 * [TPL-07](../../../../spec/language/templates.md#tpl-07) distinguishes
 * deferred metadata from Type and Template; [TPL-12](../../../../spec/language/static-results.md#tpl-12)
 * requires actual producer metadata for selected aliases.
 */
#ifndef AMA_STATIC_SIGNATURE_INTERNAL_H
#define AMA_STATIC_SIGNATURE_INTERNAL_H
#include "signature.h"
#include "../internal.h"

/** Deferred carries no assumed category; Template borrows an owned signature.
 * Component contract: [TPL-07](../../../../spec/language/templates.md#tpl-07).
 */
typedef struct { StaticCategory category; StaticSignature *signature; } StaticMetadata;
/** Read only independently known expression metadata. Bound AST/source and
 * inventory roots outlive this view. No allocation or actual selection occurs.
 * Contract: [TPL-07](../../../../spec/language/templates.md#tpl-07).
 */
StaticMetadata static_expression_metadata(const StaticInventory *, const StaticModule *, const AstTypeExpression *);
/** Monotonically resolve finite known result and constructor slots. Applications
 * and projected aliases remain explicit obligations; no default is chosen.
 * Contract: [TPL-12](../../../../spec/language/static-results.md#tpl-12).
 */
void static_resolve_metadata(StaticInventory *);
#endif
