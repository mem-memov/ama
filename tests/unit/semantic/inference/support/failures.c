/** @file
 * @brief Refuse every measured guarded source-preparation and ordinary native/certificate allocation.
 * @see [T193 retirement contract](../../../../../docs/implementation/static-relocation.md)
 */
/** Current source/Type preparation, not obsolete event replay, owns inferred-field allocation. */
static void it_unwinds_every_inference_allocation_failure(void) {
    prepare_inference();
    const char *text = "@!\nworker\n.worker\nitems[]\n.value\nworker !\n > first\ncopy\n first\nitems[item&]\n{\n escaped&\n  item&\n}\nitems[]\n append !\n  > appended&\n length !\n  > length\n";
    file("generated/program/main",text);
    setup_semantic();
    ModuleRoot roots[] = {{"generated/program",NULL,0,MODULE_ROOT_PROJECT},
        {"generated/standard",NULL,0,MODULE_ROOT_STANDARD}};
    assert(project_load(&project,roots,2,".main") == PROJECT_OK);
    semantic_program_init(&semantic,&project,sources.allocator);
    size_t before = allocation_count;
    assert(semantic_fixture_names(&semantic) == SEMANTIC_OK);
    assert(semantic_native(&semantic) == SEMANTIC_OK);
    assert(semantic_infer(&semantic) == SEMANTIC_OK);
    size_t total = allocation_count-before;
    assert(total > 0);
    destroy_semantic();
    for (size_t failure = 1; failure <= total; failure++) {
        setup_semantic();
        assert(project_load(&project,roots,2,".main") == PROJECT_OK);
        semantic_program_init(&semantic,&project,sources.allocator);
        failure_at = allocation_count+failure;
        SemanticStatus status = semantic_fixture_names(&semantic);
        if (status == SEMANTIC_OK) { status = semantic_native(&semantic); }
        if (status == SEMANTIC_OK) { status = semantic_infer(&semantic); }
        assert(status == SEMANTIC_OUT_OF_MEMORY && !semantic.inference_valid);
        assert(diagnostic_valid(&semantic.error));
        destroy_semantic();
    }
    fprintf(stderr,"Current source preparation checked %zu allocation refusal points\n",total);
}
