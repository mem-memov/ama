/** @file
 * @brief Resolve constant namespaces from closed source-context evidence, preserving specialization identity.
 * [CONST-02](../../../spec/language/declarations.md#const-02) preserves qualified-source namespace identity.
 * [CONST-03](../../../spec/language/declarations.md#const-03) requires independent namespace selection.
 * [TPL-05](../../../spec/language/templates.md#tpl-05) retains every capture in nominal identity.
 * [CONST-05](../../../spec/language/declarations.md#const-05) carries the exact
 * selected declaration proof and optional original member provenance.
 */
#ifndef AMA_SPECIALIZATION_NAMESPACE_H
#define AMA_SPECIALIZATION_NAMESPACE_H
#include "semantic/semantic.h"

/** Resolve one saved read in its exact lexical concrete owner. Raw ordinary
 * analysis retains its unique source-path service. Closed analysis consumes all
 * matching incoming proofs, requiring one identical result; no receiver evidence
 * determines the namespace. Refusal records a semantic diagnostic.
 * [CONST-03](../../../spec/language/declarations.md#const-03) fixes namespace closure before member selection.
 */
SemanticType *semantic_read_namespace(SemanticProgram *, const SemanticType *, const AstNode *);
/** Select the exact declared symbol from the original source's closed value proofs.
 * Every matching caller proof must agree on namespace and declaration; no new
 * member lookup or guessed raw-path split occurs. On success member_site receives
 * actual optional provenance, which may belong to an alias file or be absent.
 * Failure leaves member_site unchanged and records a semantic diagnostic.
 * [CONST-02](../../../spec/language/declarations.md#const-02) selects the original immutable namespace;
 * [CONST-05](../../../spec/language/declarations.md#const-05) preserves its independently selected member.
 */
SemanticSymbol *semantic_qualified_constant(SemanticProgram *, const SemanticType *, const AstSource *, SourceSpan *);
#endif
