/** @file
 * @brief Describe the six intrinsic list interfaces independently of Ama module instances and source declarations.
 * [LIST-04](../../../spec/language/lists.md#list-04) fixes method names,
 * independent input/result positions and element-reference versus integer types.
 */
#ifndef AMA_SEMANTIC_LIST_INTERFACE_H
#define AMA_SEMANTIC_LIST_INTERFACE_H
#include "source/text/source.h"
/** Intrinsic entries are compiler interfaces, never action constants or first-class method values. */
typedef enum { SEMANTIC_LIST_APPEND, SEMANTIC_LIST_INSERT, SEMANTIC_LIST_REMOVE,
    SEMANTIC_LIST_CLEAR, SEMANTIC_LIST_SWAP, SEMANTIC_LIST_LENGTH, SEMANTIC_LIST_METHOD_COUNT } SemanticListMethod;
/** Immutable field shape specialized to a service's element or exact standard integer type.
 * Element-reference inputs default empty; integer inputs default zero. Length's
 * owned integer output is constructed through the explicit from_size bridge.
 */
typedef struct { const char *name; bool element_reference; } SemanticListField;
/** Match one exact source base name without normalization, imports or fallback; false leaves the output unused. */
bool semantic_list_method(SourceSpan name, SemanticListMethod *method);
/** Return a static interface field by independent input/result ordinal, or null past arity; no allocation or instance creation. */
const SemanticListField *semantic_list_position(SemanticListMethod method, bool result, size_t position);
/** Match an exact field base name and return its ordinal, or null; static descriptors have process lifetime. */
const SemanticListField *semantic_list_name(SemanticListMethod method, bool result, SourceSpan name, size_t *position);
#endif
