/** @file
 * @brief Specialize intrinsic interfaces to exact element and standard integer types without creating method storage.
 * [LIST-04](../../../spec/language/lists.md#list-04) fixes typed inputs/results;
 * [LIST-09](../../../spec/language/lists.md#list-09) distinguishes omitted defaults from explicit owning copies.
 */
#ifndef AMA_SEMANTIC_LIST_INTERNAL_H
#define AMA_SEMANTIC_LIST_INTERNAL_H
#include "semantic/internal.h"
/** Require exact native ..integer copy/literal/index contracts and retain the later typed from_size bridge obligation. Component contract: [LIST-04](../../../spec/language/lists.md#list-04), [LIST-09](../../../spec/language/lists.md#list-09). */
SemanticType *semantic_list_integer(SemanticProgram *, SourceSpan);
/** Classify a whole-list retained parent with selected methods and no ordinary bindings; invalid profiles record original diagnostics. Component contract: [LIST-04](../../../spec/language/lists.md#list-04), [LIST-09](../../../spec/language/lists.md#list-09). */
bool semantic_list_service(SemanticProgram *, SemanticStatement *);
/** Resolve a fixed selected method or its ephemeral scalar output; ordinary operands remain caller-side. Component contract: [LIST-04](../../../spec/language/lists.md#list-04), [LIST-09](../../../spec/language/lists.md#list-09). */
bool semantic_list_target(SemanticProgram *, SemanticStatement *, SemanticStatement *);
/** Specialize one declared intrinsic input/result by exact name/position; no synthetic source declaration is manufactured. Component contract: [LIST-04](../../../spec/language/lists.md#list-04), [LIST-09](../../../spec/language/lists.md#list-09). */
bool semantic_list_binding(SemanticProgram *, SemanticStatement *, const AstArgument *, SemanticBinding *);
/** After normal source typing, require cloning only for supplied append/insert sources in owning lists. Component contract: [LIST-04](../../../spec/language/lists.md#list-04), [LIST-09](../../../spec/language/lists.md#list-09). */
bool semantic_list_copy_requirement(SemanticProgram *, const SemanticStatement *, const SemanticBinding *);
#endif
