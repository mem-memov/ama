/** @file
 * @brief Build complete semantic projects while exercising storage independently of later execution lowering.
 */
static const char storage_main[] =
    "@=\nshared\n/test/fixture/v1.0.0/src/value\n |1\n@!\nitems[]\n/test/fixture/v1.0.0/src/value\n |2\nsource\n/test/fixture/v1.0.0/src/value\nproducer\n/test/fixture/v1.0.0/src/producer\nsaved&\n/test/fixture/v1.0.0/src/value\nself&\n/test/fixture/v1.0.0/src/main\n &\n"
    "items[item&]\n{\n source\n {\n  producer !\n   child !\n    > source\n }\n items[item&]\n {\n  %\n }\n}\n"
    "items[item&]\n{\n}\nitems[saved&]\n{\n}\n";
/** Load exact self/call/traversal identities; storage planning never opens/compiles native C bodies. */
static void prepare_storage(const char *text) {
    prepare_types();
    file("generated/program/packages/test/fixture/v1.0.0/src/producer", "@>\nchild\n/test/fixture/v1.0.0/src/worker\n@!\n");
    file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@>\ncondition\n/test/fixture/v1.0.0/src/value\n@!\n");
    file("generated/program/packages/test/fixture/v1.0.0/src/value.h", "#error Storage planning must not compile native headers\n");
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
