/**
 * @file
 * @brief Enumerate root subsets against a hand-specified structural and physical association relation.
 * Contract: [selection](../../../../../../docs/implementation/project-configuration.md#t262-selection-and-explicit-edge-contract).
 */

/** Fixed actual-layout examples distinguish ordinary test directories from plus subtrees. */
enum { OWNER, MAIN, OWN_TEST, VERSION, A, B, C, SIBLING, AB, STRUCT, NODE, TOOL,
    A_TEST, B_TEST, STRUCT_MAIN, STRUCT_TEST, NODE_MAIN, NODE_TEST, TOOL_TEST,
    ORPHAN, FREE, FREE_CHILD, VARIANT, VARIANT_A, RECORDS };
static const char *names[RECORDS] = {
    "/home/app/v1.0.0", "/home/app/v1.0.0/src/main", "/home/app/v1.0.0/tests/check",
    "/ext/pkg/v1.0.0", "/ext/pkg/v1.0.0/src/a", "/ext/pkg/v1.0.0/src/a/b",
    "/ext/pkg/v1.0.0/src/a/b/c", "/ext/pkg/v1.0.0/src/a/sibling", "/ext/pkg/v1.0.0/src/ab",
    "/ext/pkg/v1.0.0/src/struct", "/ext/pkg/v1.0.0/src/branch/node", "/ext/pkg/v1.0.0/src/namespace/tool",
    "/ext/pkg/v1.0.0/tests/a/check", "/ext/pkg/v1.0.0/tests/a/b/check",
    "/ext/pkg/v1.0.0/tests/struct", "/ext/pkg/v1.0.0/tests/struct/case",
    "/ext/pkg/v1.0.0/tests/branch/node", "/ext/pkg/v1.0.0/tests/branch/node/case",
    "/ext/pkg/v1.0.0/tests/namespace/tool/check", "/ext/pkg/v1.0.0/tests/orphan/check",
    "/ext/pkg/v1.0.0/tests/free", "/ext/pkg/v1.0.0/tests/free/child",
    "/ext/pkg/v1.0.0-speed", "/ext/pkg/v1.0.0-speed/src/a"
};
static const char *physical[RECORDS] = {
    NULL, "main", "check", NULL, "a", "a+/b", "a+/b+/c", "a+/sibling", "ab", "struct",
    "branch/node", "namespace/tool", "a/check", "a/b/check", "struct", "struct+/case",
    "branch/node", "branch/node+/case", "namespace/tool/check", "orphan/check", "free", "free+/child", NULL, "a"
};
static DependencyViewInput inputs[RECORDS];
static const DependencyView *view;

/** Independent explicit record-to-physical-directory projection, not inferred from logical names. */
static void prepare(void) {
    begin();
    for (size_t index = 0; index < RECORDS; index++) { inputs[index] = (DependencyViewInput){path(names[index]), 0}; }
    inputs[A_TEST].test_directory_prefix = 1;
    inputs[B_TEST].test_directory_prefix = 2;
    inputs[NODE_MAIN].test_directory_prefix = 1;
    inputs[NODE_TEST].test_directory_prefix = 1;
    inputs[TOOL_TEST].test_directory_prefix = 2;
    inputs[ORPHAN].test_directory_prefix = 1;
    for (size_t index = 0; index < RECORDS; index++) {
        if (physical[index] == NULL) { continue; }
        DependencyViewInput projected;
        assert(dependency_view_provenance(inputs[index].identity, (const unsigned char *)physical[index],
            strlen(physical[index]), &projected, &issue) == DEPENDENCY_SELECTION_OK);
        assert(projected.identity == inputs[index].identity
            && projected.test_directory_prefix == inputs[index].test_directory_prefix);
        inputs[index] = projected;
    }
    assert(dependency_view_build(&results, inputs, RECORDS, &view, &issue) == DEPENDENCY_SELECTION_OK);
}

/** An explicit finite relation lists expected unit membership without production prefix predicates. */
static uint64_t relation(size_t root) {
    uint64_t a = (UINT64_C(1) << A) | (UINT64_C(1) << B) | (UINT64_C(1) << C)
        | (UINT64_C(1) << SIBLING) | (UINT64_C(1) << A_TEST) | (UINT64_C(1) << B_TEST);
    if (root == VERSION) {
        return a | (UINT64_C(1) << VERSION) | (UINT64_C(1) << AB) | (UINT64_C(1) << STRUCT)
            | (UINT64_C(1) << NODE) | (UINT64_C(1) << TOOL) | (UINT64_C(1) << TOOL_TEST);
    }
    if (root == A) { return a; }
    if (root == B) { return (UINT64_C(1) << B) | (UINT64_C(1) << C) | (UINT64_C(1) << B_TEST); }
    if (root == AB) { return UINT64_C(1) << AB; }
    if (root == FREE) { return (UINT64_C(1) << FREE) | (UINT64_C(1) << FREE_CHILD); }
    return UINT64_C(1) << root;
}

/** All 64 mask subsets cover exact finite sets; duplicate/reversed masks cannot add roots. */
static void it_matches_all_bounded_root_unions_and_physical_test_associations(void) {
    prepare();
    const char *expressions[] = {"/ext/pkg/v1.0.0", "/ext/pkg/v1.0.0/src/a", "/ext/pkg/v1.0.0/src/a/b",
        "/ext/pkg/v1.0.0/tests/free", "/ext/*/v1.0.0/src/a*", "/missing/*"};
    const uint64_t matched[] = {UINT64_C(1) << VERSION, UINT64_C(1) << A, UINT64_C(1) << B,
        UINT64_C(1) << FREE, (UINT64_C(1) << A) | (UINT64_C(1) << AB), 0};
    const DependencySelector *patterns[6];
    for (size_t index = 0; index < 6; index++) { patterns[index] = pattern(expressions[index]); }
    for (size_t subset = 0; subset < 64; subset++) {
        const DependencySelector *selected[12];
        size_t count = 0;
        uint64_t expected_roots = 0, coverage = 0;
        for (size_t index = 6; index != 0; index--) {
            if ((subset & ((size_t)1 << (index - 1))) != 0) {
                selected[count++] = patterns[index - 1]; selected[count++] = patterns[index - 1];
                expected_roots |= matched[index - 1];
            }
        }
        size_t roots = 0;
        for (size_t root = 0; root < RECORDS; root++) {
            if ((expected_roots & (UINT64_C(1) << root)) != 0) { roots++; coverage |= relation(root); }
        }
        const DependencySelection *selection = NULL;
        assert(dependency_selection_build(&results, view, inputs[OWNER].identity, selected, count,
            &selection, &issue) == DEPENDENCY_SELECTION_OK);
        assert(selection->count == roots);
        uint64_t actual_roots = 0;
        for (const DependencySelectionRoot *root = selection->roots; root != NULL; root = root->next) {
            for (size_t index = 0; index < RECORDS; index++) {
                if (root->entry->identity == inputs[index].identity) { actual_roots |= UINT64_C(1) << index; }
            }
        }
        assert(actual_roots == expected_roots);
        for (size_t index = 0; index < RECORDS; index++) {
            assert(dependency_selection_covers(selection, inputs[index].identity)
                == ((coverage & (UINT64_C(1) << index)) != 0));
        }
    }
    finish();
}
