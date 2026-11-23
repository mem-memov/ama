/** @file
 * @brief Distinguish observed regular parents, directory collisions and absent original positions.
 * @see [T290](../../../../../../../docs/implementation/source-operations.md#t290-command-relation-construction-after-t292)
 */
/** Alternative candidate layouts share the same actual original closure/roots.
 * This tests parent placement only, not the request's full raw intent. A retained
 * destination parent can supply a position absent from the source chain; a
 * missing one cannot be invented, and a directory cannot stand in for a main. */
static void it_retains_or_refuses_actual_parent_providers_without_inventing_ancestors(void) {
    seed_fixture_begin(0);
    assert(project_source_preparation_seed(&seed_preparation) == PROJECT_SOURCE_PREPARATION_OK);
    SourceObservedImage observed; source_observed_image_init(&observed, sources.allocator);
    assert(source_observed_image_build(&observed, seed_preparation.original_capture,
        (SourceImageBounds){0}, NULL, 0) == SOURCE_OBSERVATION_OK);
    const char *names[] = {"missing", "existing", "bad"};
    const SourceCopyParentStatus expected[] = {SOURCE_COPY_PARENT_MISSING, SOURCE_COPY_PARENT_OK, SOURCE_COPY_PARENT_KIND};
    const ModuleSource *source = seed_preparation.selected->units[0]->source;
    for (size_t i = 0; i < sizeof(names) / sizeof(*names); i++) {
        SourceCloneMap mains; source_clone_map_init(&mains, sources.allocator, seed_preparation.original_capture);
        const char *stem = source_arena_join(&mains.arena, seed_preparation.root_settings.final[0].path, "/", names[i]); assert(stem);
        const char *path = source_arena_join(&mains.arena, stem, "", "+/copy"); assert(path);
        const char *identity = source_arena_join(&mains.arena, "//", names[i], "/copy"); assert(identity);
        const SourceCloneMapping *row = NULL;
        assert(source_clone_map_add(&mains, source, path, identity, &row) == SOURCE_CLONE_OK);
        SourceCopyParentInputs inputs = {.copy = &seed_preparation.seeds.copy, .mains = &mains, .observed = &observed,
            .roots = seed_preparation.root_settings.final, .root_count = seed_preparation.root_settings.final_count};
        SourceCopyParentIssue issue;
        assert(source_copy_context_place(&inputs, &issue) == expected[i]);
        assert(mains.first && !mains.first->next);
        if (expected[i] == SOURCE_COPY_PARENT_OK) {
            assert(!issue.path && source_clone_map_seal(&mains) == SOURCE_CLONE_OK);
        } else {
            assert(issue.path && !strcmp(issue.path, stem) && !issue.required);
            assert(source_clone_map_seal(&mains) != SOURCE_CLONE_OK);
        }
        source_clone_map_destroy(&mains);
    }
    assert(access("generated/source/existing+", F_OK) != 0 && errno == ENOENT);
    source_observed_image_destroy(&observed); seed_fixture_end(0);
}
