/** @file
 * @brief Inject every owner-codec and owner/role/registry allocation refusal with exact teardown accounting.
 * @see [T188 contract](../../../../../../docs/implementation/specialized-execution.md)
 */

/** Each successful-trace owner allocation may fail; partial descriptors remain arena-owned and receivers remain unchanged. */
static void it_unwinds_every_owner_key_allocation_without_publishing_partial_keys(void) {
    setup_owners();
    size_t baseline = live_allocations, total = 0;
    for (size_t cut = 0; cut <= total; cut++) {
        SourceArena temporary;
        source_arena_init(&temporary, sources.allocator);
        QualifiedIdentity output = {.category = IDENTITY_LABEL, .module_path = "/test/fixture/v1.0.0/src/marker"};
        size_t before = allocation_count;
        if (cut) { failure_at = before + cut; }
        IdentityStatus status = semantic_type_key(&frontend.semantic, owners[0], &temporary, &output);
        if (!cut) { assert(status == IDENTITY_OK); total = allocation_count - before; assert(total); }
        else {
            assert(status == IDENTITY_OUT_OF_MEMORY);
            assert(output.category == IDENTITY_LABEL && strcmp(output.module_path, "/test/fixture/v1.0.0/src/marker") == 0 && !output.members && !output.member_count);
        }
        failure_at = 0;
        source_arena_destroy(&temporary);
        assert(live_allocations == baseline);
    }
    printf("owner key allocation refusal positions: %zu\n", total);
    destroy_owners();
}

/** Refuse every no-role and role-appended registration allocation; repeated calls preserve the first diagnostic without another allocation. */
static void it_unwinds_every_scaffold_owner_role_registration_allocation_and_preserves_first_failure(void) {
    setup_owners();
    native_scaffold_destroy(&scaffold);
    size_t baseline = live_allocations;
    for (size_t mode = 0; mode < 2; mode++) {
        size_t total = 0;
        for (size_t cut = 0; cut <= total; cut++) {
            native_scaffold_init(&scaffold, &frontend, sources.allocator);
            size_t before = allocation_count;
            if (cut) { failure_at = before + cut; }
            const IdentityName *name = scaffold_identity(&scaffold, owners[0], IDENTITY_TYPE, mode ? "read" : NULL, (SourceSpan){0});
            if (!cut) { assert(name); total = allocation_count - before; assert(total); }
            else {
                assert(!name && scaffold.error.code == DIAGNOSTIC_OUT_OF_MEMORY);
                Diagnostic original = scaffold.error;
                size_t failed_count = allocation_count;
                assert(!scaffold_identity(&scaffold, owners[0], IDENTITY_TYPE, "write", (SourceSpan){0}));
                assert(scaffold.error.message == original.message && allocation_count == failed_count);
            }
            failure_at = 0;
            native_scaffold_destroy(&scaffold);
            assert(live_allocations == baseline);
        }
        printf("scaffold owner role %zu allocation refusal positions: %zu\n", mode, total);
    }
    destroy_frontend();
}
