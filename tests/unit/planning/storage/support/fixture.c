/** @file
 * @brief Build complete semantic projects while exercising storage independently of later execution lowering.
 */
static const char storage_main[] =
    "@=\nshared\n.value\n |1\n@!\nitems[]\n.value\n |2\nsource\n.value\nproducer\n.producer\nsaved&\n.value\nself&\n.main\n &\n"
    "items[item&]\n{\n source\n {\n  producer !\n   child !\n    > source\n }\n items[item&]\n {\n  %\n }\n}\n"
    "items[item&]\n{\n}\nitems[saved&]\n{\n}\n";
/** Load exact self/call/traversal identities; storage planning never opens/compiles native C bodies. */
static void prepare_storage(const char *text) {
    prepare_types();
    file("generated/program/producer", "@>\nchild\n.worker\n@!\n");
    file("generated/program/worker", "@>\ncondition\n.value\n@!\n");
    file("generated/program/value.h", "#error Storage planning must not compile native headers\n");
    SemanticStatus status = resolve_text(text);
    assert(status == SEMANTIC_OK);
    status = semantic_relations(&semantic);
    assert(status == SEMANTIC_OK);
    status = semantic_constants(&semantic);
    assert(status == SEMANTIC_OK);
    program_plan_init(&plan, &semantic, sources.allocator);
}
/** Inspect one exact lexical frame ordinal in the main module. */
static StorageFrame *frame_at(size_t slot) {
    for (StorageFrame *frame = plan.entry->frames; frame != NULL; frame = frame->next) {
        if (frame->slot == slot) { return frame; }
    }
    assert(false);
    return NULL;
}
