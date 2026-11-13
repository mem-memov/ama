/** @file
 * @brief Fail every inference-owned event, encountered-name, field, scope and operation allocation.
 */
/** Mixed operation/result/local paths keep every partial owner destructible and never publish valid inference on failure. */
static void it_unwinds_every_inference_allocation_failure(void) {
    prepare_inference();
    const char *text = "@!\nworker\n.worker\nitems[]\n.value\nworker !\n > first\ncopy\n first\nitems[item&]\n{\n escaped&\n  item&\n}\nitems[]\n append !\n  > appended&\n length !\n  > length\n";
    valid_inference(text);
    size_t total = allocation_count;
    destroy_semantic();
    for (size_t failure = 1; failure <= total; failure++) {
        setup_semantic();
        ModuleRoot roots[] = {{"generated/program", NULL, 0, MODULE_ROOT_PROJECT},
            {"generated/standard", NULL, 0, MODULE_ROOT_STANDARD}};
        ProjectStatus loaded = project_load(&project, roots, 2, ".main");
        assert(loaded == PROJECT_OK);
        semantic_program_init(&semantic, &project, sources.allocator);
        SemanticStatus status = semantic_names(&semantic);
        assert(status == SEMANTIC_OK);
        status = semantic_native(&semantic);
        assert(status == SEMANTIC_OK);
        if (failure <= allocation_count) { destroy_semantic(); continue; }
        failure_at = failure;
        status = semantic_infer(&semantic);
        assert(status == SEMANTIC_OUT_OF_MEMORY && !semantic.inference_valid);
        assert(diagnostic_valid(&semantic.error));
        destroy_semantic();
    }
}
