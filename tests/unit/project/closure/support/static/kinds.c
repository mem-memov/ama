/** @file
 * @brief Exercise parsed occurrence roles against all complete source kinds.
 */

/** Write a finite source closure; graph outputs deliberately defer export/type
 * legality to later semantic consumers so this test observes only source roles.
 */
static void prepare_kind_case(const char *use, const char *target) {
    fixture_package_prepare("generated/static kinds", "fixture");
    file("generated/static kinds/packages/test/fixture/v1.0.0/src/main", use);
    file("generated/static kinds/packages/test/fixture/v1.0.0/src/target", target);
    file("generated/static kinds/packages/test/fixture/v1.0.0/src/value", "@!\n");
}

/** Every syntactic role is independent of target loading order. A graph is
 * required for a producer or selector and forbidden for ordinary path uses.
 * Neither labels nor the selected export need exist as filesystem modules.
 */
static void it_checks_every_static_path_role_against_each_complete_source_kind(void) {
    const char *uses[] = {
        "@...\n.named\n/test/fixture/v1.0.0/src/target\n",
        "@...\n/test/fixture/v1.0.0/src/target\n .export > .received\n",
        "@!\nvalue\n/test/fixture/v1.0.0/src/target\n .export\n",
        "@!\n/test/fixture/v1.0.0/src/target\n item >> value\n",
        "@!\nvalue\n/test/fixture/v1.0.0/src/target\n",
        "@!\nvalue\n/test/fixture/v1.0.0/src/value\nvalue\n /test/fixture/v1.0.0/src/target/item\n",
        "@...\n.named\n/test/fixture/v1.0.0/src/value\n .input < /test/fixture/v1.0.0/src/target\n",
        "@!\n/test/fixture/v1.0.0/src/target\n .export\n item >> value\n"
    };
    const char *targets[] = {"@!\n", "@C\n", "@=\nitem\n/test/fixture/v1.0.0/src/value\n |0\n", "@...\n.export\n/test/fixture/v1.0.0/src/value\n"};
    for (size_t use = 0; use < sizeof(uses) / sizeof(*uses); use++) {
        for (size_t kind = 0; kind < sizeof(targets) / sizeof(*targets); kind++) {
            prepare_kind_case(uses[use], targets[kind]);
            setup_project();
            ProjectStatus status = load_root("generated/static kinds");
            bool requires_graph = use == 1 || use == 2 || use == 7;
            bool accepted = requires_graph == (kind == 3);
            if (status != (accepted ? PROJECT_OK : PROJECT_INVALID_KIND)) {
                fprintf(stderr, "Role %zu kind %zu: %d %s\n", use, kind, status, project.error.message);
            }
            assert(status == (accepted ? PROJECT_OK : PROJECT_INVALID_KIND));
            assert(project.valid == accepted);
            if (!accepted) {
                assert(diagnostic_valid(&project.error));
                assert(project.error.code == DIAGNOSTIC_TYPE_MISMATCH);
                assert(layout_text_is(project.error.primary.span, use == 5
                    ? "/test/fixture/v1.0.0/src/target/item" : "/test/fixture/v1.0.0/src/target"));
                assert(project.error.primary.span.source == project.entry->source->text);
                assert(project.error.related_count == 1);
                assert(strcmp(project.error.related[0].name, "/test/fixture/v1.0.0/src/main") == 0);
                assert(project_find(&project, "/test/fixture/v1.0.0/src/target") == NULL);
            }
            destroy_project();
        }
    }
}

/** A source-loading cycle must resolve to the same records, but never exempts
 * an ordinary path from the static-kind rule once both sources have parsed.
 */
static void it_checks_kinds_after_cyclic_targets_have_parsed_and_unwinds_each_failure(void) {
    const char *uses[] = {
        "@...\n/test/fixture/v1.0.0/src/target\n .export > .received\n",
        "@...\n.named\n/test/fixture/v1.0.0/src/target\n"
    };
    for (size_t scenario = 0; scenario < 2; scenario++) {
        prepare_kind_case(uses[scenario], "@...\n/test/fixture/v1.0.0/src/main\n .received > .export\n");
        setup_project();
        ProjectStatus expected = scenario == 0 ? PROJECT_OK : PROJECT_INVALID_KIND;
        assert(load_root("generated/static kinds") == expected);
        size_t total = allocation_count;
        if (scenario == 0) {
            const ProjectModule *target = project_find(&project, "/test/fixture/v1.0.0/src/target");
            assert(target->first->target == project.entry);
            assert(project.entry->first->role == PROJECT_STATIC_PRODUCER);
        }
        destroy_project();
        for (size_t failure = 1; failure <= total; failure++) {
            setup_project();
            failure_at = failure;
            assert(load_root("generated/static kinds") == PROJECT_OUT_OF_MEMORY);
            assert(!project.valid && diagnostic_valid(&project.error));
            destroy_project();
        }
    }
}

/** Shared target identity cannot erase a later incompatible occurrence's role;
 * late checking preserves the complete first-requester chain at its own site.
 */
static void it_retains_occurrence_roles_and_nested_requester_locations(void) {
    prepare_kind_case("@!\nvalue\n/test/fixture/v1.0.0/src/middle\n .alias\n", "@...\n.export\n/test/fixture/v1.0.0/src/value\n");
    file("generated/static kinds/packages/test/fixture/v1.0.0/src/middle", "@...\n/test/fixture/v1.0.0/src/target\n .export > .alias\n.bad\n/test/fixture/v1.0.0/src/target\n");
    setup_project();
    assert(load_root("generated/static kinds") == PROJECT_INVALID_KIND);
    const SourceSpan site = project.error.primary.span;
    assert(layout_text_is(site, "/test/fixture/v1.0.0/src/target"));
    assert(project.error.related_count == 2);
    assert(strcmp(project.error.related[0].name, "/test/fixture/v1.0.0/src/middle") == 0);
    assert(strcmp(project.error.related[1].name, "/test/fixture/v1.0.0/src/main") == 0);
    assert(project.error.related[0].location.span.start == site.start);
    assert(layout_text_is(project.error.related[1].location.span, "/test/fixture/v1.0.0/src/middle"));
    const ProjectModule *middle = project.entry->first->target;
    assert(middle->first->target == middle->last->target);
    assert(middle->first->role == PROJECT_STATIC_PRODUCER);
    assert(middle->last->role == PROJECT_ORDINARY_PATH);
    assert(site.start == middle->last->requirement.start);
    assert(!project.valid);
    destroy_project();
}
