#include "codegen/internal.h"

bool codegen_emit_body(NativeScaffold *scaffold, FILE *output, const ScaffoldType *unused) {
    (void)unused;
    if (fputs("/* Included by the single launcher execution function. */\n"
            "AmaWorld ama_world = {.types = ama_project_registry,\n"
            "    .type_count = sizeof(ama_project_registry) / sizeof(ama_project_registry[0]),\n"
            "    .allocator = source_system_allocator()};\n"
            "AmaStatus ama_status = ama_status_success();\n"
            "(void)ama_world;\n(void)ama_status;\n(void)ama_project_prepare_types;\n", output) == EOF) { return false; }
    for (const SharedStorage *shared = scaffold->frontend->plan.shared_first; shared != NULL; shared = shared->next) {
        const char *name = codegen_constant_name(scaffold, shared->constant);
        if (name == NULL || fprintf(output, "(void)%s;\n", name) < 0) { return false; }
    }
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        for (const ControlNode *node = type->plan->controls; node != NULL; node = node->allocated_next) {
            if (fprintf(output, "if (0) goto ama_label_%zu;\n", node->selector) < 0) { return false; }
        }
    }
    if (fprintf(output, "goto ama_label_%zu;\n", scaffold->selected->plan->entry->selector) < 0) { return false; }
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        for (const ControlNode *node = type->plan->controls; node != NULL; node = node->allocated_next) {
            if (fprintf(output, "ama_label_%zu:;\nfputs(\"Ama execution wiring is not complete\\n\", stderr);\nreturn EXIT_FAILURE;\n", node->selector) < 0) { return false; }
        }
    }
    return true;
}

bool codegen_emit_launcher(NativeScaffold *scaffold, FILE *output, const ScaffoldType *unused) {
    (void)scaffold;
    (void)unused;
    return fputs("#include \"project.h\"\n\nint main(void) {\n    #include \"project.c\"\n}\n", output) != EOF;
}
