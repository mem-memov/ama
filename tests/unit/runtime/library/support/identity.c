/** @file
 * @brief Reject foreign/stale host controls before any local pool identity can be selected.
 */
/** Existing core references and new component controls retain disjoint, explicit authorities. */
static void it_keeps_kernel_references_and_component_controls_distinct(void) {
    setup_library();
    uint64_t old = create_context();
    AmaContextLease lease = {0};
    AmaContextView view = enter_context(old, &lease);
    AmaContextReference root = create_root(&lease, view);
    RootValue *value = root_value(&lease, root);
    value->native.number = 73;
    leave_context(&lease);
    AmaLibraryInstance output = {0};
    AmaLibraryResult result = ama_library_instance_create(&service, (AmaLibraryContext){(uintptr_t)&service,old}, 1, &output);
    assert(result.code == AMA_LIBRARY_INVALID_REQUEST && output.identity == 0);
    AmaStatus status = ama_context_dispose(&registry, old); assert(status.code == AMA_OK);
    finish_library();
}
/** Equal local type/slot/generation and root-ID numbers never bypass originating context checks. */
static void it_rejects_cross_context_foreign_and_obsolete_controls(void) {
    setup_library();
    AmaLibraryContext first = library_context(), second = library_context();
    AmaLibraryInstance a = library_root(first,1), b = library_root(second,1);
    assert(a.identity == b.identity);
    AmaLibraryAccess left = {0}, right = {0};
    AmaLibraryResult result = ama_library_enter(&service, first, &left); assert(result.code == AMA_LIBRARY_OK);
    result = ama_library_enter(&service, second, &right); assert(result.code == AMA_LIBRARY_OK);
    assert(ama_handle_equal(left.record->roots->handle, right.record->roots->handle));
    ama_library_leave(&right); ama_library_leave(&left);
    AmaLibraryInstance copied = a;
    result = ama_library_instance_dispose(&service, second, &copied);
    assert(result.code == AMA_LIBRARY_INVALID_REQUEST && copied.identity == a.identity);
    AmaLibraryContext foreign = first; foreign.owner++;
    AmaLibraryInstance output = {0};
    result = ama_library_instance_create(&service, foreign, 1, &output);
    assert(result.code == AMA_LIBRARY_INVALID_REQUEST && output.identity == 0);
    result = ama_library_instance_dispose(&service, first, &a); assert(result.code == AMA_LIBRARY_OK && a.identity == 0);
    AmaLibraryInstance replacement = library_root(first,1);
    assert(replacement.identity != copied.identity);
    result = ama_library_instance_dispose(&service, first, &copied); assert(result.code == AMA_LIBRARY_INVALID_REQUEST);
    AmaLibraryContext stale = first;
    result = ama_library_context_dispose(&service, &first); assert(result.code == AMA_LIBRARY_OK && first.identity == 0);
    AmaLibraryContext recreated = library_context(); assert(recreated.identity != stale.identity);
    result = ama_library_instance_create(&service, stale, 1, &output); assert(result.code == AMA_LIBRARY_INVALID_REQUEST);
    result = ama_library_context_dispose(&service, &second); assert(result.code == AMA_LIBRARY_OK);
    result = ama_library_context_dispose(&service, &recreated); assert(result.code == AMA_LIBRARY_OK);
    finish_library();
}
