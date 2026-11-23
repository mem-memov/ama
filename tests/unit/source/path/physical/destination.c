/** @file
 * @brief Distinguish future destination proposals from existing-source observations without creating paths.
 * @see [T287 entry review](../../../../../docs/implementation/source-operations.md#t287-entry-review-after-accepted-t286)
 */
/** Only an actually existing prefix supplies the POSIX oracle; the expected
 * missing suffix is explicit fixture data, not another run of the resolver. */
static void it_normalizes_missing_destination_tails_without_relaxing_source_traversal(void) {
    prepare(); char *base = realpath("generated/tree", NULL); assert(base);
    char input[] = "generated//tree/directory/../new//version/src/main";
    const char *expected = source_arena_join(&arena, base, "/", "new/version/src/main"); assert(expected);
    SourcePhysicalPath result; SourcePhysicalIssue issue;
    assert(source_physical_destination(&arena, input, SOURCE_PHYSICAL_ENDPOINT, &result, &issue) == SOURCE_PHYSICAL_OK);
    assert(!result.present && !result.directory_required && !result.metadata.st_ino && !strcmp(result.path, expected));
    input[0] = '!'; assert(!strcmp(result.path, expected));
    struct stat metadata; assert(lstat(result.path, &metadata) != 0 && errno == ENOENT);
    assert(lstat("generated/tree/new", &metadata) != 0 && errno == ENOENT);
    result = (SourcePhysicalPath){.path = "unchanged", .present = true};
    assert(source_physical_resolve(&arena, "generated/tree/new/version/src/main", SOURCE_PHYSICAL_ENDPOINT,
        true, &result, &issue) == SOURCE_PHYSICAL_MISSING && !strcmp(result.path, "unchanged") && result.present);
    assert(source_physical_destination(&arena, "generated/tree/new/version/src/", SOURCE_PHYSICAL_ENDPOINT, &result, &issue) == SOURCE_PHYSICAL_OK);
    assert(!result.present && result.directory_required && !strcmp(result.path,
        source_arena_join(&arena, base, "/", "new/version/src")));
    assert(source_physical_destination(&arena, "generated/tree/new/version/src", SOURCE_PHYSICAL_DIRECTORY, &result, &issue) == SOURCE_PHYSICAL_OK
        && !result.present && result.directory_required);
    char *regular = realpath("generated/tree/regular", NULL); assert(regular);
    assert(source_physical_destination(&arena, "generated/tree/directory/../regular", SOURCE_PHYSICAL_ENDPOINT, &result, &issue) == SOURCE_PHYSICAL_OK
        && result.present && !strcmp(result.path, regular));
    assert(lstat(regular, &metadata) == 0 && result.metadata.st_dev == metadata.st_dev && result.metadata.st_ino == metadata.st_ino);
    const char *raw = source_arena_join(&arena, base, "/", "\xff zone/new/child"); assert(raw);
    assert(source_physical_destination(&arena, raw, SOURCE_PHYSICAL_ENDPOINT, &result, &issue) == SOURCE_PHYSICAL_OK
        && !result.present && !strcmp(result.path, raw));
    free(base); free(regular); finish();
}
/** A future proposal cannot erase a missing traversal, replace a regular parent,
 * follow either a live/broken alias or reinterpret a special inode. */
static void it_refuses_unverified_destination_navigation_aliases_and_wrong_kinds(void) {
    prepare();
    const char *missing[] = {"generated/tree/new/../regular", "generated/tree/new/./child", "generated/tree/new/level/../child"};
    for (size_t i = 0; i < sizeof(missing)/sizeof(*missing); i++) {
        SourcePhysicalPath result = {.path = "unchanged", .present = true}; SourcePhysicalIssue issue;
        assert(source_physical_destination(&arena, missing[i], SOURCE_PHYSICAL_ENDPOINT, &result, &issue) == SOURCE_PHYSICAL_MISSING);
        assert(!strcmp(result.path, "unchanged") && result.present && issue.status == SOURCE_PHYSICAL_MISSING && issue.probe == PATH_MISSING);
        assert(issue.path && strlen(issue.path) >= 4 && !strcmp(issue.path + strlen(issue.path) - 4, "/new"));
    }
    const char *unsupported[] = {"generated/tree/link/new/child", "generated/tree/broken/new/child", "generated/tree/link/../new/child",
        "generated/tree/pipe/new/child", "generated/tree/regular/new/child", "generated/tree/regular/"};
    for (size_t i = 0; i < sizeof(unsupported)/sizeof(*unsupported); i++) {
        SourcePhysicalPath result = {.path = "unchanged"}; SourcePhysicalIssue issue;
        assert(source_physical_destination(&arena, unsupported[i], SOURCE_PHYSICAL_ENDPOINT, &result, &issue) == SOURCE_PHYSICAL_UNSUPPORTED);
        assert(!strcmp(result.path, "unchanged") && issue.path);
    }
    finish();
}
/** Exhaust both verified-prefix and future-buffer allocations; failed calls keep
 * their previous output and return owned, precise failure evidence. */
static void it_retires_destination_proposal_failures_and_preserves_invalid_outputs(void) {
    size_t total = 0;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        prepare(); SourcePhysicalPath result = {.path = "unchanged", .present = true}; SourcePhysicalIssue issue;
        size_t before = allocation_count; if (failure) { failure_at = before + failure; }
        SourcePhysicalStatus status = source_physical_destination(&arena, "generated/tree/directory/../new/with/several/ancestors",
            SOURCE_PHYSICAL_ENDPOINT, &result, &issue);
        if (!failure) { assert(status == SOURCE_PHYSICAL_OK && !result.present); total = allocation_count - before; assert(total); }
        else { assert(status == SOURCE_PHYSICAL_STORAGE && !strcmp(result.path, "unchanged") && result.present); }
        failure_at = 0; finish();
    }
    prepare(); SourcePhysicalPath result = {.path = "unchanged"}; SourcePhysicalIssue issue;
    assert(source_physical_destination(NULL, "path", SOURCE_PHYSICAL_ENDPOINT, &result, &issue) == SOURCE_PHYSICAL_INVALID);
    assert(source_physical_destination(&arena, NULL, SOURCE_PHYSICAL_ENDPOINT, &result, &issue) == SOURCE_PHYSICAL_INVALID);
    assert(source_physical_destination(&arena, "", SOURCE_PHYSICAL_ENDPOINT, &result, &issue) == SOURCE_PHYSICAL_INVALID);
    assert(source_physical_destination(&arena, ".", (SourcePhysicalKind)99, &result, &issue) == SOURCE_PHYSICAL_INVALID);
    assert(source_physical_destination(&arena, ".", SOURCE_PHYSICAL_ENDPOINT, NULL, &issue) == SOURCE_PHYSICAL_INVALID);
    assert(source_physical_destination(&arena, ".", SOURCE_PHYSICAL_ENDPOINT, &result, NULL) == SOURCE_PHYSICAL_INVALID);
    assert(!strcmp(result.path, "unchanged")); finish();
}
