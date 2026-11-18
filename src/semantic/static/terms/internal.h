/**
 * @file
 * @brief Declare checked term construction, identity membership and first-error boundaries.
 *
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial
 * records. [TPL-08](../../../../spec/language/templates.md#tpl-08) requires
 * exact identity membership; [COMP-04](../../../../spec/implementation/compiler.md#comp-04)
 * retains original source diagnostics without reporting allocation.
 */
#ifndef AMA_STATIC_TERM_INTERNAL_H
#define AMA_STATIC_TERM_INTERNAL_H
#include "store.h"

/** Allocate zeroed store-owned records only while healthy.
 * Contract: [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
void *static_term_allocate(StaticTermStore *, size_t);
/** Preserve the first allocation refusal without allocating diagnostics.
 * Contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04).
 */
bool static_term_memory_failure(StaticTermStore *);
/** Preserve a constant message/original site; absent sites denote bad internal
 * identity metadata, never fabricated source.
 * Contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04).
 */
bool static_term_invalid(StaticTermStore *, SourceSpan, const char *);
/** Check exact registered source module membership before dereferencing it.
 * Contract: [TPL-08](../../../../spec/language/templates.md#tpl-08).
 */
bool static_term_has_module(const StaticTermStore *, const StaticModule *);
/** Check exact symbol membership in the accepted source inventory.
 * Contract: [TPL-08](../../../../spec/language/templates.md#tpl-08).
 */
bool static_term_has_symbol(const StaticTermStore *, const StaticSymbol *);
/** Check exact already published environment membership without allocation.
 * Contract: [TPL-08](../../../../spec/language/templates.md#tpl-08).
 */
bool static_term_has_environment(const StaticTermStore *, const StaticTermEnvironment *);
/** Check exact already published value membership without structural guesses.
 * Contract: [TPL-08](../../../../spec/language/templates.md#tpl-08).
 */
bool static_term_has_value(const StaticTermStore *, const StaticTerm *);
/** Reuse an exact variable key or prepare/link one complete new vertex.
 * Contract: [TPL-08](../../../../spec/language/templates.md#tpl-08).
 */
bool static_term_variable(StaticTermStore *, StaticVariableIdentity, StaticCategory,
    const StaticSignature *, SourceSpan, const StaticTerm **);
#endif
