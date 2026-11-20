/** @file
 * @brief Check original static occurrence contexts through every owning runtime subtree.
 */
/** Parser/project-valid source shapes exercise collection before category/type
 * checking; foreign signatures/labels are intentionally not local occurrences.
 */
static const char *event_source = "@=\nshared\n.from interface\n"
    "@<\ninput&\n.source type\nother&\n.from interface\n"
    "@>\nresult\n.from interface\n"
    "@!\nflag\n?\n branch field\n .source type\nworker !\n:\n"
    " handler field\n .from interface\n/test/fixture/v1.0.0/src/other\n .selected export\n zero >> read\n";

/** A separate graph owns nested input values and named producer receivers. */
static const char *graph_event_source = "@...\n.prepared\n.factory\n"
    " .foreign only < .local type\n  .nested slot < .nested type\n"
    "/test/fixture/v1.0.0/src/other\n .slot < .local type\n .out > .received type\n";

/** All runtime owning edges can contain static uses, including handlers in a
 * folded header; alias pointers must not duplicate those declaration owners.
 */
static const char *expanded_event_source = "@!\nroot !\n selected !\n :\n"
    "  selected field\n  .selected type\n > receiving\n:\n root field\n .root type\n"
    "chain !\n next !\n  condition\n  {\n   loop field\n   .loop type\n  }\n"
    " :\n  header field\n  .header type\nitems[item&]\n{\n iterator field\n .iterator type\n}\n"
    "flag\n?\n positive field\n .positive type\n:\n negative field\n .negative type\n";

/** Every owning static expression is collected with its original role/root;
 * runtime branch/handler nesting does not change the static module scope.
 */
static void it_collects_exact_static_heads_and_receivers_in_all_source_contexts(void) {
    setup_static(event_source, "@...\n.selected export\n.selected type\n");
    StaticModule *module = static_module_add(&inventory, project.entry);
    assert(module != NULL && static_collect_occurrences(&inventory, module));
    static_sort_occurrences(module);
    const char *names[] = {".from interface", ".source type", ".from interface",
        ".from interface", ".source type", ".from interface", "/test/fixture/v1.0.0/src/other"};
    StaticContext contexts[] = {STATIC_CONTEXT_CONSTANT, STATIC_CONTEXT_INPUT,
        STATIC_CONTEXT_INPUT, STATIC_CONTEXT_RESULT, STATIC_CONTEXT_BODY,
        STATIC_CONTEXT_BODY, STATIC_CONTEXT_NAMESPACE};
    assert(module->occurrence_count == sizeof(names) / sizeof(*names));
    const StaticOccurrence *record = module->first_occurrence, *last = NULL;
    size_t previous = 0;
    for (size_t index = 0; index < sizeof(names) / sizeof(*names); index++) {
        assert(record != NULL && layout_text_is(record->site, names[index]));
        assert(record->context == contexts[index] && record->root);
        assert(record->receiver == NULL);
        assert(record->expression != NULL && record->statement != NULL && record->symbol == NULL && record->origin == NULL);
        assert(record->site.source == project.entry->source->text && record->site.start > previous);
        previous = record->site.start;
        last = record;
        record = record->next;
    }
    assert(record == NULL && last == module->last_occurrence && !inventory.valid);
    assert(module->first == NULL && module->parameter_count == 0 && module->result_count == 0);
    assert(module->first_occurrence->site.start == static_name_site(project.entry, ".from interface").start);
    destroy_static();
}

/** Foreign selector/slot/output labels never become local occurrences; graph
 * heads and receivers retain their exact owned expression and source context.
 */
static void it_collects_static_graph_owners_without_importing_foreign_labels(void) {
    setup_static(graph_event_source, "@...\n.out\n.slot\n");
    StaticModule *module = static_module_add(&inventory, project.entry);
    assert(module != NULL && static_collect_occurrences(&inventory, module));
    static_sort_occurrences(module);
    const char *names[] = {".factory", ".local type", ".nested type", "/test/fixture/v1.0.0/src/other",
        ".local type", ".received type"};
    const bool roots[] = {true, false, false, true, false, true};
    const StaticOccurrence *record = module->first_occurrence;
    assert(module->occurrence_count == 6);
    for (size_t index = 0; index < 6; index++) {
        assert(record != NULL && layout_text_is(record->site, names[index]));
        assert(record->context == STATIC_CONTEXT_DEFINITION && record->root == roots[index]);
        assert((record->receiver != NULL) == (index == 5));
        assert(record->statement->kind == (index < 3 ? AST_STATIC_RESULT : AST_STATIC_PROJECTION));
        record = record->next;
    }
    assert(record == NULL);
    destroy_static();
}

/** Follow selected handlers, lexical loop commands/bodies, traversal bodies and
 * both branch arms once each, retaining module-wide static body context.
 */
static void it_visits_every_runtime_owning_subtree_once_without_following_alias_links(void) {
    setup_static(expanded_event_source, STATIC_SCOPE_SOURCE_TEXT);
    StaticModule *module = static_module_add(&inventory, project.entry);
    assert(module != NULL && static_collect_occurrences(&inventory, module));
    static_sort_occurrences(module);
    const char *names[] = {".selected type", ".root type", ".loop type", ".header type",
        ".iterator type", ".positive type", ".negative type"};
    const StaticOccurrence *record = module->first_occurrence;
    assert(module->occurrence_count == sizeof(names) / sizeof(*names));
    for (size_t index = 0; index < sizeof(names) / sizeof(*names); index++) {
        assert(record != NULL && layout_text_is(record->site, names[index]));
        assert(record->context == STATIC_CONTEXT_BODY && record->root && record->receiver == NULL);
        assert(record->statement->kind == AST_DECLARATION);
        record = record->next;
    }
    assert(record == NULL);
    destroy_static();
}
