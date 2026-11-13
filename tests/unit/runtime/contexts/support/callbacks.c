/**
 * @file
 * @brief Probe checked identity retirement and registry pinning from a real cleanup callback.
 *
 * Borrow the enclosing counted fixture; the callback owns no persistent view or context token.
 */
#ifndef AMA_CONTEXT_CALLBACK_TEST_C
#define AMA_CONTEXT_CALLBACK_TEST_C
/** Cleanup callback observations borrow their registry only through the active lifecycle pin. */
typedef struct {
    uint64_t retiring;
    uint64_t independent;
    size_t observed;
} ContextCleanupProbe;

/** Check retirement and recursive lifecycle admission while a native destructor reports failure.
 * No registry metadata gate may span this callback; the independent context remains accessible.
 */
static void context_cleanup_report(void *argument, AmaStatus failure) {
    ContextCleanupProbe *probe = argument;
    report_status(NULL, failure);
    probe->observed++;
    AmaContextLease lease = {0};
    AmaStatus status = ama_context_enter(&registry, probe->retiring, &lease);
    assert(status.code == AMA_STALE_REFERENCE);
    status = ama_context_registry_close(&registry);
    assert(status.code == AMA_BUSY);
    status = ama_context_registry_destroy(&registry);
    assert(status.code == AMA_BUSY && registry.initialized);
    (void)enter_context(probe->independent, &lease);
    leave_context(&lease);
}


#endif
