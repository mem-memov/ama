/** @file
 * @brief Verify full structural owner shape, exact role separation and unchanged concrete native namespaces.
 * @see [T188 contract](../../../../../../docs/implementation/specialized-execution.md)
 */

/** Complete owner descriptors survive temporary teardown through registered encodings; native leaves retain their original namespace category/key. */
static void it_preserves_full_owner_role_keys_and_concrete_native_namespace_identity(void) {
    setup_owners();
    SourceArena temporary;
    source_arena_init(&temporary, sources.allocator);
    QualifiedIdentity keys[2];
    const IdentityName *views[2];
    for (size_t index = 0; index < 2; index++) {
        assert(semantic_type_key(&frontend.semantic, owners[index], &temporary, &keys[index]) == IDENTITY_OK);
        assert(keys[index].category == IDENTITY_TYPE && keys[index].member_count == 2);
        assert(strcmp(keys[index].module_path, "/test/fixture/v1.0.0/src/container") == 0 && strcmp(keys[index].members[0], "ama-static-key-v1") == 0);
        views[index] = scaffold_identity(&scaffold, owners[index], IDENTITY_TYPE, "read", (SourceSpan){0});
        assert(views[index] && views[index] == scaffold_identity(&scaffold, owners[index], IDENTITY_TYPE, "read", (SourceSpan){0}));
        const IdentityName *write = scaffold_identity(&scaffold, owners[index], IDENTITY_TYPE, "write", (SourceSpan){0});
        assert(write && strcmp(views[index]->internal_name, write->internal_name) != 0);
    }
    assert(strcmp(keys[0].members[1], keys[1].members[1]) != 0);
    source_arena_destroy(&temporary);
    assert(strcmp(views[0]->internal_name, views[1]->internal_name) != 0);
    const SemanticType *native = owners[0]->first->type;
    assert(native->module->syntax.sections.kind == MODULE_NATIVE);
    const IdentityName *prefix = scaffold_identity(&scaffold, native, IDENTITY_NATIVE_NAMESPACE, NULL, (SourceSpan){0});
    QualifiedIdentity expected = {.category = IDENTITY_NATIVE_NAMESPACE, .module_path = native->module->source->logical_path};
    const IdentityName *original;
    assert(prefix && identity_register(&scaffold.identities, expected, &original) == IDENTITY_OK && prefix == original);
    destroy_owners();
}

/** Foreign copied metadata, incomplete certificates and forbidden destination aliasing must leave output untouched and allocate nothing. */
static void it_refuses_foreign_incomplete_and_aliased_owner_keys_without_changing_output(void) {
    setup_owners();
    SourceArena temporary;
    source_arena_init(&temporary, sources.allocator);
    SemanticType copied = *owners[0];
    for (size_t shape = 0; shape < 8; shape++) {
        QualifiedIdentity output = {.category = IDENTITY_LABEL, .module_path = "/test/fixture/v1.0.0/src/marker"};
        size_t before = allocation_count;
        IdentityStatus status;
        switch (shape) {
            case 0: status = semantic_type_key(NULL, owners[0], &temporary, &output); break;
            case 1: status = semantic_type_key(&frontend.semantic, NULL, &temporary, &output); break;
            case 2: status = semantic_type_key(&frontend.semantic, &copied, &temporary, &output); break;
            case 3: status = semantic_type_key(&frontend.semantic, owners[0], NULL, &output); break;
            case 4: status = semantic_type_key(&frontend.semantic, owners[0], &temporary, NULL); break;
            case 5:
                frontend.specializations.closure.closed = false;
                status = semantic_type_key(&frontend.semantic, owners[0], &temporary, &output);
                frontend.specializations.closure.closed = true;
                break;
            case 6:
                frontend.semantic.relations_valid = false;
                status = semantic_type_key(&frontend.semantic, owners[0], &temporary, &output);
                frontend.semantic.relations_valid = true;
                break;
            default: status = semantic_type_key(&frontend.semantic, owners[0], &frontend.specializations.terms.arena, &output); break;
        }
        assert(status == IDENTITY_INVALID && allocation_count == before);
        assert(output.category == IDENTITY_LABEL && strcmp(output.module_path, "/test/fixture/v1.0.0/src/marker") == 0 && !output.members && !output.member_count);
    }
    source_arena_destroy(&temporary);
    destroy_owners();
}

/** Captured ordinary types cannot masquerade as concrete native companion namespaces; failure leaves no registered partial name. */
static void it_refuses_native_namespace_category_for_captured_ordinary_owners(void) {
    setup_owners();
    assert(!scaffold_identity(&scaffold, owners[0], IDENTITY_NATIVE_NAMESPACE, NULL, (SourceSpan){0}));
    assert(scaffold.error.code == DIAGNOSTIC_TYPE_MISMATCH && scaffold.identities.first == NULL);
    assert(frontend.specializations.closure.closed && frontend.semantic.constants_valid);
    destroy_owners();
}
