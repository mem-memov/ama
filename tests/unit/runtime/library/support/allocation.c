/** @file
 * @brief Fail every lifecycle allocation and prove unpublished-prefix cleanup.
 */
/** Measure one operation, then fail each allocation ordinal without guessing its count.
 * Context factories publish nothing on failure; admitted root and deletion failures
 * consume context resources and preserve a disposable diagnostic record.
 */
static void allocation_case(unsigned operation, size_t offset, size_t *count) {
    setup_library();
    AmaLibraryContext context = {0};
    AmaLibraryInstance instance = {0};
    if (operation != 0) { context = library_context(); }
    if (operation == 2) { instance = library_root(context,1); }
    size_t before = allocation_count;
    if (offset != 0) { failure_at = before + offset; }
    AmaLibraryResult result = operation == 0 ? ama_library_context_create(&service,&context)
        : operation == 1 ? ama_library_instance_create(&service,context,1,&instance)
        : ama_library_instance_dispose(&service,context,&instance);
    if (offset == 0) {
        assert(result.code == AMA_LIBRARY_OK);
        *count = allocation_count - before;
    } else {
        assert(result.code == AMA_LIBRARY_TECHNICAL_FAILURE && result.status.code == AMA_OUT_OF_MEMORY);
        if (operation == 0) { assert(context.identity == 0); }
        else { assert(instance.identity == 0); }
    }
    failure_at = 0;
    if (context.identity != 0) {
        result = ama_library_context_dispose(&service,&context);
        assert(result.code == (offset != 0 && operation != 0 ? AMA_LIBRARY_TECHNICAL_FAILURE : AMA_LIBRARY_OK));
        assert(context.identity == 0);
    }
    finish_library();
}
/** Every measured allocation failure has exact publication and ownership assertions. */
static void it_unwinds_every_lifecycle_allocation_failure(void) {
    for (unsigned operation = 0; operation != 3; operation++) {
        size_t count = 0;
        allocation_case(operation,0,&count);
        assert(count != 0);
        for (size_t offset = 1; offset <= count; offset++) { allocation_case(operation,offset,&count); }
    }
}
/** Issue the last representable local ID once; exhaustion must never recycle identity one. */
static void it_never_wraps_instance_control_generation(void) {
    setup_library();
    AmaLibraryContext context = library_context();
    AmaLibraryAccess access = {0};
    AmaLibraryResult result = ama_library_enter(&service,context,&access);
    assert(result.code == AMA_LIBRARY_OK);
    access.record->next_instance = UINT64_MAX;
    ama_library_leave(&access);
    AmaLibraryInstance last = library_root(context,2);
    assert(last.identity == UINT64_MAX);
    AmaLibraryInstance output = {0};
    result = ama_library_instance_create(&service,context,2,&output);
    assert(result.code == AMA_LIBRARY_TECHNICAL_FAILURE && result.status.code == AMA_LIMIT_EXCEEDED);
    assert(output.identity == 0 && destroyed == constructed);
    result = ama_library_context_dispose(&service,&context);
    assert(result.code == AMA_LIBRARY_TECHNICAL_FAILURE);
    finish_library();
}
