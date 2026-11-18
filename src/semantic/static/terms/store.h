/**
 * @file
 * @brief Own immutable term/environment records over accepted scoped source metadata.
 *
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial
 * construction. [TPL-08](../../../../spec/language/templates.md#tpl-08) fixes
 * identity by lexical environments rather than same-spelled names.
 * [TPL-05](../../../../spec/language/templates.md#tpl-05) preserves complete original capture maps.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) separates immutable terms from later solutions.
 */
#ifndef AMA_STATIC_TERM_STORE_H
#define AMA_STATIC_TERM_STORE_H
#include "model.h"
#include "../inventory.h"

/** Sticky construction failures invalidate further operations, not borrowed
 * input inventories or prior immutable records.
 * Contract: [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
typedef enum { STATIC_TERM_OK, STATIC_TERM_INVALID, STATIC_TERM_OUT_OF_MEMORY } StaticTermStatus;

/** Forward separate use-provenance owner; values do not absorb application sites.
 * Contract: [TPL-08](../../../../spec/language/templates.md#tpl-08).
 */
struct StaticTermApplication;

/** Borrow the accepted inventory/Project until destruction. Own only compiler
 * terms/environments/scratch; equations and runtime state have other owners.
 * Contract: [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
struct StaticTermStore {
    SourceArena arena;
    const StaticInventory *inventory;
    StaticTerm *first;
    StaticTerm *last;
    size_t term_count;
    StaticTermEnvironment *first_environment;
    StaticTermEnvironment *last_environment;
    size_t environment_count;
    struct StaticTermApplication *first_application;
    struct StaticTermApplication *last_application;
    size_t application_count;
    StaticTermStatus status;
    Diagnostic error;
};

/** Initialize without allocation, borrowing a valid immutable inventory.
 * Contract: [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
void static_term_store_init(StaticTermStore *, const StaticInventory *, SourceAllocator);
/** Destroy all partial records without following source/value graph edges.
 * Contract: [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
void static_term_store_destroy(StaticTermStore *);
/** Reuse/create a definition or supplied normalized owner-Type environment.
 * Source module and optional owner value must belong to this store's domain.
 * Publish only on success; no source bodies are expanded.
 * Contract: [TPL-08](../../../../spec/language/templates.md#tpl-08).
 */
bool static_term_environment(StaticTermStore *, const StaticModule *, const StaticTerm *, const StaticTermEnvironment **);
/** Resolve a whole-module local symbol in its explicit environment. An applied
 * owner's formal returns its supplied value; generic formals/results/aliases
 * have stable scoped variables. No text lookup or equation solving occurs.
 * Contract: [TPL-08](../../../../spec/language/templates.md#tpl-08).
 */
bool static_term_symbol(StaticTermStore *, const StaticTermEnvironment *, const StaticSymbol *, const StaticTerm **);
/** Reuse/create a field or producer inference variable with explicit lexical
 * owner and original residual formal. Exactly one declaration/producer is set.
 * Category/signature come from that formal, never a runtime sample/default.
 * Contract: [TPL-08](../../../../spec/language/templates.md#tpl-08).
 */
bool static_term_inference(StaticTermStore *, const StaticTermEnvironment *, const StaticSymbol *,
    const AstNode *, const AstTypeExpression *, const StaticTerm **);
/** Construct an immutable nominal from unique exact original formal/value pairs.
 * Captures are a count-sized array; next links are ignored. Input order is
 * irrelevant; omitted original formals become local residuals.
 * All values must already belong to this store. No application/type solving occurs.
 * Contract: [TPL-05](../../../../spec/language/templates.md#tpl-05).
 */
bool static_term_nominal(StaticTermStore *, const StaticModule *,
    const StaticTermCapture *, size_t, SourceSpan, const StaticTerm **);
/** Compare canonical immutable values within one owner; source sites do not
 * affect identity. Invalid/foreign handles fail instead of comparing spelling.
 * Contract: [TPL-08](../../../../spec/language/templates.md#tpl-08).
 */
bool static_term_equal(StaticTermStore *, const StaticTerm *, const StaticTerm *, bool *);
/** Report absence of surrounding free variables. Locally bound residuals are
 * allowed; this does not admit executable specialization or resolve signatures.
 * Contract: [TPL-10](../../../../spec/language/templates.md#tpl-10).
 */
bool static_term_ground(StaticTermStore *, const StaticTerm *, bool *);
#endif
