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
#include "usage/usage.h"
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
/** Freeze exact bare local forwarding classes with canonical source representatives.
 * Applied/producer equations belong to subsequent normalizers, not this relation.
 * [TPL-07](../../../../spec/language/templates.md#tpl-07) propagates all use requirements.
 */
void static_prepare_usage_groups(StaticInventory *);
/** Inspect one application against its actual head metadata without closing a
 * minimum requirement set. Pending is evidence still owed by later solving.
 * [TPL-07](../../../../spec/language/templates.md#tpl-07) preserves residuals.
 */
StaticUsageResult static_usage_expression(const StaticInventory *, const StaticModule *,
    const AstTypeExpression *, StaticCategory, const StaticSignature *, const StaticSymbol *);
/** Read independently known head metadata, not the result of its application.
 * [TPL-07](../../../../spec/language/templates.md#tpl-07) separates actuals and operations.
 */
StaticMetadata static_head_metadata(const StaticInventory *, const StaticModule *, const AstTypeExpression *);
/** Monotonically resolve finite known result and constructor slots. Applications
 * and projected aliases remain explicit obligations; no default is chosen.
 * Contract: [TPL-12](../../../../spec/language/static-results.md#tpl-12).
 */
void static_resolve_metadata(StaticInventory *);
/** Preserve an actual known constructor interface alongside the original
 * signature-use failure, without allocation or changing earlier errors.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) retains responsible original sites.
 */
bool static_signature_error(StaticInventory *, SourceSpan, const char *, const StaticSymbol *, const ProjectModule *);
#endif
