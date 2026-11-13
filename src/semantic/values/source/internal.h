/**
 * @file
 * @brief Separate source categories while retaining one typed transfer contract.
 * [TYPE-05](../../../../spec/language/declarations.md#type-05) requires exact
 * form/type identity; [REF-01](../../../../spec/language/values-and-references.md#ref-01)
 * gives standalone self a lexical identity with no named access.
 */
#ifndef AMA_SEMANTIC_SOURCE_INTERNAL_H
#define AMA_SEMANTIC_SOURCE_INTERNAL_H
#include "semantic/internal.h"
/** Resolve a qualified immutable member and its original declaration/visibility context; false records an exact semantic failure. */
bool semantic_constant_source(SemanticProgram *, SemanticSource *, const SemanticSymbol *);
/** Resolve named local/module accesses and source-mode flags; borrowed syntax and resulting views retain the semantic owner's lifetime. */
bool semantic_instance_source(SemanticProgram *, SemanticType *, SemanticSource *, const SemanticSymbol *);
/** Type a nonowning scalar self handle as the lexical module; shared initialization has no such instance. */
bool semantic_self_source(SemanticProgram *, SemanticType *, SemanticSource *, const SemanticSymbol *);
#endif
