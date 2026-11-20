/** @file
 * @brief Verify lexical self typing, forbidden destinations and allocation-failure cleanup.
 */
/** Self sources have lexical scalar-reference identity, independent of declaration role or destination. */
static void it_types_lexical_self_in_initializers_assignments_and_explicit_inputs(void) {
    prepare_types();
    file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@<\ncaller&\n/test/fixture/v1.0.0/src/main\nself&\n/test/fixture/v1.0.0/src/worker\n &\n@>\nresult&\n/test/fixture/v1.0.0/src/worker\n &\n@!\n");
    const char *text = "@<\nparameter&\n/test/fixture/v1.0.0/src/main\n &\n@>\nresult&\n/test/fixture/v1.0.0/src/main\n &\n@!\n"
        "references&[]\n/test/fixture/v1.0.0/src/main\n &\nworker\n/test/fixture/v1.0.0/src/worker\nworker !\n caller& < &\n"
        "fresh&\n &\nresult&\n &\n";
    SemanticStatus status = resolve_text(text);
    if (status != SEMANTIC_OK) { fprintf(stderr, "Self typing: %s\n", semantic.error.message); }
    assert(status == SEMANTIC_OK && semantic.resolved);
    size_t initializers = 0;
    size_t assignments = 0;
    size_t inputs = 0;
    for (SemanticType *type = semantic.first; type != NULL; type = type->next) {
        for (SemanticSymbol *symbol = type->first; symbol != NULL; symbol = symbol->next) {
            for (SemanticInitializer *value = symbol->initializers; value != NULL; value = value->next) {
                SemanticSource *source = value->source;
                assert(source->kind == AST_SELF_REFERENCE_SOURCE && source->type == type);
                assert(source->reference && !source->list && source->access == NULL && source->constant == NULL);
                assert(source->declaration == NULL && value->transfer == TRANSFER_HANDLE);
                assert(source->site.end - source->site.start == 1);
                initializers++;
            }
        }
        for (SemanticStatement *statement = type->statements; statement != NULL; statement = statement->next) {
            if (statement->source != NULL) {
                assert(statement->source->kind == AST_SELF_REFERENCE_SOURCE && statement->source->type == type);
                assert(statement->transfer == TRANSFER_HANDLE);
                assignments++;
            }
            for (SemanticBinding *binding = statement->bindings; binding != NULL; binding = binding->next) {
                assert(binding->source->type == type && binding->source->type != binding->field->owner);
                assert(binding->source->reference && !binding->source->list && binding->transfer == TRANSFER_HANDLE);
                inputs++;
            }
        }
    }
    assert(initializers == 5 && assignments == 2 && inputs == 1);
    destroy_semantic();
}

/** No shared instance, owned destination, whole-list transfer or nominal mismatch can reinterpret standalone self. */
static void it_rejects_incompatible_self_destinations_without_adopting_their_types(void) {
    prepare_types();
    const char *invalid[] = {
        "@=\nshared&\n/test/fixture/v1.0.0/src/main\n &\n@!\n",
        "@!\nowned\n/test/fixture/v1.0.0/src/main\n &\n",
        "@!\nreference&\n/test/fixture/v1.0.0/src/value\n &\n",
        "@!\nreference&\n/test/fixture/v1.0.0/src/main\nreference\n &\n",
        "@!\nreferences&[]\n/test/fixture/v1.0.0/src/main\nreferences&[]\n &\n",
        "@!\nnew value\n &\n",
        "@!\nnew values&[]\n &\n"
    };
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        expect_resolved(invalid[index], false);
    }
}

/** The self branch and all surrounding typed records unwind at every counted allocation boundary. */
static void it_unwinds_every_allocation_while_typing_self_sources(void) {
    prepare_types();
    const char *text = "@>\nself&\n/test/fixture/v1.0.0/src/main\n &\n@!\nfresh&\n &\n";
    SemanticStatus status = resolve_text(text);
    assert(status == SEMANTIC_OK);
    size_t total = allocation_count;
    destroy_semantic();
    for (size_t failure = 1; failure <= total; failure++) {
        file("generated/program/packages/test/fixture/v1.0.0/src/main", text);
        setup_semantic();
        status = load_semantic("generated/program");
        assert(status == SEMANTIC_OK);
        status = semantic_native(&semantic);
        assert(status == SEMANTIC_OK);
        if (failure <= allocation_count) { destroy_semantic(); continue; }
        failure_at = failure;
        status = semantic_resolve(&semantic);
        assert(status == SEMANTIC_OUT_OF_MEMORY && !semantic.resolved && diagnostic_valid(&semantic.error));
        destroy_semantic();
    }
}
