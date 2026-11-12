/**
 * @file
 * @brief Publish shared startup order and reverse cleanup links from a valid relation.
 *
 * [CONST-02](../../../spec/language/declarations.md#const-02)
 * Shared storage is initialized once and cleaned in the exact reverse successful order, independently of instance pools.
 */
#include <assert.h>
#include "semantic/internal.h"

SemanticStatus semantic_constants(SemanticProgram *program) {
    assert(program->relations_valid && !program->constants_attempted);
    program->constants_attempted = true;
    for (GraphVertex *vertex = program->constants.order; vertex != NULL; vertex = vertex->order_next) {
        SemanticConstantPlan *plan = semantic_allocate(program, sizeof(*plan));
        if (plan == NULL) {
            return program->status;
        }
        SemanticSymbol *symbol = (SemanticSymbol *)vertex->value;
        *plan = (SemanticConstantPlan){.previous = program->constant_last,
            .symbol = symbol, .ordinal = program->constant_count++,
            .initializers = symbol->initializers};
        if (program->constant_last == NULL) {
            program->constant_first = plan;
        } else {
            program->constant_last->next = plan;
        }
        program->constant_last = plan;
        symbol->shared = plan;
    }
    program->constants_valid = true;
    return SEMANTIC_OK;
}
