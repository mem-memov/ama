/** @file
 * @brief Materialize fixed operation-frame offsets and exact chain release paths.
 * [CALL-09](../../../spec/language/calls-and-results.md#call-09) requires retained
 * ancestors, depth-first children and whole-chain release before a local handler.
 * The validated IR supplies ancestry and successors; this writer never rebuilds
 * source structure or keeps a C activation across an Ama label transfer.
 * [FLOW-04](../../../spec/language/control-flow.md#flow-04) uses the same exact
 * ancestry to release a successful condition header before either successor.
 */
#include "codegen/internal.h"
#include "emission/emission.h"
#include "lists/lists.h"

/** Release one exact frame domain; abandonment alone tolerates unentered slots. */
static bool release_frame(FILE *output, const ScaffoldType *type, const StorageFrame *frame, bool abandoning) {
    const char *function = frame->statement->operation_kind == SEMANTIC_LIST_SERVICE_OPERATION
        ? "ama_program_list_close_frame" : frame->statement->operation_kind == SEMANTIC_LIST_METHOD_OPERATION
        ? "ama_program_list_end_frame" : "ama_program_release_result";
    return fprintf(output, "%s(&ama_program,ama_current,", function) >= 0
        && codegen_list_offset(output, type, frame) && fprintf(output, ",%s)", abandoning ? "true" : "false") >= 0;
}

bool codegen_emit_tree_control(FILE *output, const ScaffoldType *type, const ControlNode *node, const char *name) {
    if (node->operation == CONTROL_ACQUIRE_INSTANCE) {
        if (node->statement->intrinsic_target) { return codegen_list_value_acquire(output, type, node, name); }
        if (fprintf(output, "{ bool refused=false; ama_status=ama_program_acquire_result(&ama_program,ama_current,&%s_Call,", name) < 0
            || !codegen_list_offset(output, type, node->frame)
            || fprintf(output, ",&refused);\nif(ama_status.code!=AMA_OK) { if(refused) goto ama_label_%zu; goto ama_project_finish; }\n",
                node->alternative->selector) < 0) { return false; }
        if (node->statement->syntax->as.call.mode == AST_ENTRY_CALL
            && fprintf(output, "ama_status=ama_program_prepare(&ama_program,ama_current,&%s_Call);\n"
                "if(ama_status.code!=AMA_OK) goto ama_project_finish;\n", name) < 0) { return false; }
        return fputs("}\n", output) != EOF;
    }
    if (node->operation == CONTROL_RELEASE_TARGET) {
        return fputs("ama_status=", output) != EOF && release_frame(output, type, node->frame, false)
            && fputs("; if(ama_status.code!=AMA_OK) goto ama_project_finish;\n", output) != EOF;
    }
    bool abandoning = node->operation == CONTROL_ABANDON_CHAIN;
    if ((!abandoning && node->operation != CONTROL_RELEASE_CHAIN)
        || fputs("{ AmaStatus released=ama_status_success();\n", output) == EOF) { return false; }
    const StorageFrame *last = abandoning ? node->frame : node->loop->test->frame;
    for (const StorageFrame *frame = last; frame != NULL; frame = frame->operation_parent) {
        bool ordinary_root = frame->operation_parent == NULL
            && frame->statement->operation_kind == SEMANTIC_INSTANCE_OPERATION;
        if (ordinary_root && abandoning && fputs("if(!ama_handle_is_empty(ama_cell->metadata.service.outgoing)) {\n", output) == EOF) { return false; }
        if (fputs("released=", output) == EOF) { return false; }
        if (ordinary_root) {
            if (fputs("ama_program_release(&ama_program,ama_current)", output) == EOF) { return false; }
        } else if (!release_frame(output, type, frame, abandoning)) { return false; }
        if (fputs("; if(released.code!=AMA_OK) { ama_status=released; goto ama_project_finish; }\n", output) == EOF
            || (ordinary_root && abandoning && fputs("}\n", output) == EOF)) { return false; }
    }
    return fputs("}\n", output) != EOF;
}
