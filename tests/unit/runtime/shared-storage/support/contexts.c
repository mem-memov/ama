/** @file
 * @brief Verify two independent shared startup closures, explicit root copying and root-before-reverse-shared shutdown.
 */
/** Constants initialize once per context outside pools; one context's explicit root/disposal cannot mutate the other's shared values. */
static void it_owns_nested_shared_values_per_context_without_materializing_instance_pools(void) {
    setup_shared();
    uint64_t first = create_context(), second = create_context();
    assert(constructed == 14 && cloned == 10 && shared_literals == 2 && shared_defaults == 2);
    AmaContextLease left = {0}, right = {0};
    AmaContextView a = enter_context(first, &left), b = enter_context(second, &right);
    assert_no_instances(a);
    assert_no_instances(b);
    assert_shared_values(a);
    assert_shared_values(b);
    assert_distinct_shared_storage(a, b);
    uintptr_t expected_a[10], expected_b[7];
    expected_shared_cleanup(a, expected_a + 3);
    expected_shared_cleanup(b, expected_b);
    AmaContextReference root = create_root(&left, a);
    RootValue *value = root_value(&left, root);
    assert(value->native.number == 0);
    expected_a[0] = (uintptr_t)&value->native;
    AmaObjectType *leaf = a.world->types[0];
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_pool_lookup(&leaf->pool, value->right, false, &cell);
    assert(status.code == AMA_OK);
    assert(((const LeafValue *)ama_pool_payload(&leaf->pool, cell))->native.number == 37);
    expected_a[1] = (uintptr_t)((unsigned char *)ama_pool_payload(&leaf->pool, cell) + leaf->native_offset);
    status = ama_pool_lookup(&leaf->pool, value->left, false, &cell);
    assert(status.code == AMA_OK);
    assert(((const LeafValue *)ama_pool_payload(&leaf->pool, cell))->native.number == 37);
    expected_a[2] = (uintptr_t)((unsigned char *)ama_pool_payload(&leaf->pool, cell) + leaf->native_offset);
    assert_no_instances(b);
    leave_context(&left);
    status = ama_context_dispose(&registry, first);
    assert(status.code == AMA_OK && cleanup_count == 10);
    for (size_t index = 0; index < 10; index++) { assert(cleanup_addresses[index] == expected_a[index]); }
    assert_shared_values(b);
    assert_no_instances(b);
    leave_context(&right);
    status = ama_context_dispose(&registry, second);
    assert(status.code == AMA_OK && cleanup_count == 17 && constructed == destroyed);
    for (size_t index = 0; index < 7; index++) { assert(cleanup_addresses[index + 10] == expected_b[index]); }
    finish_contexts();
}
