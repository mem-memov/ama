/** @file
 * @brief Compose current ordinary Type interfaces and closed graph pair identities.
 */

/** Input and output Type lines introduce the same category and remain separate
 * from runtime field records; repeated names resolve to the one formal.
 */
static void it_binds_ordinary_input_and_result_type_lines_without_static_runtime_fields(void) {
    const char *cases[] = {"@<\ninput&\n...type\n@!\nstate\n...type\n",
        "@>\noutput\n...type\n@!\nstate\n...type\n"};
    for (size_t index = 0; index < 2; index++) {
        setup_static(cases[index], STATIC_SCOPE_SOURCE_TEXT);
        assert(static_inventory_collect(&inventory) == STATIC_OK && inventory.valid && inventory.attempted);
        const StaticModule *module = static_module_find(&inventory, project.entry);
        assert(module->parameter_count == 1 && module->result_count == 0);
        assert(named("...type")->ordinal == 0 && named("...type")->category == STATIC_CATEGORY_TYPE);
        assert(module->constructor_signature->count == 1);
        for (const StaticOccurrence *site = module->first_occurrence; site != NULL; site = site->next) {
            assert(site->symbol == named("...type") && site->origin == NULL);
        }
        destroy_static();
    }
}

/** Closed pair definitions bind throughout the graph, including forward uses,
 * while their public order follows headers and never creates runtime fields.
 */
static void it_binds_closed_graph_pairs_in_public_order_without_runtime_storage(void) {
    setup_static("@...\n...forwarded\n...known\n...known\n.other\n", STATIC_SCOPE_SOURCE_TEXT);
    assert(static_inventory_collect(&inventory) == STATIC_OK && inventory.valid);
    const StaticModule *module = static_module_find(&inventory, project.entry);
    assert(module->parameter_count == 0 && module->result_count == 2);
    assert(named("...forwarded")->role == STATIC_RESULT && named("...forwarded")->ordinal == 0);
    assert(named("...known")->ordinal == 1 && named("...known")->category == STATIC_CATEGORY_TYPE);
    assert(named("...forwarded")->category == STATIC_CATEGORY_TYPE);
    assert(project.entry->syntax.body.first == NULL && project.entry->syntax.results.first == NULL);
    destroy_static();
}

/** A body-only placeholder cannot become an undeclared free formal. */
static void it_rejects_ordinary_body_only_placeholder_types_at_original_sources(void) {
    setup_static("@!\nvalue\n...unknown\n", STATIC_SCOPE_SOURCE_TEXT);
    assert(static_inventory_collect(&inventory) == STATIC_INVALID && !inventory.valid && inventory.attempted);
    assert(diagnostic_valid(&inventory.error));
    assert(layout_text_is(inventory.error.primary.span, "...unknown"));
    destroy_static();
}

/** A resolved nongeneric ordinary Type is not a constructor, even when its
 * application is structurally valid graph syntax. Refusal retains the written
 * input marker rather than inventing a missing formal or changing the AST.
 */
static void it_rejects_static_application_of_a_known_concrete_type(void) {
    setup_static("@...\n...result\n.other\n ...slot < .other\n", STATIC_SCOPE_SOURCE_TEXT);
    assert(static_inventory_collect(&inventory) == STATIC_INVALID && !inventory.valid);
    assert(diagnostic_valid(&inventory.error));
    assert(layout_text_is(inventory.error.primary.span, "<"));
    assert(strstr(inventory.error.message, "Type is not an applied constructor") != NULL);
    assert(project.entry->syntax.graph.first->as.static_result.expression->first != NULL);
    destroy_static();
}
