/** @file
 * @brief Require the accepted guarded source/Type proof before ordinary operation validation.
 * [TYPE-06](../../../spec/language/declarations.md#type-06) fixes lexical field identity before substitution;
 * [TPL-06](../../../spec/language/templates.md#tpl-06) requires closed concrete bindings without replaying inference.
 */
#include <assert.h>
#include "semantic/internal.h"
#include "semantic/static/closure/plan.h"

/** Source collection, typed connections and closure already establish every
 * inferred field and its exact capture context. No module-path projection or
 * second event classifier is permitted here; missing proof refuses publication.
 * [TPL-06](../../../spec/language/templates.md#tpl-06) distinguishes accepted Type inference from subsequent ordinary operation checks.
 */
SemanticStatus semantic_infer(SemanticProgram *program) {
    assert(program->names_valid && program->native_valid && !program->inference_attempted);
    program->inference_attempted = true;
    if (!program->type_closure || !program->type_closure->closed || program->type_closure->status != STATIC_CLOSURE_OK) {
        (void)semantic_fail(program,(SourceSpan){0},"Ordinary checking requires accepted guarded Type inference",NULL);
        return program->status;
    }
    program->inference_valid = true;
    return SEMANTIC_OK;
}
