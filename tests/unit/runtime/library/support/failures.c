/** @file
 * @brief Distinguish admitted technical lifecycle failures from effect-free refusals.
 */
/** Failed startup returns its own cause without a context handle or retained resource prefix. */
static void it_returns_startup_failure_before_context_publication(void) {
    setup_library(); failed_constructor = 1;
    AmaLibraryContext context = {0};
    AmaLibraryResult result = ama_library_context_create(&service,&context);
    assert(result.code == AMA_LIBRARY_TECHNICAL_FAILURE && result.status.code == AMA_INVALID_LITERAL && context.identity == 0);
    assert(destroyed == 0 && reports >= 1);
    finish_library();
}
/** A failed root constructor destroys its prefix and all earlier roots, then permits only cleanup. */
static void it_fails_only_the_admitted_context_and_unwinds_all_live_roots(void) {
    setup_library();
    AmaLibraryContext failed = library_context(), healthy = library_context();
    (void)library_root(failed,1);
    failed_constructor = constructed + 2;
    AmaLibraryInstance output = {0};
    AmaLibraryResult result = ama_library_instance_create(&service,failed,1,&output);
    assert(result.code == AMA_LIBRARY_TECHNICAL_FAILURE && result.status.code == AMA_INVALID_LITERAL && output.identity == 0);
    result = ama_library_instance_create(&service,failed,1,&output); assert(result.code == AMA_LIBRARY_REFUSED);
    (void)library_root(healthy,1);
    result = ama_library_context_dispose(&service,&failed); assert(result.code == AMA_LIBRARY_TECHNICAL_FAILURE && failed.identity == 0);
    result = ama_library_context_dispose(&service,&healthy); assert(result.code == AMA_LIBRARY_OK);
    finish_library();
}
/** Destruction errors consume the target and fail the context rather than becoming busy refusal. */
static void it_consumes_admitted_destructor_failure_exactly_once(void) {
    setup_library();
    AmaLibraryContext context = library_context(); AmaLibraryInstance root = library_root(context,1);
    failed_destructor = 1;
    AmaLibraryResult result = ama_library_instance_dispose(&service,context,&root);
    assert(result.code == AMA_LIBRARY_TECHNICAL_FAILURE && result.status.code == AMA_IO_FAILURE && root.identity == 0);
    size_t before = destroyed; assert(before == constructed);
    result = ama_library_context_dispose(&service,&context); assert(result.code == AMA_LIBRARY_TECHNICAL_FAILURE);
    assert(destroyed == before);
    finish_library();
}
