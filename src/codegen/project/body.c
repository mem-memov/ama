#include "codegen/internal.h"

bool codegen_emit_body(NativeScaffold *scaffold, FILE *output, const ScaffoldType *unused) {
    (void)unused;
    if (fprintf(output, "/* Included by the single launcher execution function. */\n"
            "AmaWorld ama_world = {.types = ama_project_registry,\n"
            "    .type_count = sizeof(ama_project_registry) / sizeof(ama_project_registry[0]),\n"
            "    .allocator = source_system_allocator()};\n"
            "AmaProgram ama_program = {.world=&ama_world, .types=ama_project_program_types,\n"
            "    .type_count=sizeof(ama_project_program_types)/sizeof(ama_project_program_types[0]),\n"
            "    .constants=ama_project_constants, .constant_count=%zu};\n"
            "AmaHandle ama_root = {0};\n"
            "AmaStatus ama_status = ama_project_prepare_types(&ama_world);\n"
            "if (ama_status.code == AMA_OK) { ama_status = ama_program_start(&ama_program, UINT64_C(%zu), &ama_root); }\n"
            "if (ama_status.code != AMA_OK) { goto ama_project_finish; }\n",
            scaffold->frontend->semantic.constant_count, codegen_type_id(scaffold, scaffold->selected)) < 0) { return false; }
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        for (const ControlNode *node = type->plan->controls; node != NULL; node = node->allocated_next) {
            if (fprintf(output, "if (0) goto ama_label_%zu;\n", node->selector) < 0) { return false; }
        }
    }
    if (fprintf(output, "goto ama_label_%zu;\n", scaffold->selected->plan->entry->selector) < 0) { return false; }
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        for (const ControlNode *node = type->plan->controls; node != NULL; node = node->allocated_next) {
            if (fprintf(output, "ama_label_%zu:;\n", node->selector) < 0) { return false; }
            if (node->operation != CONTROL_COMPLETE) {
                if (fputs("ama_status = ama_status_make(AMA_INVALID_STATE, \"Ama execution wiring is not complete\", 36);\n", output) == EOF) { return false; }
            }
            if (fputs("goto ama_project_finish;\n", output) == EOF) { return false; }
        }
    }
    return fputs("ama_project_finish:;\n"
        "if (ama_status.code != AMA_OK) { fprintf(stderr, \"Ama runtime failure: %u\\n\", (unsigned)ama_status.code); }\n"
        "ama_status = ama_program_stop(&ama_program, ama_root, ama_status);\n"
        "return ama_status.code == AMA_OK ? EXIT_SUCCESS : EXIT_FAILURE;\n", output) != EOF;
}

bool codegen_emit_launcher(NativeScaffold *scaffold, FILE *output, const ScaffoldType *unused) {
    (void)scaffold;
    (void)unused;
    return fputs("#include \"project.h\"\n\nint main(void) {\n    #include \"project.c\"\n}\n", output) != EOF;
}
