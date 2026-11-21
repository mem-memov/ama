/**
 * @file
 * @brief Check Unicode sets, caller-array retirement and nonretained compiled patterns.
 * Contract: [selection](../../../../../../docs/implementation/project-configuration.md#t262-selection-and-explicit-edge-contract).
 */

/** Sparse roots retain only their shared immutable view, not temporary array or mask storage. */
static void it_preserves_unicode_identity_and_independent_view_pattern_lifetimes(void) {
    begin();
    DependencyViewInput entries[] = {
        {path("/вендор/пакет/v1.0.0-быстрый/tests/имя с пробелом/проверка"), 1},
        {path("/厂/包/v1.0.0"), 0},
        {path("/вендор/пакет/v1.0.0-быстрый/src/имя с пробелом/подмодуль"), 0},
        {path("/вендор/пакет/v1.0.0-быстрый"), 0},
        {path("/вендор/пакет/v1.0.0-быстрый/src/имя с пробелом"), 0}
    };
    const ModulePathValue *owner = entries[1].identity, *test = entries[0].identity, *child = entries[2].identity;
    const DependencyView *actual = NULL;
    assert(dependency_view_build(&results, entries, 5, &actual, &issue) == DEPENDENCY_SELECTION_OK);
    memset(entries, 0, sizeof(entries));
    SourceArena masks;
    source_arena_init(&masks, sources.allocator);
    const DependencySelector *selector = NULL;
    assert(dependency_selector_compile(&masks,
        configuration_test_text("/вендор/пакет/v1.0.0-быстрый/src/имя с пробелом"), NULL,
        &selector, &issue) == DEPENDENCY_SELECTOR_OK);
    const DependencySelection *selection = NULL;
    assert(dependency_selection_build(&results, actual, owner, &selector, 1, &selection, &issue) == DEPENDENCY_SELECTION_OK);
    source_arena_destroy(&masks);
    assert(selection->count == 1 && dependency_selection_covers(selection, test) && dependency_selection_covers(selection, child));
    const DependencySelection *unchanged = selection;
    assert(dependency_selection_build(&results, actual, test, NULL, 0, &unchanged, &issue) == DEPENDENCY_SELECTION_INVALID);
    assert(dependency_selection_build(&results, actual, owner, NULL, 1, &unchanged, &issue) == DEPENDENCY_SELECTION_INVALID);
    assert(unchanged == selection);
    const DependencyView *empty = NULL;
    assert(dependency_view_build(&results, NULL, 0, &empty, &issue) == DEPENDENCY_SELECTION_OK);
    assert(empty->count == 0 && empty->entries == NULL);
    assert(dependency_selection_build(&results, empty, owner, NULL, 0, &unchanged, &issue) == DEPENDENCY_SELECTION_INVALID);
    assert(unchanged == selection);
    finish();
}
