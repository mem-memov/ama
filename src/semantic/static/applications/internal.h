/** @file
 * @brief Declare checked application preparation, source context and first-error operations.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial snapshots.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) preserves actual formal identities.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) propagates explicit equality evidence.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) retains original sites.
 */
#ifndef AMA_STATIC_APPLICATION_INTERNAL_H
#define AMA_STATIC_APPLICATION_INTERNAL_H
#include "plan.h"
#include "../constraints/internal.h"

/** Allocate zeroed plan-owned records only while healthy.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial records.
 */
void *static_application_allocate(StaticApplicationPlan *, size_t);
/** Keep the first constant diagnostic and optional actual formal definition.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) avoids reporting allocation.
 */
bool static_application_fail(StaticApplicationPlan *, StaticApplicationStatus,
    SourceSpan, const char *, const StaticSymbol *);
/** Propagate borrowed owner errors without copying/destroying their metadata.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves original context.
 */
bool static_application_dependencies(StaticApplicationPlan *);
/** Check supplied omission/source context before generating any terms.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) fixes lexical scope.
 */
bool static_application_context_valid(StaticApplicationPlan *, StaticApplicationContext);
/** Append a complete written/generated input snapshot with checked count.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial links.
 */
bool static_application_input(StaticApplicationPlan *, StaticApplicationRecord *, StaticApplicationInput);
/** Read a value's known declared/equated contract without inventing its origin.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) preserves explicit connections.
 */
void static_application_description(const StaticApplicationPlan *, const StaticTerm *,
    StaticCategory *, const StaticSignature **);
/** Validate one already prepared snapshot before or after publication.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) requires binding contracts.
 */
bool static_application_validate(StaticApplicationPlan *, const StaticApplicationRecord *, StaticApplicationOutcome *);
/** Validate written original slots against the actual residual snapshot.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) forbids captured rebinding.
 */
bool static_application_inputs_valid(StaticApplicationPlan *, const StaticTerm *,
    const StaticApplicationInput *, size_t, SourceSpan);
/** Merge actual captures and supplied/generated values without expanding bodies.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) preserves partial identity.
 */
bool static_application_merge(StaticApplicationPlan *, StaticApplicationRecord *);
/** Read independently refined expected metadata or the original declaration.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) keeps argument scope separate.
 */
const StaticSignature *static_application_expected(const StaticApplicationPlan *,
    const StaticApplicationRecord *, const StaticSymbol *);
/** Publish a complete residual description only when every local slot's expected
 * metadata is independently known; the solver owns the resulting snapshot.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) preserves local binders.
 */
bool static_application_residual_metadata(StaticApplicationPlan *, const StaticApplicationRecord *);
#endif
