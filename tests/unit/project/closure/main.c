/**
 * @file
 * @brief Verify exact project dependency closure, requester chains and failed-load cleanup.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#include "../common.c"
#include "support/identity.c"
#include "support/diagnostics.c"

/**
 * Write the explicit diamond dependency fixture so shared module identity and requester edges
 * are observable.
 */
static void prepare_diamond(void) {
    directory("generated/diamond");
    file("generated/diamond/main", "@<\nleft\n.left\n@>\nright\n.right\n@!\nagain\n.left\n");
    file("generated/diamond/left", "@!\nvalue\n.shared\n");
    file("generated/diamond/right", "@!\nvalue\n.shared\n");
    file("generated/diamond/shared", "@C\n");
    file("generated/diamond/draft", "not active, and not parsed");
    file("generated/diamond/unused", "@!\n syntactically invalid\n");
}

/**
 * Assert that the component shares diamond dependencies and ignores unused syntax. The explicit
 * fixture setup and observations define this regression; assertion failure keeps its generated
 * evidence.
 */
static void it_shares_diamond_dependencies_and_ignores_unused_syntax(void) {
    prepare_diamond();
    setup_project();
    ProjectStatus status = load_root("generated/diamond");
    assert(status == PROJECT_OK && project.valid && module_count() == 4);
    const ProjectModule *left = project_find(&project, ".left");
    const ProjectModule *right = project_find(&project, ".right");
    const ProjectModule *shared = project_find(&project, ".shared");
    assert(left->first->target == shared && right->first->target == shared);
    assert(project.entry->first->target == project.entry->last->target);
    assert(shared->state == PROJECT_MODULE_READY);
    assert(project_find(&project, ".unused") == NULL);
    assert(project_find(&project, ".draft") == NULL);
    destroy_project();
}

/**
 * Assert that the component finishes self and mutual loading cycles. The explicit fixture setup
 * and observations define this regression; assertion failure keeps its generated evidence.
 */
static void it_finishes_self_and_mutual_loading_cycles(void) {
    directory("generated/cycle");
    file("generated/cycle/main", "@!\nself&\n.main\nother&[]\n.other\n");
    file("generated/cycle/other", "@!\nback&\n.main\n");
    setup_project();
    ProjectStatus status = load_root("generated/cycle");
    assert(status == PROJECT_OK && module_count() == 2);
    assert(project.entry->first->target == project.entry);
    assert(project.last->first->target == project.entry);
    destroy_project();
}

/**
 * Assert that the component discovers all source forms and nested blocks in lexical order. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_discovers_all_source_forms_and_nested_blocks_in_lexical_order(void) {
    directory("generated/forms");
    file("generated/forms/main", "@=\nconstant\n.kind\n .constants.value\n"
        "@<\nparameter\n.kind\n@>\nresult\n.kind\n@!\n"
        "copy\n .assignment.value\nworker !\n < .argument.value\n:\n"
        " fallback\n .fallback\nready\n?\n flag\n {\n  child\n  .nested\n }\n:\n"
        " alternative\n .alternative\n.namespace\n value >> answer\n");
    const char *names[] = {"kind", "constants", "assignment", "argument", "fallback",
        "nested", "alternative", "namespace"};
    for (size_t index = 0; index < sizeof(names) / sizeof(names[0]); index++) {
        char path[128];
        int length = snprintf(path, sizeof(path), "generated/forms/%s", names[index]);
        assert(length > 0 && (size_t)length < sizeof(path));
        file(path, "@C\n");
    }
    setup_project();
    ProjectStatus status = load_root("generated/forms");
    if (status != PROJECT_OK) {
        fprintf(stderr, "Forms failed: %s\n", project.error.message);
    }
    assert(status == PROJECT_OK && module_count() == 9);
    const char *edges[] = {".kind", ".constants", ".kind", ".kind", ".assignment",
        ".argument", ".fallback", ".nested", ".alternative", ".namespace"};
    ProjectDependency *edge = project.entry->first;
    for (size_t index = 0; index < sizeof(edges) / sizeof(edges[0]); index++) {
        assert(edge != NULL && strcmp(edge->logical_path, edges[index]) == 0);
        assert(source_span_valid(edge->requirement));
        edge = edge->next;
    }
    assert(edge == NULL);
    destroy_project();
}

/**
 * Write a finite explicit dependency chain for exact requester diagnostics and iterative closure
 * tests.
 */
static void prepare_chain(const char *last) {
    directory("generated/chain");
    file("generated/chain/main", "@!\nchild\n.middle\n");
    file("generated/chain/middle", "@!\nchild\n.last\n");
    if (last != NULL) {
        file("generated/chain/last", last);
    }
}

/**
 * Compare the actual requester/dependency chain with the exact expected module sequence; no
 * basename or depth approximation.
 */
static void expect_chain(ProjectStatus expected) {
    setup_project();
    ProjectStatus status = load_root("generated/chain");
    assert(status == expected && !project.valid);
    assert(project_find(&project, ".main") == NULL);
    assert(diagnostic_valid(&project.error));
    assert(strcmp(project.error.subject, ".last") == 0);
    assert(project.error.related_count == 2);
    assert(strcmp(project.error.related[0].name, ".middle") == 0);
    assert(strcmp(project.error.related[1].name, ".main") == 0);
    assert(project.error.related[0].location.span.start == 9);
    destroy_project();
}

/**
 * Assert that the component reports request chains for missing draft and invalid modules. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_reports_request_chains_for_missing_draft_and_invalid_modules(void) {
    prepare_chain(NULL);
    expect_chain(PROJECT_MISSING);
    prepare_chain("unfinished module");
    expect_chain(PROJECT_DRAFT);
    prepare_chain("@!\n bad\n");
    expect_chain(PROJECT_SYNTAX_ERROR);
}

/**
 * Assert that the component owns entry errors and locates a draft entry. The explicit fixture
 * setup and observations define this regression; assertion failure keeps its generated evidence.
 */
static void it_owns_entry_errors_and_locates_a_draft_entry(void) {
    directory("generated/entry");
    setup_project();
    ModuleRoot root = {"generated/entry", NULL, 0, MODULE_ROOT_PROJECT};
    char entry[] = ".main";
    ProjectStatus status = project_load(&project, &root, 1, entry);
    entry[1] = 'X';
    assert(status == PROJECT_MISSING && diagnostic_valid(&project.error));
    assert(strcmp(project.error.subject, ".main") == 0);
    assert(project.error.related_count == 0);
    destroy_project();
    file("generated/entry/main", "unfinished");
    setup_project();
    status = load_root("generated/entry");
    assert(status == PROJECT_DRAFT && diagnostic_valid(&project.error));
    assert(project.error.primary.span.source == project.entry->source->text);
    destroy_project();
}

/**
 * Assert that the component rejects constants only entries but accepts them as dependencies. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_rejects_constants_only_entries_but_accepts_them_as_dependencies(void) {
    directory("generated/constants");
    file("generated/constants/main", "@=\n");
    setup_project();
    ProjectStatus status = load_root("generated/constants");
    assert(status == PROJECT_INVALID_ENTRY && diagnostic_valid(&project.error));
    destroy_project();
    file("generated/constants/main", "@!\n.library\n value >> result\n");
    file("generated/constants/library", "@=\n");
    setup_project();
    status = load_root("generated/constants");
    assert(status == PROJECT_OK && module_count() == 2);
    destroy_project();
}

/**
 * Assert that the component resolves multiple roots and submodules without loading the parent.
 * The explicit fixture setup and observations define this regression; assertion failure keeps
 * its generated evidence.
 */
static void it_resolves_multiple_roots_and_submodules_without_loading_the_parent(void) {
    directory("generated/application");
    directory("generated/library");
    directory("generated/library/商店");
    file("generated/application/main", "@!\n商品\n.商店.商品.详情\n");
    file("generated/library/商店/商品", "unfinished parent");
    directory("generated/library/商店/商品+");
    file("generated/library/商店/商品+/详情", "@C\n");
    ModuleRoot roots[] = {{"generated/application", NULL, 0, MODULE_ROOT_PROJECT}, {"generated/library", NULL, 0, MODULE_ROOT_PROJECT}};
    for (size_t order = 0; order < 2; order++) {
        setup_project();
        ProjectStatus status = project_load(&project, roots, 2, ".main");
        assert(status == PROJECT_OK && module_count() == 2);
        assert(project_find(&project, ".商店.商品") == NULL);
        assert(project_find(&project, ".商店.商品.详情") != NULL);
        destroy_project();
        ModuleRoot temporary = roots[0];
        roots[0] = roots[1];
        roots[1] = temporary;
    }
    file("generated/library/main", "@!\n");
    setup_project();
    ProjectStatus status = project_load(&project, roots, 2, ".main");
    assert(status == PROJECT_CATALOG_ERROR && project.error.code == DIAGNOSTIC_AMBIGUOUS_MODULE);
    assert(project.first == NULL && project.error.related_count == 1);
    assert(diagnostic_valid(&project.error));
    destroy_project();
}

/**
 * Assert that the component cleans every allocation failure in success and error paths. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_cleans_every_allocation_failure_in_success_and_error_paths(void) {
    prepare_diamond();
    prepare_chain("unfinished");
    const char *roots[] = {"generated/diamond", "generated/chain"};
    for (size_t root = 0; root < 2; root++) {
        setup_project();
        ProjectStatus status = load_root(roots[root]);
        assert(status == (root == 0 ? PROJECT_OK : PROJECT_DRAFT));
        size_t allocations = allocation_count;
        destroy_project();
        for (size_t failure = 1; failure <= allocations; failure++) {
            setup_project();
            failure_at = failure;
            status = load_root(roots[root]);
            assert(status == PROJECT_OUT_OF_MEMORY && !project.valid);
            assert(diagnostic_valid(&project.error));
            destroy_project();
        }
    }
}

/**
 * Assert that the component matches reachability for every directed graph on three modules. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_matches_reachability_for_every_directed_graph_on_three_modules(void) {
    directory("generated/graphs");
    const char *names[] = {"main", "left", "right"};
    for (unsigned graph = 0; graph < 512; graph++) {
        for (size_t vertex = 0; vertex < 3; vertex++) {
            char path[64];
            int length = snprintf(path, sizeof(path), "generated/graphs/%s", names[vertex]);
            assert(length > 0 && (size_t)length < sizeof(path));
            FILE *stream = fopen(path, "wb");
            assert(stream != NULL);
            int written = fprintf(stream, "@!\n");
            assert(written > 0);
            for (size_t target = 0; target < 3; target++) {
                if ((graph & (1u << (vertex * 3 + target))) != 0) {
                    written = fprintf(stream, "field%zu&\n.%s\n", target, names[target]);
                    assert(written > 0);
                }
            }
            int closed = fclose(stream);
            assert(closed == 0);
        }
        bool reachable[3] = {true, false, false};
        for (size_t step = 0; step < 3; step++) {
            for (size_t from = 0; from < 3; from++) {
                for (size_t to = 0; to < 3; to++) {
                    if (reachable[from] && (graph & (1u << (from * 3 + to))) != 0) {
                        reachable[to] = true;
                    }
                }
            }
        }
        setup_project();
        ProjectStatus status = load_root("generated/graphs");
        assert(status == PROJECT_OK);
        size_t count = 0;
        for (size_t vertex = 0; vertex < 3; vertex++) {
            char logical[32];
            int length = snprintf(logical, sizeof(logical), ".%s", names[vertex]);
            assert(length > 0 && (size_t)length < sizeof(logical));
            const ProjectModule *module = project_find(&project, logical);
            assert((module != NULL) == reachable[vertex]);
            if (module != NULL) {
                count++;
                for (ProjectDependency *edge = module->first; edge != NULL; edge = edge->next) {
                    assert(edge->target == project_find(&project, edge->logical_path));
                }
            }
        }
        assert(module_count() == count);
        destroy_project();
    }
}

/**
 * Assert that the component walks deep blocks and dependency chains without c recursion. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_walks_deep_blocks_and_dependency_chains_without_c_recursion(void) {
    directory("generated/deep");
    const size_t depth = 128;
    FILE *stream = fopen("generated/deep/main", "wb");
    assert(stream != NULL);
    int written = fprintf(stream, "@!\n");
    assert(written > 0);
    for (size_t level = 0; level < depth; level++) {
        written = fprintf(stream, "%*sflag\n%*s{\n", (int)level, "", (int)level, "");
        assert(written > 0);
    }
    written = fprintf(stream, "%*sitem\n%*s.n0\n", (int)depth, "", (int)depth, "");
    assert(written > 0);
    for (size_t level = depth; level > 0; level--) {
        written = fprintf(stream, "%*s}\n", (int)(level - 1), "");
        assert(written > 0);
    }
    int closed = fclose(stream);
    assert(closed == 0);
    for (size_t index = 0; index < depth; index++) {
        char path[64];
        char content[64];
        int length = snprintf(path, sizeof(path), "generated/deep/n%zu", index);
        assert(length > 0 && (size_t)length < sizeof(path));
        if (index + 1 == depth) {
            file(path, "@C\n");
        } else {
            length = snprintf(content, sizeof(content), "@!\nnext&\n.n%zu\n", index + 1);
            assert(length > 0 && (size_t)length < sizeof(content));
            file(path, content);
        }
    }
    setup_project();
    ProjectStatus status = load_root("generated/deep");
    assert(status == PROJECT_OK && module_count() == depth + 1);
    destroy_project();
}

#include "support/expanded.c"
#include "support/order.c"
#include "support/activation.c"
#include "support/failures.c"
#include "support/intrinsic.c"
#include "support/analysis-set.c"

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_loads_constants_only_and_empty_analysis_sets_without_an_entry();
    it_requires_the_fixed_standard_integer_once_for_intrinsic_interface_parents();
    it_unwinds_every_implicit_interface_dependency_allocation_in_success_and_error_paths();
    it_selects_native_companions_only_for_required_parsed_native_modules();
    it_reloads_required_drafts_after_activation_without_reusing_an_inactive_record();
    it_invalidates_activation_changes_and_tracks_only_contributing_companions();
    it_unwinds_every_expanded_discovery_and_native_error_allocation_failure();
    it_discovers_expanded_trees_once_in_exact_physical_request_order();
    it_reports_requester_chains_from_expanded_header_and_body_paths();
    it_sorts_every_seven_request_permutation_stably_without_allocation();
    it_preserves_narrow_expected_constructs_and_both_dependency_locations();
    it_composes_real_filesystem_and_unicode_failures_without_losing_source_context();
    it_shares_complete_multiword_seeds_across_root_orders();
    it_preserves_exact_missing_seed_identity_without_family_fallback();
    it_unwinds_every_complete_identity_registry_allocation_failure();
    it_shares_diamond_dependencies_and_ignores_unused_syntax();
    it_finishes_self_and_mutual_loading_cycles();
    it_discovers_all_source_forms_and_nested_blocks_in_lexical_order();
    it_reports_request_chains_for_missing_draft_and_invalid_modules();
    it_owns_entry_errors_and_locates_a_draft_entry();
    it_rejects_constants_only_entries_but_accepts_them_as_dependencies();
    it_resolves_multiple_roots_and_submodules_without_loading_the_parent();
    it_cleans_every_allocation_failure_in_success_and_error_paths();
    it_matches_reachability_for_every_directed_graph_on_three_modules();
    it_walks_deep_blocks_and_dependency_chains_without_c_recursion();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
