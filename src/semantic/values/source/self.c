/**
 * @file
 * @brief Type standalone self as the exact lexical module rather than the dynamic caller or destination.
 * [REF-01](../../../../spec/language/values-and-references.md#ref-01) supplies
 * one scalar nonowning identity and forbids shared initialization. Execution and
 * final construction/clone identities belong to the later runtime tasks.
 */
#include "internal.h"
bool semantic_self_source(SemanticProgram *program, SemanticType *type,
    SemanticSource *source, const SemanticSymbol *initializing) {
    if (initializing != NULL && initializing->role == SYMBOL_CONSTANT) {
        return semantic_fail(program, source->site, "Shared initialization has no current instance for standalone &", initializing);
    }
    source->type = type;
    source->reference = true;
    return true;
}
