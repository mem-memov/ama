/**
 * @file
 * @brief Exercise the internal fixed-library size recipe after owned-field construction.
 *
 * Synthetic nominal descriptors isolate backend mechanics. The separate integer
 * ABI fixture validates the actual declared from_size payload/protocol contract.
 */

/** The single selected size callback follows owned children, without + or literal dispatch. */
static void it_selects_size_construction_once_and_unwinds_its_completed_owned_fields(void) {
    const AmaStatus outcomes[] = {{AMA_OK, NULL, 0}, {AMA_OK, NULL, 1}, {AMA_OUT_OF_MEMORY, NULL, 0}};
    for (size_t index = 0; index < sizeof(outcomes) / sizeof(*outcomes); index++) {
        setup_protocols();
        context_types[1].hooks.initialize = protocol_initialize;
        context_types[1].hooks.literal = protocol_literal;
        protocol_status = outcomes[index];
        uint64_t identity = create_context();
        AmaContextLease lease = {0};
        AmaContextView view = enter_context(identity, &lease);
        const AmaRecipe recipe = {.constructor = AMA_CONSTRUCT_SIZE,
            .from_size = protocol_from_size, .size_value = SIZE_MAX};
        AmaHandle output = {0};
        AmaStatus status = ama_object_create(view.world, 2, &recipe, &output);
        assert(protocol_sizes == 1 && protocol_defaults == 0 && protocol_literals == 0);
        assert(constructed == 4);
        if (index == 0) {
            assert(status.code == AMA_OK && !ama_handle_is_empty(output));
            AmaPoolCell *cell = view.world->last_root;
            assert(cell->metadata.lifetime.constructor == AMA_CONSTRUCT_SIZE);
            AmaContextReference reference = {0};
            status = ama_context_reference(&lease, output, &reference);
            assert(status.code == AMA_OK && root_value(&lease, reference)->native.number == SIZE_MAX);
            leave_context(&lease);
            finish_contexts();
        } else {
            assert(status.code == (index == 1 ? AMA_INVALID_STATE : AMA_OUT_OF_MEMORY));
            assert(ama_handle_is_empty(output) && destroyed == (index == 1 ? 3 : 2));
            assert(protocol_self_releases == (index == 1 ? 0 : 1));
            leave_context(&lease);
            finish_protocol_failure(identity, index == 1 ? AMA_INVALID_STATE : AMA_OUT_OF_MEMORY);
        }
    }
}
