/** @file
 * @brief Verify strict-build prerequisite refusal of unresolved static syntax at executable lowering.
 */

/** Native prepass integration compiles the whole frontend under strict warnings.
 * Both static graph AST kinds must be explicitly refused if an invalid semantic
 * producer leaks them into lowering; no runtime node or allocation is created.
 */
static void it_refuses_unresolved_static_nodes_at_the_executable_lowering_boundary(void) {
    const char *texts[] = {"@...\n...result\n.other\n", "@...\n.other\n ...result > ...alias\n"};
    const AstNodeKind kinds[] = {AST_STATIC_RESULT, AST_STATIC_PROJECTION};
    for (size_t index = 0; index < 2; index++) {
        setup_test();
        const SourceText *source = NULL;
        assert(source_manager_add(&sources, "lowering boundary", (const unsigned char *)texts[index], strlen(texts[index]), &source) == SOURCE_OK);
        AstModule syntax;
        ast_module_init(&syntax, sources.allocator);
        assert(parser_module_parse(&syntax, source) == PARSE_OK);
        const AstNode *node = syntax.graph.first;
        assert(node != NULL && node->kind == kinds[index]);
        ProgramPlan plan = {0};
        Lowering lowering = {.plan = &plan};
        SemanticStatement statement = {.syntax = node};
        size_t before = allocation_count;
        assert(lower_statement(&lowering, &statement, NULL, NULL) == NULL);
        assert(plan.status == PLAN_INVALID && !plan.valid && plan.first == NULL && allocation_count == before);
        assert(diagnostic_valid(&plan.error) && plan.error.primary.span.source == source);
        assert(plan.error.primary.span.start == node->span.start && plan.error.primary.span.end == node->span.end);
        ast_module_destroy(&syntax);
        assert(diagnostic_valid(&plan.error));
        destroy_test();
    }
}
