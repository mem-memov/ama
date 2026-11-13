/**
 * @file
 * @brief Verify malformed destruction status never restores or retries native credits.
 */

/** Raw-success destruction errors become technical while all later resources still release. */
static void it_consumes_native_credits_once_and_preserves_malformed_destroy_as_primary(void) {
    for (size_t roots = 0; roots < 2; roots++) {
        setup_protocols();
        context_types[0].hooks.destroy = roots == 0 ? protocol_destroy : protocol_destroy_io;
        context_types[1].hooks.destroy = protocol_destroy;
        uint64_t identity = create_context();
        if (roots != 0) {
            AmaContextLease lease = {0};
            AmaContextView view = enter_context(identity, &lease);
            (void)create_root(&lease, view);
            leave_context(&lease);
        }
        AmaStatus status = ama_context_dispose(&registry, identity);
        assert(status.code == AMA_INVALID_STATE && destroyed == (roots == 0 ? 1 : 4));
        assert(reports == (roots == 0 ? 1 : 4) && live_allocations == 0);
        size_t before = destroyed;
        status = ama_context_dispose(&registry, identity);
        assert(status.code == AMA_STALE_REFERENCE && destroyed == before);
        finish_contexts();
    }
}
