/** @file
 * @brief Check actual exclusive contention, canonical recovery placement and failed descriptor prefixes.
 */
/** Separate recovery directories do not bypass the shared source/root leases; destruction releases every descriptor. */
static void it_serializes_independent_leases_and_releases_descriptors(void) {
    prepare("lease-contention");
    size_t baseline = descriptors();
    RelocationTransaction first, second;
    application(&first);
    assert(relocation_transaction_lease(&first,path("recovery1")));
    size_t held = descriptors();
    assert(held > baseline);
    assert(relocation_transaction_revalidate(&first));
    application(&second);
    assert(!relocation_transaction_lease(&second,path("recovery2")));
    assert(second.status == RELOCATION_BUSY);
    relocation_transaction_destroy(&second);
    assert(descriptors() == held);
    relocation_transaction_destroy(&first);
    assert(descriptors() == baseline);
    application(&second);
    assert(relocation_transaction_lease(&second,path("recovery2")));
    relocation_transaction_destroy(&second);
    assert(descriptors() == baseline);
    application(&first);
    assert(!relocation_transaction_lease(&first,path("project/packages/test/fixture/v1.0.0/src/old")));
    assert(first.status == RELOCATION_INVALID_PATH);
    relocation_transaction_destroy(&first);
    assert(descriptors() == baseline);
    done();
}

/** Every measured lease and full revalidation allocation can fail without leaking a lock, descriptor or original byte owner. */
static void it_unwinds_every_lease_and_revalidation_allocation(void) {
    prepare("lease-allocations");
    const char *recovery = path("recovery1");
    size_t baseline = live_allocations, descriptor_baseline = descriptors();
    RelocationTransaction transaction;
    application(&transaction);
    allocation_count = 0;
    assert(relocation_transaction_lease(&transaction,recovery));
    size_t lease_allocations = allocation_count;
    allocation_count = 0;
    assert(relocation_transaction_revalidate(&transaction));
    size_t revalidation_allocations = allocation_count;
    relocation_transaction_destroy(&transaction);
    assert(live_allocations == baseline && descriptors() == descriptor_baseline);
    for (size_t ordinal = 1; ordinal <= lease_allocations; ordinal++) {
        application(&transaction);
        allocation_count = 0;
        failure_at = ordinal;
        assert(!relocation_transaction_lease(&transaction,recovery));
        assert(transaction.status == RELOCATION_OUT_OF_MEMORY);
        failure_at = 0;
        relocation_transaction_destroy(&transaction);
        assert(live_allocations == baseline && descriptors() == descriptor_baseline);
    }
    for (size_t ordinal = 1; ordinal <= revalidation_allocations; ordinal++) {
        application(&transaction);
        assert(relocation_transaction_lease(&transaction,recovery));
        allocation_count = 0;
        failure_at = ordinal;
        assert(!relocation_transaction_revalidate(&transaction));
        assert(transaction.status == RELOCATION_OUT_OF_MEMORY);
        failure_at = 0;
        relocation_transaction_destroy(&transaction);
        assert(live_allocations == baseline && descriptors() == descriptor_baseline);
    }
    done();
}
