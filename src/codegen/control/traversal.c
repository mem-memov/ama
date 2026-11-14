/** @file
 * @brief Execute foreach and retained-scope IR as stable-state helper/label transfers.
 * [FLOW-05](../../../spec/language/lists.md#flow-05) uses live selection and
 * explicit cleanup rather than snapshots; [CONT-04](../../../spec/language/continuations.md#cont-04)
 * resumes a valid body or the first missing iteration's corrected boundary.
 * [STACK-01](../../../spec/implementation/compiler.md#stack-01) keeps every Ama
 * body transfer at labels; these bounded C helpers return before user execution.
 */
#include "codegen/internal.h"
#include "emission/emission.h"

/** Validate all retained scopes before any saved body instruction. Repeat labels
 * are exact static foreach selectors, not arbitrary source-controlled integers.
 */
static bool codegen_traversal_resume(FILE *output, const ScaffoldType *type) {
    if (fputs("{ uint64_t repeat=0; ama_status=ama_program_foreach_resume(&ama_program,ama_current,&repeat);\n"
        "if(ama_status.code!=AMA_OK) goto ama_project_finish;\nswitch(repeat) { case 0: break;\n", output) == EOF) { return false; }
    for (const PlannedLoop *loop = type->plan->loops; loop != NULL; loop = loop->next) {
        if (loop->statement->syntax->kind != AST_FOREACH) { continue; }
        if (fprintf(output, "case UINT64_C(%zu): ama_cell->metadata.service.resume=(AmaResumePosition){0}; goto ama_label_%zu;\n",
            loop->repeat->selector, loop->repeat->selector) < 0) { return false; }
    }
    return fputs("default: ama_status=ama_status_make(AMA_INVALID_STATE,NULL,0); goto ama_project_finish; } }\n", output) != EOF;
}

bool codegen_emit_traversal_control(NativeScaffold *scaffold, FILE *output,
    const ScaffoldType *type, const ControlNode *node) {
    const char *name = type->names[SCAFFOLD_WRITE]->internal_name;
    if (node->operation == CONTROL_RESET_SCOPES || node->operation == CONTROL_CLEANUP_SCOPES) {
        if (fputs("ama_status=ama_program_foreach_reset(&ama_program,ama_current);\n", output) == EOF) { return false; }
    } else if (node->operation == CONTROL_VALIDATE_TRAVERSALS) {
        if (!codegen_traversal_resume(output, type)) { return false; }
    } else {
        size_t scope;
        if (!codegen_foreach_scope_index(type, node->loop, &scope)) {
            return scaffold_error(scaffold, DIAGNOSTIC_TYPE_MISMATCH, "A traversal control has no exact lexical scope", NULL);
        }
        if (node->operation == CONTROL_FOREACH_NEXT) {
            if (node->next == NULL || node->alternative == NULL) {
                return scaffold_error(scaffold, DIAGNOSTIC_TYPE_MISMATCH, "A traversal selection lacks an explicit successor", NULL);
            }
            return fprintf(output, "{ bool selected=false; ama_status=ama_program_foreach_next(&ama_program,ama_current,&%s_ForeachScopes[%zu].plan,&selected);\n"
                "if(ama_status.code!=AMA_OK) goto ama_project_finish;\nif(selected) goto ama_label_%zu; else goto ama_label_%zu; }\n",
                name, scope, node->next->selector, node->alternative->selector) >= 0;
        }
        const char *function = node->operation == CONTROL_FOREACH_EMPTY ? "ama_program_foreach_empty"
            : node->operation == CONTROL_FOREACH_ENTER ? "ama_program_foreach_enter"
            : node->operation == CONTROL_FOREACH_LEAVE ? "ama_program_foreach_leave" : NULL;
        if (function == NULL || fprintf(output, "ama_status=%s(&ama_program,ama_current,&%s_ForeachScopes[%zu].plan%s);\n",
            function, name, scope, node->operation == CONTROL_FOREACH_LEAVE ? ",true" : "") < 0) { return false; }
    }
    return fputs("if(ama_status.code!=AMA_OK) goto ama_project_finish;\n", output) != EOF
        && codegen_control_jump(output, node->next == NULL, node->next == NULL ? 0 : node->next->selector);
}
