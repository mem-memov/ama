/** @file
 * @brief Observe explicit construction, retained positions and reverse successful-root cleanup.
 */
/** Context construction initializes constants only; root creation and disposal never execute statements. */
static void it_constructs_only_explicit_roots_and_disposes_yielded_state_without_resume(void) {
    setup_library();
    assert(constructed == 0);
    AmaLibraryContext context = library_context(); assert(constructed == 1);
    AmaLibraryInstance first = library_root(context,1); assert(constructed == 4);
    AmaLibraryAccess access = {0};
    AmaLibraryResult result = ama_library_enter(&service, context, &access); assert(result.code == AMA_LIBRARY_OK);
    AmaHandle local = access.record->roots->handle;
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_program_cell(access.view.program, local, &cell); assert(status.code == AMA_OK);
    cell->metadata.service.activity = AMA_INSTANCE_YIELDED;
    cell->metadata.service.resume = (AmaResumePosition){true,17};
    RootValue *payload = ama_pool_payload(&ama_world_type(access.view.world,2)->pool, cell);
    payload->native.number = 99;
    ama_library_leave(&access);
    result = ama_library_enter(&service, context, &access); assert(result.code == AMA_LIBRARY_OK);
    status = ama_program_cell(access.view.program, local, &cell); assert(status.code == AMA_OK);
    assert(cell->metadata.service.resume.valid && cell->metadata.service.resume.selector == 17);
    assert(((RootValue *)ama_pool_payload(&ama_world_type(access.view.world,2)->pool,cell))->native.number == 99);
    ama_library_leave(&access);
    AmaLibraryInstance second = library_root(context,2); (void)second;
    result = ama_library_context_dispose(&service, &context); assert(result.code == AMA_LIBRARY_OK);
    assert(first.identity != 0 && destroyed == constructed);
    const size_t expected[] = {5,99,3,2,37};
    assert(destroyed == sizeof(expected)/sizeof(*expected) && memcmp(expected,destruction_order,sizeof(expected)) == 0);
    finish_library();
}
/** A held context lease rejects creation/disposal before changing any published identity. */
static void it_refuses_busy_and_invalid_factory_requests_without_effects(void) {
    setup_library();
    AmaLibraryContext context = library_context(); AmaLibraryInstance root = library_root(context,1);
    AmaLibraryAccess access = {0};
    AmaLibraryResult result = ama_library_enter(&service, context, &access); assert(result.code == AMA_LIBRARY_OK);
    size_t before = constructed;
    AmaLibraryInstance empty = {0};
    result = ama_library_instance_create(&service,context,1,&empty); assert(result.code == AMA_LIBRARY_BUSY && empty.identity == 0);
    AmaLibraryContext copied = context;
    result = ama_library_context_dispose(&service,&copied); assert(result.code == AMA_LIBRARY_BUSY && copied.identity == context.identity);
    result = ama_library_instance_dispose(&service,context,&root); assert(result.code == AMA_LIBRARY_BUSY && root.identity != 0);
    assert(constructed == before && destroyed == 0);
    ama_library_leave(&access);
    result = ama_library_instance_create(&service,context,999,&empty); assert(result.code == AMA_LIBRARY_INVALID_REQUEST);
    result = ama_library_instance_create(&service,context,1,&root); assert(result.code == AMA_LIBRARY_INVALID_REQUEST);
    result = ama_library_context_dispose(&service,&context); assert(result.code == AMA_LIBRARY_OK);
    finish_library();
}
/** Reject null/nonempty destinations and live service shutdown before lifecycle effects. */
static void it_checks_destination_shapes_before_lifecycle_effects(void) {
    setup_library();
    AmaLibraryResult result = ama_library_context_create(&service,NULL);
    assert(result.code == AMA_LIBRARY_INVALID_REQUEST && constructed == 0);
    AmaLibraryContext context = library_context(), original = context;
    result = ama_library_context_create(&service,&context);
    assert(result.code == AMA_LIBRARY_INVALID_REQUEST && context.identity == original.identity && constructed == 1);
    result = ama_library_instance_create(&service,context,1,NULL);
    assert(result.code == AMA_LIBRARY_INVALID_REQUEST && constructed == 1);
    result = ama_library_context_dispose(&service,NULL); assert(result.code == AMA_LIBRARY_INVALID_REQUEST);
    result = ama_library_instance_dispose(&service,context,NULL); assert(result.code == AMA_LIBRARY_INVALID_REQUEST);
    result = ama_library_diagnostic(&service,context,NULL); assert(result.code == AMA_LIBRARY_INVALID_REQUEST);
    AmaStatus diagnostic = ama_status_make(AMA_IO_FAILURE,NULL,0);
    result = ama_library_diagnostic(&service,context,&diagnostic);
    assert(result.code == AMA_LIBRARY_INVALID_REQUEST && diagnostic.code == AMA_IO_FAILURE);
    AmaStatus status = ama_library_service_destroy(&service); assert(status.code == AMA_BUSY);
    assert(constructed == 1 && destroyed == 0);
    result = ama_library_context_dispose(&service,&context); assert(result.code == AMA_LIBRARY_OK);
    finish_library();
}
