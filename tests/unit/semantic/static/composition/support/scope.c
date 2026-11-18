/** @file
 * @brief Verify actual static lexical identities across interfaces, results and runtime blocks.
 */

/** New output-only interface heads and explicit controls have identical Type
 * behavior. Alias category stays deferred until actual producer output metadata.
 */
static void it_binds_output_only_type_heads_and_retains_actual_alias_obligations(void) {
    const char *cases[] = {
        "@<\ninput&\n...type\n ...out > ...received\n@!\n",
        "@<\ninput&\n...type\n ... >\n@!\n",
        "@<\n...type\ninput&\n...type\n ...out > ...received\n@!\n",
        "@<\n...type\ninput&\n...type\n ... >\n@!\n",
        "@>\noutput\n...type\n ... >\n@!\n"
    };
    for (size_t index = 0; index < 5; index++) {
        setup_static(cases[index], STATIC_SCOPE_SOURCE_TEXT);
        assert(static_inventory_collect(&inventory) == STATIC_OK && inventory.valid && inventory.attempted);
        const StaticModule *module = static_module_find(&inventory, project.entry);
        assert(module->parameter_count == 1 && named("...type")->ordinal == 0);
        assert(named("...type")->category == STATIC_CATEGORY_TYPE);
        for (const StaticOccurrence *site = module->first_occurrence; site != NULL; site = site->next) {
            assert(site->symbol != NULL && site->origin == NULL);
            if (site->receiver != NULL) {
                assert(site->symbol->role == STATIC_ALIAS && site->symbol->category == STATIC_CATEGORY_DEFERRED);
                assert(site->symbol->receiver == site->receiver);
            }
        }
        destroy_static();
    }
}

/** Static results may refer to later declared types; exact result order differs
 * from parameter order. A foreign slot spelling does not capture the local.
 */
static void it_binds_result_definitions_and_foreign_names_without_runtime_storage(void) {
    setup_static("@<\n...family\n ...foreign\n@>\n...result\n...type\noutput\n...type\n"
        "...constructor result\n...family\n@!\n", STATIC_SCOPE_SOURCE_TEXT);
    assert(static_inventory_collect(&inventory) == STATIC_OK && inventory.valid);
    const StaticModule *module = static_module_find(&inventory, project.entry);
    assert(module->parameter_count == 2 && module->result_count == 2);
    assert(named("...result")->role == STATIC_RESULT && named("...result")->ordinal == 0);
    assert(named("...result")->category == STATIC_CATEGORY_TYPE);
    assert(named("...constructor result")->ordinal == 1);
    assert(named("...constructor result")->signature == named("...family")->signature);
    assert(static_symbol_find(module, static_name_site(project.entry, "...foreign"), SIZE_MAX) == NULL);
    assert(module->constructor_signature->count == 2 && named("...type")->ordinal == 1);
    assert(project.entry->syntax.body.first == NULL);
    destroy_static();
}

/** Earlier alias sources, result-only unknown constructors, applied Types and
 * body-only placeholders fail at actual source spans without inventing formals.
 */
static void it_rejects_body_only_names_and_sources_before_alias_reception(void) {
    const char *cases[] = {
        "@!\nvalue\n...unknown\n",
        "@!\nvalue\n...alias\n.other\n ...out > ...alias\n",
        "@!\n...alias\n ...out > ...alias\n",
        "@>\noutput\n...unknown\n ...slot < ...type\n@!\n",
        "@<\n...type\nvalue\n...type\n ...slot < ...other\n@!\n",
        "@>\n...result\n.other\nvalue\n...result\n ...slot < .other\n@!\n"
    };
    for (size_t index = 0; index < 6; index++) {
        setup_static(cases[index], STATIC_SCOPE_SOURCE_TEXT);
        assert(static_inventory_collect(&inventory) == STATIC_INVALID && !inventory.valid && inventory.attempted);
        assert(diagnostic_valid(&inventory.error));
        assert(inventory.error.primary.span.source == project.entry->source->text);
        destroy_static();
    }
}

/** Static branch nesting changes neither alias scope nor execution: all names
 * are collected once without evaluating a runtime condition or native hook.
 */
static void it_preserves_aliases_received_inside_runtime_blocks_for_later_uses(void) {
    setup_static("@!\ncondition\n?\n .other\n  ...out > ...alias\nvalue\n...alias\n", STATIC_SCOPE_SOURCE_TEXT);
    assert(static_inventory_collect(&inventory) == STATIC_OK && inventory.valid);
    const StaticSymbol *alias = named("...alias");
    assert(alias->role == STATIC_ALIAS && alias->category == STATIC_CATEGORY_DEFERRED);
    const StaticModule *module = static_module_find(&inventory, project.entry);
    assert(module->parameter_count == 0);
    assert(module->last_occurrence->symbol == alias);
    assert(module->first_occurrence->origin == project_find(&project, ".other"));
    destroy_static();
}
