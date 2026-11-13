/** @file
 * @brief Check independent expected facts of one complete modern project and exact staged/composite equivalence.
 */
/** Complete analysis exposes typed current constructs while keeping native C and pending executable-frame support outside this boundary. */
static void it_composes_every_current_construct_with_exact_source_and_role_metadata(void) {
    prepare_composition();
    load_composition(composed_main);
    semantic_program_init(&semantic, &project, sources.allocator);
    SemanticStatus status = semantic_analyze(&semantic);
    if (status != SEMANTIC_OK) { fprintf(stderr, "Composed semantics: %s\n", semantic.error.message); }
    assert(status == SEMANTIC_OK && semantic.constants_valid);
    assert(semantic.names_valid && semantic.native_valid && semantic.inference_valid && semantic.resolved && semantic.relations_valid);
    assert(semantic.constant_count == 2 && strcmp(semantic.constant_first->symbol->owner->module->source->logical_path, ".palette") == 0);
    assert(semantic.list_integer != NULL && semantic.list_integer->requires_size_bridge);
    size_t traversals = 0, headers = 0, methods = 0, continued = 0, locals = 0;
    for (const SemanticSymbol *symbol = semantic.first->locals; symbol != NULL; symbol = symbol->next) { assert(symbol->local); locals++; }
    for (const SemanticStatement *statement = semantic.first->statements; statement != NULL; statement = statement->next) {
        if (statement->syntax->kind == AST_FOREACH) { assert(statement->traversal != NULL); traversals++; }
        if (statement->syntax->kind == AST_LOOP) { assert(statement->loop_root != statement->condition_owner); headers++; }
        if (statement->syntax->kind == AST_CALL && statement->operation_kind == SEMANTIC_LIST_METHOD_OPERATION) { methods++; }
        if (statement->syntax->kind == AST_ASSIGNMENT && statement->transfer == TRANSFER_CONTINUATION) { continued++; }
    }
    assert(traversals == 1 && headers == 1 && methods == 2 && continued == 1 && locals == 1);
    bool payload = false, inferred_saved = false;
    for (const SemanticSymbol *symbol = semantic.first->first; symbol != NULL; symbol = symbol->next) {
        if (layout_text_is(symbol->name, "source")) {
            payload = layout_text_is(symbol->initializers->source->syntax->payload, "data % ref& # bytes  ");
        }
        if (layout_text_is(symbol->name, "saved")) { inferred_saved = symbol->inferred && symbol->reference && !symbol->local; }
    }
    assert(payload && inferred_saved);
    destroy_semantic();
}
/** Exact projection equality uses shared original AST identities and complete spans/types, not hashes or struct padding. */
static void it_matches_composite_and_explicit_stages_without_pointer_or_hash_identity_guesses(void) {
    prepare_composition();
    const char *projects[] = {composed_main, "@!\n"};
    for (size_t shape = 0; shape < sizeof(projects) / sizeof(*projects); shape++) {
        load_composition(projects[shape]);
        SemanticProgram left, right;
        semantic_program_init(&left, &project, sources.allocator);
        semantic_program_init(&right, &project, sources.allocator);
        SemanticStatus a = semantic_analyze(&left);
        SemanticStatus b = analyze_staged(&right);
        assert(a == SEMANTIC_OK && b == SEMANTIC_OK);
        compare_programs(&left, &right);
        semantic_program_destroy(&right);
        semantic_program_destroy(&left);
        destroy_project();
    }
}
