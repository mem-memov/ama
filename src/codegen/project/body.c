#include "codegen/internal.h"

bool codegen_emit_body(NativeScaffold *scaffold, FILE *output, const ScaffoldType *unused) {
    (void)unused;
    if (fprintf(output, "/* Included by the single launcher execution function. */\n"
            "AmaWorld ama_world = {.types = ama_project_registry,\n"
            "    .type_count = sizeof(ama_project_registry) / sizeof(ama_project_registry[0]),\n"
            "    .allocator = source_system_allocator()};\n"
            "AmaProgram ama_program = {.world=&ama_world, .types=ama_project_program_types,\n"
            "    .type_count=sizeof(ama_project_program_types)/sizeof(ama_project_program_types[0]),\n"
            "    .constants=ama_project_constants, .constant_count=%zu,\n"
            "    .locations=ama_project_locations, .location_count=sizeof(ama_project_locations)/sizeof(ama_project_locations[0])};\n"
            "ama_world.report=ama_program_report; ama_world.report_context=&ama_program;\n"
            "AmaHandle ama_root = {0}, ama_current = {0};\n"
            "AmaPoolCell *ama_cell=NULL; uint64_t ama_return=0;\n"
            "AmaStatus ama_status = ama_project_prepare_types(&ama_world);\n"
            "if (ama_status.code == AMA_OK) { ama_status = ama_program_start(&ama_program, UINT64_C(%zu), &ama_root); }\n"
            "if (ama_status.code != AMA_OK) { goto ama_project_finish; }\n"
            "ama_current=ama_root; ama_program.current=ama_current; ama_status=ama_program_cell(&ama_program,ama_current,&ama_cell);\n"
            "if(ama_status.code!=AMA_OK) goto ama_project_finish;\n"
            "ama_cell->metadata.service.activity=AMA_INSTANCE_RUNNING;\n"
            "ama_cell->metadata.service.return_link=(AmaReturnLink){.attached=true,.host=true};\n",
            scaffold->frontend->semantic.constant_count, codegen_type_id(scaffold, scaffold->selected)) < 0) { return false; }
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
            "ama_status=ama_status_make(AMA_INVALID_STATE,\"Unhandled root yield\",20); goto ama_project_finish;\n", output) == EOF) { return false; }
    if (fputs("ama_project_return: __attribute__((unused));\nswitch(ama_return) {\n", output) == EOF) { return false; }
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        for (const ControlNode *node = type->plan->controls; node != NULL; node = node->allocated_next) {
            if (node->operation == CONTROL_SET_RETURN && fprintf(output, "case UINT64_C(%zu): goto ama_label_%zu;\n", node->resume->selector, node->resume->selector) < 0) { return false; }
        }
    }
    if (fputs("default: ama_status=ama_status_make(AMA_INVALID_STATE,NULL,0); goto ama_project_finish;\n}\n", output) == EOF) { return false; }
    return fputs("ama_project_finish:;\n"
        "if (ama_status.code != AMA_OK && !ama_program.failure_reported) { ama_program_report(&ama_program,ama_status); }\n"
        "ama_status = ama_program_stop(&ama_program, ama_root, ama_status);\n"
        "return ama_status.code == AMA_OK ? EXIT_SUCCESS : EXIT_FAILURE;\n", output) != EOF;
}

bool codegen_emit_launcher(NativeScaffold *scaffold, FILE *output, const ScaffoldType *unused) {
    (void)scaffold;
    (void)unused;
    return fputs("#include \"project.h\"\n\nint main(void) {\n    #include \"project.c\"\n}\n", output) != EOF;
}
