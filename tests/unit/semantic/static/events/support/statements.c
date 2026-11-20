/** @file
 * @brief Verify original statement identity, section roles and checked catalog publication.
 */
/** Every selected handler, folded command, branch and foreach body contributes
 * exactly its original owning node; control condition-owner aliases add none.
 */
static void it_catalogs_original_statement_owners_without_control_alias_duplicates(void) {
    setup_static(expanded_event_source, STATIC_SCOPE_SOURCE_TEXT);
    StaticModule *module = static_module_add(&inventory, project.entry);
    assert(module && static_collect_occurrences(&inventory, module));
    const AstNode *root = project.entry->syntax.body.first;
    const AstNode *selected = root->as.call.first_output->operation;
    const AstNode *loop = root->next;
    const AstNode *command = loop->as.control.command;
    const AstNode *method = command->as.call.first_output->operation;
    const AstNode *traversal = loop->next;
    const AstNode *branch = traversal->next;
    const AstNode *expected[] = {root, selected, selected->as.call.alternative.first,
        root->as.call.alternative.first, loop, command, method,
        loop->as.control.body.first, method->as.call.alternative.first,
        traversal, traversal->as.traversal.body.first, branch,
        branch->as.control.body.first, branch->as.control.alternative.first};
    bool seen[sizeof(expected) / sizeof(*expected)] = {false};
    assert(module->statement_count == sizeof(expected) / sizeof(*expected));
    size_t count = 0;
    for (const StaticStatement *statement = module->first_statement; statement; statement = statement->next) {
        assert(statement->context == STATIC_CONTEXT_BODY && statement->syntax != NULL);
        size_t index = 0;
        while (index < sizeof(expected) / sizeof(*expected) && expected[index] != statement->syntax) { index++; }
        assert(index < sizeof(expected) / sizeof(*expected) && !seen[index]);
        seen[index] = true;
        count++;
        if (statement->next == NULL) { assert(statement == module->last_statement); }
    }
    assert(count == sizeof(expected) / sizeof(*expected));
    assert(loop->as.control.condition_owner == method);
    destroy_static();
}

/** Module-wide explicit inventory needs the original shared/input/result/body
 * role even before any field Type or static formal becomes concrete.
 */
static void it_preserves_written_section_roles_in_the_original_statement_catalog(void) {
    setup_static("@=\nshared\n/test/fixture/v1.0.0/src/other\n@<\ninput\n/test/fixture/v1.0.0/src/other\n@>\nresult\n/test/fixture/v1.0.0/src/other\n@!\nstate\n/test/fixture/v1.0.0/src/other\n", "@!\n");
    StaticModule *module = static_module_add(&inventory, project.entry);
    assert(module && static_collect_occurrences(&inventory, module));
    const AstNode *expected[] = {project.entry->syntax.constants.first, project.entry->syntax.parameters.first,
        project.entry->syntax.results.first, project.entry->syntax.body.first};
    const StaticContext roles[] = {STATIC_CONTEXT_CONSTANT, STATIC_CONTEXT_INPUT, STATIC_CONTEXT_RESULT, STATIC_CONTEXT_BODY};
    bool seen[4] = {false};
    assert(module->statement_count == 4);
    for (const StaticStatement *statement = module->first_statement; statement; statement = statement->next) {
        size_t index = 0;
        while (index < 4 && expected[index] != statement->syntax) { index++; }
        assert(index < 4 && !seen[index] && statement->context == roles[index]);
        seen[index] = true;
    }
    for (size_t index = 0; index < 4; index++) { assert(seen[index]); }
    destroy_static();
}

/** Count overflow refuses before allocation or chain mutation; the existing
 * exhaustive collection-allocation sweep also covers every statement append.
 */
static void it_rejects_statement_counter_overflow_before_catalog_publication(void) {
    setup_static(expanded_event_source, STATIC_SCOPE_SOURCE_TEXT);
    StaticModule *module = static_module_add(&inventory, project.entry);
    assert(module != NULL);
    module->statement_count = SIZE_MAX;
    size_t before = allocation_count;
    assert(!static_statement_append(&inventory, module, project.entry->syntax.body.first, STATIC_CONTEXT_BODY));
    assert(module->first_statement == NULL && module->last_statement == NULL && module->statement_count == SIZE_MAX);
    assert(allocation_count == before && inventory.status == STATIC_OUT_OF_MEMORY && diagnostic_valid(&inventory.error));
    destroy_static();
}
