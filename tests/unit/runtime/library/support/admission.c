/** @file
 * @brief Exercise native reentry, true overlapping admission and numeric-error provenance.
 */
static AmaLibraryContext callback_context;
static size_t callback_count;
/** Reentry from a native constructor must reject before constructing any additional root. */
static AmaStatus reentrant_initialize(void *address) {
    AmaLibraryInstance output = {0};
    AmaLibraryResult result = ama_library_instance_create(&service,callback_context,2,&output);
    assert(result.code == AMA_LIBRARY_BUSY && output.identity == 0);
    AmaLibraryContext original = callback_context;
    result = ama_library_context_dispose(&service,&callback_context);
    assert(result.code == AMA_LIBRARY_BUSY && callback_context.identity == original.identity);
    AmaStatus diagnostic = ama_status_success();
    result = ama_library_diagnostic(&service,callback_context,&diagnostic);
    assert(result.code == AMA_LIBRARY_BUSY && diagnostic.code == AMA_OK);
    callback_count++;
    return initialize_payload(address);
}
/** A native Busy status after destructor admission is a technical failure, never a refusal. */
static AmaStatus busy_destroy(void *address) {
    AmaStatus status = destroy_payload(address);
    assert(status.code == AMA_OK);
    return ama_status_make(AMA_BUSY,"native cleanup failure",22);
}
/** Reentry has no lifecycle effects; accepted native Busy consumes resources exactly once. */
static void it_preserves_admission_provenance_during_native_callbacks(void) {
    setup_library();
    context_types[1].hooks.initialize = reentrant_initialize;
    context_types[1].hooks.destroy = busy_destroy;
    callback_context = library_context(); callback_count = 0;
    AmaLibraryInstance instance = library_root(callback_context,1);
    assert(callback_count == 1 && constructed == 4);
    AmaLibraryResult result = ama_library_instance_dispose(&service,callback_context,&instance);
    assert(result.code == AMA_LIBRARY_TECHNICAL_FAILURE && result.status.code == AMA_BUSY && instance.identity == 0);
    AmaStatus diagnostic = ama_status_success();
    result = ama_library_diagnostic(&service,callback_context,&diagnostic);
    assert(result.code == AMA_LIBRARY_OK && diagnostic.code == AMA_BUSY);
    assert(diagnostic.message_length == 22 && memcmp(diagnostic.message,"native cleanup failure",22) == 0);
    result = ama_library_context_dispose(&service,&callback_context);
    assert(result.code == AMA_LIBRARY_TECHNICAL_FAILURE && callback_context.identity == 0);
    assert(destroyed == constructed && memcmp(diagnostic.message,"native cleanup failure",22) == 0);
    finish_library();
}
/** Another C thread sees the same gate even when requesting a different exported root. */
static int competing_factory(void *argument) {
    AmaLibraryContext context = *(AmaLibraryContext *)argument;
    AmaLibraryInstance output = {0};
    AmaLibraryResult result = ama_library_instance_create(&service,context,2,&output);
    assert(result.code == AMA_LIBRARY_BUSY && output.identity == 0);
    return 0;
}
/** A held lease deterministically overlaps a real other thread without races in test counters. */
static void it_rejects_a_simultaneous_context_executor(void) {
    setup_library();
    AmaLibraryContext context = library_context();
    AmaLibraryAccess access = {0};
    AmaLibraryResult result = ama_library_enter(&service,context,&access);
    assert(result.code == AMA_LIBRARY_OK);
    thrd_t thread;
    int status = thrd_create(&thread,competing_factory,&context); assert(status == thrd_success);
    int returned = -1; status = thrd_join(thread,&returned); assert(status == thrd_success && returned == 0);
    ama_library_leave(&access);
    result = ama_library_context_dispose(&service,&context); assert(result.code == AMA_LIBRARY_OK);
    finish_library();
}
/** A refused core retirement must preserve the public identity and live metadata mapping. */
static void it_preserves_context_when_retirement_is_not_admitted(void) {
    setup_library();
    AmaLibraryContext context = library_context(), original = context;
    service.contexts.pending = SIZE_MAX;
    AmaLibraryResult result = ama_library_context_dispose(&service,&context);
    assert(result.code == AMA_LIBRARY_REFUSED && context.identity == original.identity);
    service.contexts.pending = 0;
    (void)library_root(context,2);
    result = ama_library_context_dispose(&service,&context); assert(result.code == AMA_LIBRARY_OK);
    finish_library();
}
