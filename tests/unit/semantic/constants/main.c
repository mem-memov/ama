#include "../typed-common.c"

static SemanticStatus prepare_constant_plan(void) {
    prepare_types();
    file("generated/program/worker", "@=\nfirst\n.value\n |+\nsecond\n.value\n |-\n");
    SemanticStatus status = resolve_text("@=\nfirst\n.value\n .worker.second\nsecond\n.value\n first\n"
        "@!\nreceiver\n.value\n.worker\n first >> receiver\n", (SemanticLibraries){0});
    assert(status == SEMANTIC_OK);
    status = semantic_relations(&semantic);
    assert(status == SEMANTIC_OK);
    return semantic_constants(&semantic);
}

static void it_plans_shared_initialization_once_and_cleanup_in_exact_reverse_order(void) {
    SemanticStatus status = prepare_constant_plan();
    assert(status == SEMANTIC_OK && semantic.constants_valid && semantic.constant_count == 4);
    SemanticConstantPlan *entry = semantic.constant_first;
    for (size_t ordinal = 0; ordinal < 4; ordinal++) {
        assert(entry != NULL && entry->ordinal == ordinal && entry->symbol->shared == entry);
        assert(entry->symbol->role == SYMBOL_CONSTANT);
        if (ordinal < 2) {
            assert(strcmp(entry->symbol->owner->module->source->logical_path, ".worker") == 0);
        } else {
            assert(entry->symbol->owner == semantic.first);
        }
        if (entry->next != NULL) {
            assert(entry->next->previous == entry);
        }
        entry = entry->next;
    }
    assert(entry == NULL && semantic.constant_first->previous == NULL);
    entry = semantic.constant_last;
    for (size_t completed = 4; completed > 0; completed--) {
        assert(entry->ordinal == completed - 1);
        entry = entry->previous;
    }
    assert(entry == NULL);
    SemanticStatement *read = semantic.first->last_statement;
    assert(read->syntax->as.call.mode == AST_CONSTANT_READ && read->access == NULL);
    assert(read->bindings->field->shared == semantic.constant_first);
    assert(!read->namespace_type->executable);
    assert(layout_text_is(semantic.constant_first->initializers->source->syntax->payload, "+"));
    destroy_semantic();
}

static void it_uses_path_order_for_ready_constants_and_excludes_unused_namespaces(void) {
    prepare_types();
    file("generated/program/worker", "@=\nfirst\n.value\n |worker\n");
    file("generated/program/unused", "@=\nvalue\n.value\n |unused\n");
    SemanticStatus status = resolve_text("@=\nfirst\n.value\n |main\n@!\nreceiver\n.value\n.worker\n >> receiver\n",
        (SemanticLibraries){0});
    assert(status == SEMANTIC_OK);
    status = semantic_relations(&semantic);
    assert(status == SEMANTIC_OK);
    status = semantic_constants(&semantic);
    assert(status == SEMANTIC_OK && semantic.constant_count == 2);
    assert(semantic.constant_first->symbol->owner == semantic.first);
    assert(semantic_find_type(&semantic, ".unused") == NULL);
    destroy_semantic();
}

static void it_releases_a_partially_built_shared_storage_plan(void) {
    SemanticStatus status = prepare_constant_plan();
    assert(status == SEMANTIC_OK);
    size_t count = semantic.constant_count;
    destroy_semantic();
    for (size_t failure = 1; failure <= count; failure++) {
        prepare_types();
        file("generated/program/worker", "@=\nfirst\n.value\n |+\nsecond\n.value\n |-\n");
        status = resolve_text("@=\nfirst\n.value\n .worker.second\nsecond\n.value\n first\n@!\n",
            (SemanticLibraries){0});
        assert(status == SEMANTIC_OK);
        status = semantic_relations(&semantic);
        assert(status == SEMANTIC_OK);
        failure_at = allocation_count + failure;
        status = semantic_constants(&semantic);
        assert(status == SEMANTIC_OUT_OF_MEMORY && !semantic.constants_valid);
        destroy_semantic();
    }
}

void run(void) {
    it_plans_shared_initialization_once_and_cleanup_in_exact_reverse_order();
    it_uses_path_order_for_ready_constants_and_excludes_unused_namespaces();
    it_releases_a_partially_built_shared_storage_plan();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
