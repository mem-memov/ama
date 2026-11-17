/**
 * @file
 * @brief Verify shared frontend outputs, source-backed diagnostics and every allocation failure prefix.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#include "../common.c"

/**
 * Write this scenario's explicit source/native fixtures in test-owned storage before loading or
 * compiling them; the caller controls their lifetime and expected result.
 */
static void prepare_project(void) {
    directory("generated/project");
    file("generated/project/main", "@!\nvalue\n.value\n |literal\n");
    file("generated/project/value", "@C\n");
    file("generated/project/value.h", "#error Checking must not invoke the C compiler\n");
    file("generated/project/value.c", "#error Checking must not invoke the C compiler\n");
    file("generated/project/value.abi", "+ initialize\n- destroy\n| literal\n= clone\n");
}

/**
 * Assert that the component returns the same typed graph that compilation will consume without
 * running c. The explicit fixture setup and observations define this regression; assertion
 * failure keeps its generated evidence.
 */
static void it_returns_the_same_typed_graph_that_compilation_will_consume_without_running_c(void) {
    prepare_project();
    setup_frontend();
    bool valid = check_project();
    assert(valid && frontend.valid && frontend.plan.valid && frontend.semantic.constants_valid);
    assert(frontend.plan.entry->first->symbol->initializers->transfer == TRANSFER_LITERAL);
    assert(frontend.plan.entry->type->module == frontend.project.entry);
    destroy_frontend();
}

/**
 * Assert that the component preserves source semantic and relation diagnostics. The explicit
 * fixture setup and observations define this regression; assertion failure keeps its generated
 * evidence.
 */
static void it_preserves_source_semantic_and_relation_diagnostics(void) {
    const char *cases[] = {"@!\n invalid\n", "@!\nmissing\n.missing\n", "@!\n^\n", "@!\nself\n.main\n"};
    for (size_t index = 0; index < sizeof(cases) / sizeof(cases[0]); index++) {
        prepare_project();
        file("generated/project/main", cases[index]);
        setup_frontend();
        bool valid = check_project();
        assert(!valid && !frontend.valid && diagnostic_valid(&frontend.error));
        destroy_frontend();
    }
}

/**
 * Assert that the component unwinds every frontend allocation failure. The explicit fixture
 * setup and observations define this regression; assertion failure keeps its generated evidence.
 */
static void it_unwinds_every_frontend_allocation_failure(void) {
    prepare_project();
    setup_frontend();
    bool valid = check_project();
    assert(valid);
    size_t total = allocation_count;
    destroy_frontend();
    for (size_t failure = 1; failure <= total; failure++) {
        setup_frontend();
        failure_at = failure;
        valid = check_project();
        assert(!valid && frontend.error.code == DIAGNOSTIC_OUT_OF_MEMORY);
        destroy_frontend();
    }
}

/** Real frontend snapshots retain exact native bytes and propagate their changes to semantic consumers. */
static void it_composes_owned_frontend_input_snapshots_without_executing_native_c(void) {
    prepare_project();
    setup_frontend();
    bool valid = check_project();
    assert(valid && frontend.inputs.sealed && frontend.inputs.compared);
    assert(project_inputs_find(&frontend.inputs, PROJECT_INPUT_FILE, "generated/project/value.c") != NULL);
    file("generated/project/value.c", "#error Changed native input still must not execute\n");
    CompilerFrontend after;
    compiler_frontend_init(&after, sources.allocator);
    ModuleRoot root = {"generated/project", NULL, 0, MODULE_ROOT_PROJECT};
    valid = compiler_frontend_check(&after, &root, 1, ".main");
    assert(valid);
    SourceStatus status = project_inputs_compare(&after.inputs, &frontend.inputs);
    assert(status == SOURCE_OK);
    bool dirty = false;
    bool found = project_inputs_changed(&after.inputs, PROJECT_INPUT_MODULE, ".main", &dirty);
    assert(found && dirty);
    found = project_inputs_changed(&after.inputs, PROJECT_INPUT_MODULE, ".value", &dirty);
    assert(found && dirty);
    found = project_inputs_changed(&after.inputs, PROJECT_INPUT_CONFIGURATION, "root inventory", &dirty);
    assert(found && !dirty);
    compiler_frontend_destroy(&after);
    destroy_frontend();
}

/** Full checked frontend keeps equal component spellings nominally distinct across Project and Standard roots. */
static void it_resolves_multiword_declarations_through_both_root_families(void) {
    prepare_project();
    directory("generated/standard");
    file("generated/standard/value", "@C\n");
    file("generated/standard/value.h", "#error Checking must not invoke C\n");
    file("generated/standard/value.c", "#error Checking must not invoke C\n");
    file("generated/standard/value.abi", "+ initialize\n- destroy\n| literal\n= clone\n");
    file("generated/project/main", "@!\nordinary state\n.value\n |a\nstandard state\n..value\n |b\n");
    setup_frontend();
    ModuleRoot roots[] = {{"generated/project",NULL,0,MODULE_ROOT_PROJECT},
        {"generated/standard",NULL,0,MODULE_ROOT_STANDARD}};
    bool valid = compiler_frontend_check(&frontend, roots, 2, ".main");
    assert(valid && frontend.valid);
    const ProjectModule *ordinary = project_find(&frontend.project, ".value");
    const ProjectModule *standard = project_find(&frontend.project, "..value");
    assert(ordinary != NULL && standard != NULL && ordinary != standard);
    assert(frontend.plan.entry->first->symbol->type != frontend.plan.entry->first->next->symbol->type);
    assert(layout_text_is(frontend.project.entry->syntax.body.first->as.declaration.name->name, "ordinary state"));
    assert(layout_text_is(frontend.project.entry->syntax.body.last->as.declaration.type, "..value"));
    destroy_frontend();
}

/** Self-reference has complete typed IR even while final executable identity support remains a separate backend capability. */
static void it_preserves_self_reference_sources_in_complete_ir_without_executing_them(void) {
    const char *programs[] = {
        "@>\nresult&\n.main\n &\n@!\n",
        "@!\nfresh&\n &\n",
        "@!\nworker\n.worker\nworker !\n < &\n"
    };
    for (size_t index = 0; index < sizeof(programs) / sizeof(*programs); index++) {
        prepare_project();
        file("generated/project/main", programs[index]);
        file("generated/project/worker", "@<\ncaller&\n.main\n@!\n");
        setup_frontend();
        bool valid = check_project();
        assert(valid && frontend.valid && frontend.plan.valid && frontend.plan.storage_ready);
        assert(frontend.semantic.resolved && frontend.semantic.constants_valid && frontend.plan_initialized);
        assert(frontend.plan.first != NULL);
        destroy_frontend();
    }
}

/** Selected operations retain complete frame and continuation identities through the full frontend. */
static void it_preserves_selected_result_actions_in_complete_ir(void) {
    prepare_project();
    file("generated/project/main", "@!\nconstructor\n.constructor\nconstructor\n method !\n");
    file("generated/project/constructor", "@>\nmethod\n.method\n@!\n");
    file("generated/project/method", "@!\n");
    setup_frontend();
    bool valid = check_project();
    assert(valid && frontend.valid && frontend.plan_initialized && frontend.plan.valid);
    assert(frontend.semantic.resolved && frontend.semantic.constants_valid);
    assert(frontend.plan.entry->frame_count == 2 && frontend.plan.entry->chains != NULL);
    destroy_frontend();
}

/** Intrinsic interface IR retains its bridge obligation and caller frames without inspecting or executing native C. */
static void it_types_list_interfaces_and_complete_service_ir_without_inspecting_native_c(void) {
    prepare_project();
    directory("generated/standard");
    file("generated/standard/integer", "@C\n");
    file("generated/standard/integer.h", "#error Semantic checking must not inspect the from_size declaration\n");
    file("generated/standard/integer.c", "#error Semantic checking must not invoke the C compiler\n");
    file("generated/standard/integer.abi", "+ initialize\n- destroy\n= clone\n| literal\n[] index\n");
    file("generated/project/main", "@!\nitems[]\n.value\nitems[]\n length !\n  > count\n");
    setup_frontend();
    ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
        {"generated/standard", NULL, 0, MODULE_ROOT_STANDARD}};
    bool valid = compiler_frontend_check(&frontend, roots, 2, ".main");
    assert(valid && frontend.valid && frontend.semantic.resolved && frontend.semantic.constants_valid);
    assert(frontend.semantic.list_integer != NULL && frontend.semantic.list_integer->requires_size_bridge);
    assert(frontend.plan_initialized && frontend.plan.valid && frontend.plan.first != NULL);
    assert(frontend.plan.entry->frame_count == 2);
    destroy_frontend();
}

/** Typed control trees have complete loop labels independently of later execution adapters. */
static void it_builds_complete_control_ir_for_traversal_and_header_trees(void) {
    const char *cases[] = {
        "@!\nitems[]\n.value\nitems[item&]\n{\n}\n",
        "@!\nroot\n.producer\nroot !\n condition\n {\n }\n"
    };
    for (size_t index = 0; index < sizeof(cases) / sizeof(cases[0]); index++) {
        prepare_project();
        file("generated/project/producer", "@>\ncondition\n.value\n@!\n");
        file("generated/project/main", cases[index]);
        file("generated/project/value.abi", "+ initialize\n- destroy\n| literal\n= clone\n? condition\n[] index\n");
        setup_frontend();
        bool valid = check_project();
        assert(valid && frontend.valid && frontend.plan_initialized);
        assert(frontend.semantic.resolved && frontend.semantic.constants_valid && frontend.plan.valid);
        assert(frontend.plan.entry->loops != NULL);
        destroy_frontend();
    }
}

/** Resolve real hyphenated files, multiword names and structural submodules
 * through both configured roots, retaining distinct nominal and field identities.
 */
static void it_retains_hyphenated_names_and_exact_type_identities_in_both_roots(void) {
    const char *roots[] = {"generated/hyphen project", "generated/hyphen standard"};
    const char *names[] = {"a-b", "a_b", "a b", "ab"};
    for (size_t root = 0; root < 2; root++) {
        directory(roots[root]);
        for (size_t index = 0; index < 4; index++) {
            char path[128];
            int written = snprintf(path, sizeof(path), "%s/%s", roots[root], names[index]);
            assert(written > 0 && (size_t)written < sizeof(path));
            file(path, "@!\n");
        }
    }
    directory("generated/hyphen project/a-b+");
    file("generated/hyphen project/a-b+/item-2", "@!\n");
    file("generated/hyphen project/main", "@!\nx-name\n.a-b\nx_name\n.a_b\n"
        "x name\n.a b\nxname\n.ab\nstd-value\n..a-b\nchild-value\n.a-b.item-2\n");
    ModuleRoot configured[] = {{roots[0], NULL, 0, MODULE_ROOT_PROJECT},
        {roots[1], NULL, 0, MODULE_ROOT_STANDARD}};
    setup_frontend();
    assert(compiler_frontend_check(&frontend, configured, 2, ".main"));
    const char *paths[] = {".a-b", ".a_b", ".a b", ".ab", "..a-b", ".a-b.item-2"};
    const char *fields[] = {"x-name", "x_name", "x name", "xname", "std-value", "child-value"};
    SemanticType *types[6];
    SemanticSymbol *field = frontend.plan.entry->type->first;
    for (size_t index = 0; index < 6; index++) {
        types[index] = semantic_find_type(&frontend.semantic, paths[index]);
        assert(types[index] != NULL && field != NULL && field->type == types[index]);
        assert(layout_text_is(field->name, fields[index]));
        for (size_t earlier = 0; earlier < index; earlier++) {
            assert(types[index] != types[earlier]);
        }
        field = field->next;
    }
    assert(field == NULL && frontend.plan.valid);
    destroy_frontend();
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_retains_hyphenated_names_and_exact_type_identities_in_both_roots();
    it_types_list_interfaces_and_complete_service_ir_without_inspecting_native_c();
    it_builds_complete_control_ir_for_traversal_and_header_trees();
    it_preserves_selected_result_actions_in_complete_ir();
    it_preserves_self_reference_sources_in_complete_ir_without_executing_them();
    it_resolves_multiword_declarations_through_both_root_families();
    it_composes_owned_frontend_input_snapshots_without_executing_native_c();
    it_returns_the_same_typed_graph_that_compilation_will_consume_without_running_c();
    it_preserves_source_semantic_and_relation_diagnostics();
    it_unwinds_every_frontend_allocation_failure();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
