/** @file
 * @brief Exhaust controller metadata allocation refusals independently of healthy provider allocation.
 * @see [T235 contract](../../../../../../../docs/implementation/acquisition-plan.md#t235-structural-source-and-test-delivery)
 */
/** Independent allocator tracks only controller metadata, separating provider I/O
 * and compiler preparation allocations from the finite controller fault domain. */
typedef struct { size_t attempts, failure, live; } PlanFaults;
/** Refuse one exact controller allocation and count retained metadata owners. */
static void *plan_allocate(void *context, size_t size) {
    PlanFaults *faults = context; faults->attempts++;
    if (faults->failure && faults->attempts == faults->failure) { return NULL; }
    void *allocation = malloc(size); if (allocation) { faults->live++; } return allocation;
}
/** Retire exactly one successful controller allocation through its own authority. */
static void plan_release(void *context, void *allocation) {
    PlanFaults *faults = context;
    if (allocation) { assert(faults->live); faults->live--; free(allocation); }
}
/** Exhaust every measured controller allocation in cyclic/promotion and empty-version
 * compositions, while the real provider keeps its independent healthy allocator. */
static void it_retires_every_controller_allocation_failure_in_real_compositions(void) {
    for (size_t scenario = 0; scenario < 2; scenario++) {
        size_t allocations = 0;
        for (size_t failure = 0; !failure || failure <= allocations; failure++) {
            if (scenario) { prepare_empty(); } else { prepare(true, false, false, false); }
            acquisition_closure_destroy(&closure);
            PlanFaults faults = {.failure = failure};
            SourceAllocator allocator = {&faults, plan_allocate, plan_release};
            acquisition_closure_init(&closure, allocator);
            AcquisitionClosureProvider provider = {.expand = scenario ? expand_empty : expand, .admit_test = admit};
            const char *seed = scenario ? origin->version : child_identity;
            AcquisitionClosureStatus status = acquisition_closure_compute(&closure, &seed, 1, &provider);
            if (!failure) { assert(status == ACQUISITION_CLOSURE_OK && closure.valid); allocations = faults.attempts; assert(allocations); }
            else { assert(status == ACQUISITION_CLOSURE_OUT_OF_MEMORY && !closure.valid); }
            finish(); assert(!faults.live);
        }
    }
}
