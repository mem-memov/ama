/**
 * @file
 * @brief Emit one host execution function with all Ama bodies as IR labels.
 *
 * [STACK-04](../../../spec/implementation/compiler.md#stack-04)
 * Resume dispatch validates saved selector and nominal type; return dispatch uses explicit caller selectors rather than recursive Ama C functions.
 * The assembly includes its own declarations so it also compiles as a standalone
 * translation unit; launchers may include it to retain the single-unit profile.
 */
#include "codegen/internal.h"

/**
 * Write one host execution function and all resolved control labels. Emit saved-position/type
 * and return-selector switches with invalid-selector failure paths; no recursive Ama C function
 * calls.
 */
bool codegen_emit_body(NativeScaffold *scaffold, FILE *output, const ScaffoldType *unused) {
    (void)unused;
    if (fputs("#include \"project.h\"\n", output) == EOF
        || !codegen_emit_host_begin(scaffold, output)) { return false; }
    if (fprintf(output, "goto ama_label_%zu;\n", scaffold->selected->plan->entry->selector) < 0) { return false; }
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        for (const ControlNode *node = type->plan->controls; node != NULL; node = node->allocated_next) {
            if (!codegen_emit_control(scaffold, output, type, node)) { return false; }
        }
    }
    if (fputs("ama_project_resume: __attribute__((unused));\nswitch(ama_return) {\n", output) == EOF) { return false; }
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        for (const ControlNode *node = type->plan->controls; node != NULL; node = node->allocated_next) {
            bool saved = false;
            for (const ControlNode *position = type->plan->controls; position != NULL; position = position->allocated_next) {
                if (position->operation == CONTROL_SAVE_POSITION && position->resume == node) { saved = true; break; }
            }
            if (saved && fprintf(output,
                    "case UINT64_C(%zu): if(ama_current.type_id!=UINT64_C(%zu)) goto ama_project_invalid_resume; goto ama_label_%zu;\n",
                    node->selector, codegen_type_id(scaffold,type), node->selector) < 0) { return false; }
        }
    }
    if (fputs("default: break;\n}\nama_project_invalid_resume: __attribute__((unused));\n"
            "ama_status=ama_status_make(AMA_INVALID_STATE,NULL,0); goto ama_project_finish;\n"
            "ama_project_yielded: __attribute__((unused));\n"
            "ama_outcome=AMA_HOST_YIELDED; goto ama_project_finish;\n", output) == EOF) { return false; }
    if (fputs("ama_project_return: __attribute__((unused));\nswitch(ama_return) {\n", output) == EOF) { return false; }
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        for (const ControlNode *node = type->plan->controls; node != NULL; node = node->allocated_next) {
            if (node->operation == CONTROL_SET_RETURN && fprintf(output, "case UINT64_C(%zu): goto ama_label_%zu;\n", node->resume->selector, node->resume->selector) < 0) { return false; }
        }
    }
    if (fputs("default: ama_status=ama_status_make(AMA_INVALID_STATE,NULL,0); goto ama_project_finish;\n}\n", output) == EOF) { return false; }
    return codegen_emit_host_finish(output);
}
