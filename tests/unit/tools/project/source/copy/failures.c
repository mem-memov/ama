/** @file
 * @brief Fault every owned copy-preparation allocation with an independently healthy original compiler proof.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
/** The copy owner and its children share this fault domain; original providers retain their own allocator. */
typedef struct { size_t attempts, failure, live; } CopyFaults;
/** Fail one exact owner allocation, retaining separate live-block evidence. */
static void *copy_allocate(void *context, size_t length) {
    CopyFaults *faults = context; faults->attempts++;
    if (faults->failure && faults->attempts == faults->failure) { return NULL; }
    void *result = malloc(length); if (result) { faults->live++; } return result;
}
/** Release a successful owner block exactly once. */
static void copy_release(void *context, void *allocation) {
    CopyFaults *faults = context;
    if (allocation) { assert(faults->live); faults->live--; free(allocation); }
}
/** Exercise raw/default, ordinary cyclic dependency and declared-data closure failure boundaries. */
static void it_retires_every_copy_owner_allocation_for_default_dependency_and_declared_data_compositions(void) {
    for (unsigned scenario = 0; scenario < 3; scenario++) {
        size_t count = 0;
        for (size_t failure = 0; !failure || failure <= count; failure++) {
            prepare_profile(true, false, false); select_main();
            const SourceCapturedUnit *main = capture.units, *other = capture.units;
            while (main && strcmp(main->source->logical_path, main_identity)) { main = main->next; }
            while (other && strcmp(other->source->logical_path, "/vendor/pkg/v1.0.0/src/other")) { other = other->next; }
            assert(main && other);
            SourceDeclaredGroup declared = {.requester = main, .version = other->source->version, .files = other->files};
            CopyFaults faults = {.failure = failure}; SourceAllocator allocator = {&faults, copy_allocate, copy_release};
            source_copy_closure_init(&copy, allocator, &capture, &analysis, scenario == 2 ? &declared : NULL);
            SourceCopyClosureStatus status = source_copy_closure_build(&copy, &tree,
                scenario == 1 ? SOURCE_COPY_CLONE_DEPENDENCIES : SOURCE_COPY_KEEP_EXTERNAL);
            if (!failure) { assert(status == SOURCE_COPY_CLOSURE_OK && copy.valid); count = faults.attempts; assert(count); }
            else {
                assert(!copy.valid && (status == SOURCE_COPY_CLOSURE_MEMORY || status == SOURCE_COPY_CLOSURE_EXPANSION));
                if (status == SOURCE_COPY_CLOSURE_EXPANSION) { assert(copy.expansion_status == ACQUISITION_CLOSURE_OUT_OF_MEMORY); }
            }
            finish_copy(); assert(!faults.live);
        }
    }
}
