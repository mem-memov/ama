/** @file
 * @brief Verify actual immutable construction reads, lexical-self exclusions and inferred default fields using loaded Ama projects.
 */
/** Each contained self initializer creates a mutable identity even when the outer constant has no explicit initializer. */
static void it_rejects_self_handles_inside_fresh_shared_descendants(void) {
    const char *workers[] = {
        "@<\nself&\n/test/fixture/v1.0.0/src/worker\n &\n@!\n",
        "@>\nself&\n/test/fixture/v1.0.0/src/worker\n &\n@!\n",
        "@!\nself&\n/test/fixture/v1.0.0/src/worker\n &\n",
        "@!\nselves&[]\n/test/fixture/v1.0.0/src/worker\n &\n"
    };
    for (size_t index = 0; index < sizeof(workers) / sizeof(*workers); index++) {
        prepare_types();
        file("generated/program/packages/test/fixture/v1.0.0/src/worker", workers[index]);
        SemanticStatus status = resolve_text("@=\nconstant\n/test/fixture/v1.0.0/src/worker\n@!\n");
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
    file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@!\nself&\n/test/fixture/v1.0.0/src/worker\nself&\n &\n");
    SemanticStatus status = resolve_text("@=\nconstant\n/test/fixture/v1.0.0/src/worker\n@!\n");
    assert(status == SEMANTIC_OK);
    status = semantic_relations(&semantic);
    assert(status == SEMANTIC_OK);
    destroy_semantic();
}

/** An inferred executable source is never a declaration initializer or a fresh shared-value prerequisite. */
static void it_constructs_inferred_fields_from_defaults_without_reading_their_body_sources(void) {
    prepare_types();
    file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@!\ncached\n /test/fixture/v1.0.0/src/main/future\n");
    SemanticStatus status = resolve_text("@=\nconstant\n/test/fixture/v1.0.0/src/worker\nfuture\n/test/fixture/v1.0.0/src/value\n |9\n@!\n");
    assert(status == SEMANTIC_OK);
    SemanticType *worker = semantic_find_type(&semantic, "/test/fixture/v1.0.0/src/worker");
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
    SemanticStatus status = resolve_text("@=\nindex\n/test/fixture/v1.0.0/src/value\n |0\nitems[]\n/test/fixture/v1.0.0/src/value\n |2\n"
        "selected\n/test/fixture/v1.0.0/src/value\n items[index]\nqualified\n/test/fixture/v1.0.0/src/value\n /test/fixture/v1.0.0/src/main/selected\n@!\n");
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
    file("generated/program/packages/test/fixture/v1.0.0/src/palette", "@=\nvalue\n/test/fixture/v1.0.0/src/value\n |8\n");
    file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@!\nfield\n/test/fixture/v1.0.0/src/value\n /test/fixture/v1.0.0/src/palette/value\n");
    SemanticStatus status = resolve_text("@=\noriginal\n/test/fixture/v1.0.0/src/worker\ncopy\n/test/fixture/v1.0.0/src/worker\n original\n@!\n");
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
    file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@<\nfield\n/test/fixture/v1.0.0/src/value\n /test/fixture/v1.0.0/src/palette/value\n@C\n");
    status = resolve_text("@=\nitems[]\n/test/fixture/v1.0.0/src/worker\n |first\n |second\n@!\n");
    assert(status == SEMANTIC_OK);
    status = semantic_relations(&semantic);
    assert(status == SEMANTIC_OK);
    GraphVertex *owner = semantic.constants.order->order_next;
    assert(owner->first != NULL && owner->first->target == semantic.constants.order);
    destroy_semantic();
}
