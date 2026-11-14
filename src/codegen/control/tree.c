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

/** Emit the exact generated storage offset, with no guessed layout arithmetic. */
static bool frame_offset(FILE *output, const ScaffoldType *type, const StorageFrame *frame) {
    return fprintf(output, "offsetof(%s_Storage,operation_frames[%zu])",
        type->names[SCAFFOLD_WRITE]->internal_name, frame->slot) >= 0;
}

/** Emit a checked selected release; abandonment alone tolerates an unentered slot. */
static bool release_frame(FILE *output, const ScaffoldType *type, const StorageFrame *frame, bool abandoning) {
    return fputs("ama_program_release_result(&ama_program,ama_current,", output) != EOF
        && frame_offset(output, type, frame) && fprintf(output, ",%s)", abandoning ? "true" : "false") >= 0;
}

bool codegen_emit_tree_control(FILE *output, const ScaffoldType *type, const ControlNode *node, const char *name) {
    if (node->operation == CONTROL_ACQUIRE_INSTANCE) {
        if (fprintf(output, "{ bool refused=false; ama_status=ama_program_acquire_result(&ama_program,ama_current,&%s_Call,", name) < 0
            || !frame_offset(output, type, node->frame)
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
    for (const StorageFrame *frame = last; frame != NULL && frame->operation_parent != NULL; frame = frame->operation_parent) {
        if (fputs("released=", output) == EOF || !release_frame(output, type, frame, abandoning)
            || fputs("; if(released.code!=AMA_OK) { ama_status=released; goto ama_project_finish; }\n", output) == EOF) { return false; }
    }
    if (abandoning && fputs("if(!ama_handle_is_empty(ama_cell->metadata.service.outgoing)) {\n", output) == EOF) { return false; }
    return fputs("released=ama_program_release(&ama_program,ama_current);\n"
        "if(released.code!=AMA_OK) { ama_status=released; goto ama_project_finish; }\n", output) != EOF
        && (!abandoning || fputs("}\n", output) != EOF) && fputs("}\n", output) != EOF;
}
