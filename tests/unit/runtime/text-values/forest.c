/** @file
 * @brief Exercise immutable-share rollback through actual owned-tree clone cleanup.
 */
/** Two separate declared Ama children, not native-hidden instance handles. */
typedef struct { AmaHandle first, second; } TextPair;
/** Adapt the exact typed native default protocol to the runtime callback ABI. */
static AmaStatus text_initialize(void *storage) { return test_text_initialize(storage); }
/** Adapt literal construction without changing byte lengths or statuses. */
static AmaStatus text_literal(void *storage, AmaByteView bytes) { return test_text_from_literal(storage, bytes); }
/** Retain one real native share when the runtime prepares a child clone. */
static AmaStatus text_clone(void *destination, const void *source) { return test_text_clone(destination, source); }
/** Consume only the callback's own successfully constructed share. */
static AmaStatus text_destroy(void *storage) { return test_text_destroy(storage); }

/** A later child cannot acquire the reduced-limit share. Earlier cloned child
 * cleanup must return the source's exact count, not merely preserve its bytes.
 */
static void it_rolls_back_earlier_shares_after_a_later_owned_child_clone_fails(void) {
    setup_test();
    AmaObjectType text_type, pair_type;
    checked_text(ama_object_type_init(&text_type, sources.allocator, 1, sizeof(test_text_State),
        2, UINT64_MAX, NULL, 0, 0, (AmaPayloadHooks){text_initialize, text_literal, text_clone, text_destroy}));
    const AmaObjectField fields[] = {{AMA_FIELD_OWNED, 1, offsetof(TextPair, first)},
        {AMA_FIELD_OWNED, 1, offsetof(TextPair, second)}};
    checked_text(ama_object_type_init(&pair_type, sources.allocator, 2, sizeof(TextPair),
        2, UINT64_MAX, fields, 2, 0, (AmaPayloadHooks){0}));
    AmaObjectType *types[] = {&text_type, &pair_type};
    AmaWorld world = {.types = types, .type_count = 2, .allocator = sources.allocator};
    const AmaRecipe literal = {.constructor = AMA_CONSTRUCT_LITERAL,
        .literal = {(const unsigned char *)"shared", 6}};
    AmaHandle original, pair;
    checked_text(ama_object_create(&world, 1, &literal, &original));
    const AmaRecipe child = {.constructor = AMA_CONSTRUCT_CLONE, .source = original};
    const AmaFieldRecipe initializers[] = {{.count = 1, .children = &child, .supplied = true},
        {.count = 1, .children = &child, .supplied = true}};
    const AmaRecipe recipe = {.fields = initializers, .field_count = 2};
    checked_text(ama_object_create(&world, 2, &recipe, &pair));
    checked_text(ama_object_destroy(&world, original));
    AmaPoolCell *pair_cell = NULL;
    checked_text(ama_pool_lookup(&pair_type.pool, pair, false, &pair_cell));
    TextPair *value = ama_pool_payload(&pair_type.pool, pair_cell);
    AmaPoolCell *first = NULL, *second = NULL;
    checked_text(ama_pool_lookup(&text_type.pool, value->first, false, &first));
    checked_text(ama_pool_lookup(&text_type.pool, value->second, false, &second));
    const test_text_State *left = ama_pool_payload(&text_type.pool, first);
    const test_text_State *right = ama_pool_payload(&text_type.pool, second);
    assert(left->backing == right->backing && left->backing->shares == 2);
    AmaHandle copied = {0};
    AmaStatus status = ama_object_clone(&world, pair, false, &copied);
    assert(status.code == AMA_LIMIT_EXCEEDED && ama_handle_is_empty(copied)
        && left->backing->shares == 2 && left->live && right->live);
    expect_text(left, "shared");
    expect_text(right, "shared");
    checked_text(ama_object_destroy(&world, pair));
    ama_pool_dispose(&pair_type.pool);
    ama_pool_dispose(&text_type.pool);
    destroy_test();
}
