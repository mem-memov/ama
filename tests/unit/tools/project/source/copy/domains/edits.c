/** @file
 * @brief Reject copied comment edits even after ordinary compilation and graph/site checks pass.
 * @see [T276 checkpoint](../../../../../../../docs/implementation/source-operations.md#t276-remaining-implementation-steps-after-t283)
 */
/** A real applied edit may escape its authority without changing dependencies.
 * Cover comments, storage suffixes, both operand insertion boundaries, and legal
 * interior/prefix path edits. Each image independently compiles and passes sites. */
static void domain_check_main_edit_eligibility(DomainFixture *fixture, const SourceOperationPlanInputs *baseline) {
    const DeliverySourceDefinition *main = fixture->analysis.original.response.definitions;
    while (main && strcmp(main->identity, "/vendor/pkg/v1.0.0/src/main")) { main = main->next; }
    assert(main && main->dependencies && !main->dependencies->next && !main->dependencies->implicit);
    SourceSpan operand = main->dependencies->origin;
    assert(operand.start >= 2 && operand.end > operand.start + 2);
    assert(!memcmp(source_bytes(operand.source) + operand.start, "./", 2));
    const struct { const char *identity; size_t start, end; const char *bytes; size_t length; bool allowed; } variants[] = {
        {"/vendor/pkg/v1.0.0/src/helper", 5, 6, "S", 1, false},
        {main->identity, operand.start, operand.start, "# injected\n", 11, false},
        {main->identity, operand.end, operand.end, "\n# injected", 11, false},
        {main->identity, operand.start + 2, operand.start + 2, "./", 2, true},
        {main->identity, operand.start, operand.start + 2, "/vendor/pkg/v1.0.0/src/", sizeof("/vendor/pkg/v1.0.0/src/")-1, true},
        {main->identity, operand.start - 2, operand.start - 1, NULL, 0, false}
    };
    assert(source_bytes(operand.source)[operand.start - 2] == '&');
    for (size_t variant = 0; variant < sizeof(variants)/sizeof(*variants); variant++) {
        const SourceCloneMapping *copy = baseline->mains->first;
        while (copy && strcmp(copy->original->logical_path, variants[variant].identity)) { copy = copy->next; }
        assert(copy);
        const SourceGraphPreparedText *original = baseline->texts;
        while (original && strcmp(source_path(original->edits->result), copy->path)) { original = original->next; }
        assert(original && !original->edits->first);
        if (!variant) { assert(source_bytes(original->edits->original)[3] == '#' && source_bytes(original->edits->original)[5] == 's'); }
        SourceTextEdits edits; source_text_edits_init(&edits, sources.allocator, original->edits->original);
        SourceSpan span = {.source = edits.original, .start = variants[variant].start, .end = variants[variant].end};
        assert(source_text_edits_add(&edits, span, (const unsigned char *)variants[variant].bytes, variants[variant].length) == SOURCE_EDITS_OK);
        const SourceText *text = NULL;
        assert(source_text_edits_apply(&edits, &sources, copy->path, &text) == SOURCE_EDITS_OK);
        SourceGraphPreparedText *prepared = source_arena_allocate(&fixture->arena, fixture->edit_count * sizeof(*prepared)); assert(prepared);
        const SourceGraphPreparedText *changed = NULL; size_t index = 0;
        for (const SourceGraphPreparedText *row = baseline->texts; row; row = row->next, index++) {
            assert(index < fixture->edit_count);
            prepared[index] = (SourceGraphPreparedText){.edits = row == original ? &edits : row->edits,
                .next = index + 1 < fixture->edit_count ? &prepared[index + 1] : NULL};
            if (row == original) { changed = &prepared[index]; }
        }
        assert(index == fixture->edit_count && changed);
        SourceOperationPlanInputs inputs = *baseline; inputs.texts = prepared;
        SourceOperationPlan image; source_operation_plan_init(&image, sources.allocator);
        assert(source_operation_plan_build(&image, &inputs) == SOURCE_OPERATION_PLAN_OK);
        SourceCompilerClient client; source_compiler_client_init(&client, sources.allocator);
        domain_compile(&client, &image.capture.view, "generated/domain-main-edit.json");
        const char *const *removed = inputs.removal ? inputs.removal->identities : NULL;
        size_t removed_count = inputs.removal ? inputs.removal->identity_count : 0;
        SourceRegistryRetirement retired;
        assert(source_copy_registry_retirement(&fixture->arena, &fixture->original, &image.capture,
            removed, removed_count, &retired) == SOURCE_REGISTRY_OK);
        SourceSitesIssue sites;
        assert(source_graph_sites(fixture->analysis.original.response.definitions, client.response.definitions,
            inputs.mains, removed, removed_count, retired, prepared, &sites) == SOURCE_SITES_OK);
        SourceCopyProofIssue issue;
        SourceCopyProofStatus status = source_operation_plan_prove(&image, &client, &issue);
        assert(status == (variants[variant].allowed ? SOURCE_COPY_PROOF_OK : SOURCE_COPY_PROOF_EDITS));
        if (!variants[variant].allowed) { assert(issue.prepared == changed && issue.edit == edits.first && !strcmp(issue.path, copy->path)); }
        source_compiler_client_destroy(&client); source_operation_plan_destroy(&image);
        source_text_edits_destroy(&edits);
    }
}
