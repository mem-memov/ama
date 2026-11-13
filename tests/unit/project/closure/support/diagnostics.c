/**
 * @file
 * @brief Compose real catalog and parser failures into source-owned frontend diagnostic records.
 */

/** Exact lexical expectations survive both parser adaptation and the required-source requester chain. */
static void it_preserves_narrow_expected_constructs_and_both_dependency_locations(void) {
    directory("generated/diagnostic");
    file("generated/diagnostic/main", "@!\nchild\n.child\n");
    file("generated/diagnostic/child", "@!\n\tbad\n");
    setup_project();
    ProjectStatus status = load_root("generated/diagnostic");
    assert(status == PROJECT_SYNTAX_ERROR);
    assert(diagnostic_valid(&project.error));
    assert(strcmp(project.error.expected, "ASCII spaces") == 0);
    assert(strcmp(project.error.subject, ".child") == 0);
    assert(strcmp(source_path(project.error.primary.span.source), "generated/diagnostic/child") == 0);
    assert(project.error.primary.span.start == 3 && project.error.primary.span.end == 4);
    assert(project.error.related_count == 1);
    assert(strcmp(project.error.related[0].name, ".main") == 0);
    assert(strcmp(source_path(project.error.related[0].location.span.source), "generated/diagnostic/main") == 0);
    assert(project.error.related[0].location.span.start == 9);
    destroy_project();
}

/** Filesystem acquisition and malformed-byte failures retain their physical paths, fixed codes and unmodified input offsets. */
static void it_composes_real_filesystem_and_unicode_failures_without_losing_source_context(void) {
    setup_project();
    ProjectStatus status = load_root("generated/no such diagnostic root");
    assert(status == PROJECT_CATALOG_ERROR);
    assert(diagnostic_valid(&project.error));
    assert(project.error.code == DIAGNOSTIC_MISSING_MODULE);
    assert(strcmp(project.error.primary.path, "./generated/no such diagnostic root") == 0);
    assert(project.error.cause != NULL && project.error.expected != NULL);
    destroy_project();
    file("generated/diagnostic/child", "@!\n\xed\xa0\x80");
    setup_project();
    status = load_root("generated/diagnostic");
    assert(status == PROJECT_CATALOG_ERROR);
    assert(diagnostic_valid(&project.error));
    assert(project.error.code == DIAGNOSTIC_INVALID_UTF8);
    assert(strcmp(project.error.expected, "well-formed UTF-8") == 0);
    assert(strcmp(source_path(project.error.primary.span.source), "generated/diagnostic/child") == 0);
    assert(project.error.primary.span.start == 3 && project.error.primary.span.end == 4);
    destroy_project();
}
