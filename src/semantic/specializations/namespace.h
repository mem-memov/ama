/** @file
 * @brief Resolve constant namespaces from closed source-context evidence, preserving specialization identity.
 * [CONST-02](../../../spec/language/declarations.md#const-02) preserves qualified-source namespace identity.
 * [CONST-03](../../../spec/language/declarations.md#const-03) requires independent namespace selection.
 * [TPL-05](../../../spec/language/templates.md#tpl-05) retains every capture in nominal identity.
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
/** Resolve one original qualified constant source in its actual concrete owner.
 * Allocation/normalization failure preserves source owners and partial metadata.
 * [CONST-02](../../../spec/language/declarations.md#const-02) selects the original immutable namespace.
 */
SemanticType *semantic_qualified_namespace(SemanticProgram *, const SemanticType *, const AstSource *);
#endif
