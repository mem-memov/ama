/** @file
 * @brief Check actual qualified dependency normalization before namespace/member selection.
 * The owning input leaf supplies real package snapshots and counted allocation teardown.
 */

/** Prepare a complete local alias and constant namespace without a literal-type guess. */
static void prepare_qualified_project(void) {
    prepare_scoped_project();
    file("generated/scope-project/packages/test/fixture/v1.0.0/src/main.ini",
        "~whole = ./palette/values\n");
    file("generated/scope-project/packages/test/fixture/v1.0.0/src/main",
        "@!\nfirst\n ~whole[]\nsecond\n ./ghost/../palette/red\n"
        "third\n ./palette/red/../blue\n");
    file("generated/scope-project/packages/test/fixture/v1.0.0/src/palette",
        "@=\nred\n./number\n |1\nblue\n./number\n |2\nvalues[]\n./number\n@!\n");
    file("generated/scope-project/packages/test/fixture/v1.0.0/src/number", "@!\n");
}

/** A whole alias and cancelled intermediate components resolve one exact namespace. */
static void it_normalizes_whole_qualified_operands_before_selecting_constant_members(void) {
    prepare_qualified_project();
    setup_project();
    assert(load_root("generated/scope-project") == PROJECT_OK);
    const ProjectModule *palette = project_find(&project, "/test/fixture/v1.0.0/src/palette");
    const char *members[] = {"values", "red", "blue"};
    const ProjectDependency *edge = project.entry->first;
    for (size_t index = 0; index < 3; index++, edge = edge->next) {
        assert(edge && edge->target == palette && edge->role == PROJECT_CONSTANT_PATH);
        assert(edge->constant_path && edge->resolved_path == edge->constant_path->module);
        const ModulePathPart *member = &edge->constant_path->member;
        assert(member->length == strlen(members[index]));
        assert(memcmp(member->bytes, members[index], member->length) == 0);
        assert(edge->requirement.source == project.entry->source->text);
        assert(strcmp(source_path(member->origin.source), index == 0
            ? "generated/scope-project/packages/test/fixture/v1.0.0/src/main.ini"
            : "generated/scope-project/packages/test/fixture/v1.0.0/src/main") == 0);
    }
    assert(edge == NULL);
    assert(project.entry->syntax.body.first->as.assignment.source->constant_list);
    assert(project.entry->first->requirement.end - project.entry->first->requirement.start == strlen("~whole"));
    assert(project_find(&project, "/test/fixture/v1.0.0/src/ghost") == NULL);
    destroy_project();
}

/** A reduced final member can come from canonical containing-directory metadata. */
static void it_keeps_inherited_constant_members_without_fabricating_source_spans(void) {
    prepare_qualified_project();
    directory("generated/scope-project/packages/test/fixture/v1.0.0/src/palette+");
    file("generated/scope-project/packages/test/fixture/v1.0.0/src/palette+/red", "@!\n");
    directory("generated/scope-project/packages/test/fixture/v1.0.0/src/palette+/red+");
    file("generated/scope-project/packages/test/fixture/v1.0.0/src/palette+/red+/reader",
        "@!\nvalue\n ./ghost/..\n");
    setup_project();
    ModuleRoot root = {.path = "generated/scope-project", .kind = MODULE_ROOT_PROJECT};
    assert(project_load(&project, &root, 1,
        "/test/fixture/v1.0.0/src/palette/red/reader") == PROJECT_OK);
    const ProjectDependency *edge = project.entry->first;
    assert(edge && edge->constant_path && edge->constant_path->member.origin.source == NULL);
    assert(strcmp(edge->constant_path->module->text, "/test/fixture/v1.0.0/src/palette") == 0);
    assert(edge->constant_path->member.length == 3 && memcmp(edge->constant_path->member.bytes, "red", 3) == 0);
    destroy_project();
}

/** Invalid normalized prefixes and forbidden test namespaces retain raw source diagnostics. */
static void it_rejects_invalid_and_forbidden_qualified_namespaces_after_reduction(void) {
    const char *operands[] = {"~whole/../..", "../tests/palette/red", "~unknown", "/member", "//member"};
    for (size_t index = 0; index < sizeof(operands) / sizeof(*operands); index++) {
        prepare_qualified_project();
        file("generated/scope-project/packages/test/fixture/v1.0.0/tests/palette", "@!\n");
        char text[256];
        int length = snprintf(text, sizeof(text), "@!\nvalue\n %s\n", operands[index]);
        assert(length > 0 && (size_t)length < sizeof(text));
        file("generated/scope-project/packages/test/fixture/v1.0.0/src/main", text);
        setup_project();
        assert(load_root("generated/scope-project") == PROJECT_INVALID_CONFIGURATION);
        assert(!project.valid && diagnostic_valid(&project.error));
        assert(project.error.primary.span.source == project.entry->source->text);
        destroy_project();
    }
}

/** Descriptor/member-copy failures remain ordinary OOM under one project destructor. */
static void it_unwinds_every_whole_qualified_path_allocation_failure(void) {
    prepare_qualified_project();
    setup_project();
    assert(load_root("generated/scope-project") == PROJECT_OK);
    size_t measured = allocation_count;
    destroy_project();
    for (size_t ordinal = 1; ordinal <= measured; ordinal++) {
        setup_project();
        failure_at = ordinal;
        assert(load_root("generated/scope-project") == PROJECT_OUT_OF_MEMORY);
        assert(!project.valid);
        destroy_project();
    }
}
