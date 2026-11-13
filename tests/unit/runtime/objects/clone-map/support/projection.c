/** @file
 * @brief Define an independent three-entry full-triple map and compare every scalar reference and native/control value.
 */
/** Map only exact members of the known source tree; stale generations and external targets are preserved verbatim. */
static AmaHandle expected_mapped(AmaHandle original, const AmaHandle copies[3]) {
    for (size_t index = 0; index < 3; index++) {
        if (ama_handle_equal(original, source_nodes[index])) { return copies[index]; }
    }
    return original;
}
/** A completed ordinary copy is fresh/available, owns independent native resources and has no saved position or transient execution connection. */
static void assert_scalar_copy(AmaHandle copy) {
    RootValue *original = object_payload(source_root), *result = object_payload(copy);
    AmaHandle copies[] = {copy, result->left, result->right};
    for (size_t index = 0; index < 3; index++) {
        for (size_t other = 0; other < 3; other++) {
            assert(!ama_handle_equal(copies[index], source_nodes[other]));
            if (index != other) { assert(!ama_handle_equal(copies[index], copies[other])); }
        }
        AmaObjectType *type = ama_world_type(&world, copies[index].type_id);
        const NativeValue *from = (const void *)((const unsigned char *)object_payload(source_nodes[index]) + type->native_offset);
        const NativeValue *to = (const void *)((const unsigned char *)object_payload(copies[index]) + type->native_offset);
        assert(from->resource != to->resource && from->number == to->number && to->address == to);
        const AmaServiceState *service = &mapped_cell(copies[index])->metadata.service;
        assert(service->activity == AMA_INSTANCE_READY && !service->resume.valid && service->resume.selector == 0);
        assert(!service->return_link.attached && ama_handle_is_empty(service->outgoing));
        assert(service->current_operation == 0 && service->last_refusal.code == AMA_OK);
    }
    assert(ama_handle_equal(result->self, expected_mapped(original->self, copies)));
    assert(ama_handle_equal(result->selected, expected_mapped(original->selected, copies)));
    for (size_t index = 1; index < 3; index++) {
        const LeafValue *from = object_payload(source_nodes[index]);
        const LeafValue *to = object_payload(copies[index]);
        assert(ama_handle_equal(to->peer, expected_mapped(from->peer, copies)));
        assert(ama_handle_equal(to->root, expected_mapped(from->root, copies)));
        assert(ama_handle_equal(mapped_cell(copies[index])->owner, copy));
    }
    assert(ama_handle_is_empty(mapped_cell(copy)->owner) && clone_resolutions == 0);
}
