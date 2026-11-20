/** @file
 * @brief Verify actual immutable construction reads, lexical-self exclusions and inferred default fields using loaded Ama projects.
 */
/** Each contained self initializer creates a mutable identity even when the outer constant has no explicit initializer. */
static void it_rejects_self_handles_inside_fresh_shared_descendants(void) {
    const char *workers[] = {
        "@<\nself&\n.worker\n &\n@!\n",
        "@>\nself&\n.worker\n &\n@!\n",
        "@!\nself&\n.worker\n &\n",
        "@!\nselves&[]\n.worker\n &\n"
    };
    for (size_t index = 0; index < sizeof(workers) / sizeof(*workers); index++) {
        prepare_types();
        file("generated/program/worker", workers[index]);
        SemanticStatus status = resolve_text("@=\nconstant\n.worker\n@!\n");
        assert(status == SEMANTIC_OK);
        status = semantic_relations(&semantic);
        assert(status == SEMANTIC_INVALID && !semantic.relations_valid);
        assert(strstr(semantic.error.message, "mutable-object reference") != NULL);
        assert(semantic.error.primary.span.end - semantic.error.primary.span.start == 1);
        assert(source_bytes(semantic.error.primary.span.source)[semantic.error.primary.span.start] == '&');
        assert(diagnostic_valid(&semantic.error));
        destroy_semantic();
    }
    /* Empty reference cells contain no target; executing the worker body is not part of shared construction. */
    prepare_types();
    file("generated/program/worker", "@!\nself&\n.worker\nself&\n &\n");
    SemanticStatus status = resolve_text("@=\nconstant\n.worker\n@!\n");
    assert(status == SEMANTIC_OK);
    status = semantic_relations(&semantic);
    assert(status == SEMANTIC_OK);
    destroy_semantic();
}

/** An inferred executable source is never a declaration initializer or a fresh shared-value prerequisite. */
static void it_constructs_inferred_fields_from_defaults_without_reading_their_body_sources(void) {
    prepare_types();
    file("generated/program/worker", "@!\ncached\n .main.future\n");
    SemanticStatus status = resolve_text("@=\nconstant\n.worker\nfuture\n.value\n |9\n@!\n");
    assert(status == SEMANTIC_OK);
    SemanticType *worker = semantic_find_type(&semantic, ".worker");
    assert(worker != NULL && worker->first->inferred && worker->first->initializers == NULL);
    status = semantic_relations(&semantic);
    assert(status == SEMANTIC_OK);
    status = semantic_constants(&semantic);
    assert(status == SEMANTIC_OK && semantic.constant_count == 2);
    assert(semantic.constant_first->symbol->type == worker && semantic.constant_first->next == semantic.constant_last);
    assert(semantic.constants.order->first == NULL);
    destroy_semantic();
}

/** A named shared root and each index contribute exactly one actual read, independently of declaration-order edges. */
static void it_preserves_each_indexed_shared_read_without_duplicate_root_edges(void) {
    prepare_types();
    SemanticStatus status = resolve_text("@=\nindex\n.value\n |0\nitems[]\n.value\n |2\n"
        "selected\n.value\n items[index]\nqualified\n.value\n .main.selected\n@!\n");
    assert(status == SEMANTIC_OK);
    status = semantic_relations(&semantic);
    assert(status == SEMANTIC_OK);
    GraphVertex *selected = semantic.constants.order->order_next->order_next;
    assert(selected != NULL && ((SemanticSymbol *)selected->value)->role_ordinal == 2);
    size_t edges = 0;
    size_t index_reads = 0;
    size_t item_reads = 0;
    for (GraphEdge *edge = selected->first; edge != NULL; edge = edge->next) {
        SemanticSymbol *target = (SemanticSymbol *)edge->target->value;
        assert(target->owner == semantic_entry());
        if (target->role_ordinal == 0) { index_reads++; }
        if (target->role_ordinal == 1) { item_reads++; }
        edges++;
    }
    assert(edges == 3 && index_reads == 1 && item_reads == 2);
    GraphVertex *qualified = selected->order_next;
    assert(qualified->first != NULL && qualified->first->next != NULL && qualified->first->next->next == NULL);
    destroy_semantic();
}

/** Transitive fresh construction, including initialized owning lists, reads actual field defaults; cloning reads the supplied constant only. */
static void it_follows_fresh_descendants_but_does_not_reconstruct_cloned_values(void) {
    prepare_types();
    file("generated/program/palette", "@=\nvalue\n.value\n |8\n");
    file("generated/program/worker", "@!\nfield\n.value\n .palette.value\n");
    SemanticStatus status = resolve_text("@=\noriginal\n.worker\ncopy\n.worker\n original\n@!\n");
    assert(status == SEMANTIC_OK);
    status = semantic_relations(&semantic);
    assert(status == SEMANTIC_OK);
    GraphVertex *palette = semantic.constants.order;
    GraphVertex *original = palette->order_next;
    GraphVertex *copy = original->order_next;
    assert(copy != NULL && copy->order_next == NULL);
    assert(original->first != NULL && original->first->target == palette && original->first->next == NULL);
    for (GraphEdge *edge = copy->first; edge != NULL; edge = edge->next) { assert(edge->target == original); }
    destroy_semantic();
    /* Literal element construction initializes its own fields as well as its native payload. */
    native_fixture("worker", "+ initialize\n- destroy\n| literal\n= clone\n");
    file("generated/program/worker", "@<\nfield\n.value\n .palette.value\n@C\n");
    status = resolve_text("@=\nitems[]\n.worker\n |first\n |second\n@!\n");
    assert(status == SEMANTIC_OK);
    status = semantic_relations(&semantic);
    assert(status == SEMANTIC_OK);
    GraphVertex *owner = semantic.constants.order->order_next;
    assert(owner->first != NULL && owner->first->target == semantic.constants.order);
    destroy_semantic();
}
