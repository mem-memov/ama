/** @file
 * @brief Prepare current graph syntax and explicit scoped metadata for the request-only component boundary.
 */

/** The target graph has two free inputs and two public outputs. The fixture
 * supplies their independently known Type metadata through the real symbol
 * primitives; it does not claim to test the unfinished whole-graph inference.
 */
static BindingPlan binding_plan;
static StaticModule *producer;
static const AstTypeExpression *expression;

/** Parse the actual graph and freeze public/source identities before requests.
 * No source rewriting, legacy declaration or implicit default Type is used.
 */
static void setup_named_requests(const char *text) {
    setup_static(text, "@...\n...out\n...a\n...second\n...b\n");
    producer = static_module_add(&inventory, project_find(&project, ".other"));
    assert(producer != NULL && static_collect_explicit(&inventory, producer));
    const char *names[] = {"...a", "...b"};
    for (size_t index = 0; index < 2; index++) {
        SourceSpan name = static_name_site(producer->definition, names[index]);
        StaticSymbol *symbol = static_symbol_add(&inventory, producer, name, name,
            STATIC_PARAMETER, STATIC_CATEGORY_TYPE);
        assert(symbol != NULL);
        symbol->ordinal = index;
    }
    const AstNode *node = project.entry->syntax.graph.first;
    assert(node != NULL);
    expression = node->kind == AST_STATIC_RESULT ? node->as.static_result.expression
        : node->as.static_projection;
    binding_plan_init(&binding_plan, sources.allocator);
}

/** Source-interface preparation owns the frozen sequences in the same plan. */
static BindingInterface *named_interface(const StaticSymbol *const *captures, size_t count) {
    BindingInterface *view = NULL;
    assert(binding_source_interface(&binding_plan, producer, BINDING_MODULE_SURFACE,
        captures, count, &view));
    return view;
}

/** Destroy every normalized request/snapshot before its borrowed AST and symbols. */
static void destroy_named_requests(void) {
    binding_plan_destroy(&binding_plan);
    destroy_static();
}
