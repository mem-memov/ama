/** @file
 * @brief Check recursive signature categories, repeated patterns and exact known-cycle rejection.
 */

/** Explicit signatures and repeated patterns match complete named category sets;
 * reordered names agree, while missing/renamed/category changes fail.
 */
static void it_checks_explicit_and_repeated_inferred_recursive_signature_agreement(void) {
    const char *cases[] = {
        "@<\n...family\n ...slot\nvalue\n...family\n ...slot < ...type\n@!\n",
        "@<\nfirst\n...family\n ...a < ...type\n ...b < ...type\nsecond\n...family\n ...b < ...type\n ...a < ...type\n@!\n",
        "@<\n...family\n ...slot\nvalue\n...family\n ...wrong < ...type\n@!\n",
        "@<\n...family\n ...slot\nvalue\n...family\n ...slot < ...type\n ...extra < ...type\n@!\n",
        "@<\n...family\n ...slot\n...other family\n ...nested\nvalue\n...family\n ...slot < ...other family\n@!\n",
        "@<\nfirst\n...family\n ...a < ...type\nsecond\n...family\n ...b < ...type\n@!\n",
        "@<\nvalue\n...family\n ...slot < ...type\n ...slot < ...type\n@!\n"
    };
    for (size_t index = 0; index < 7; index++) {
        setup_static(cases[index], STATIC_SCOPE_SOURCE_TEXT);
        assert(static_inventory_collect(&inventory) == (index < 2 ? STATIC_OK : STATIC_INVALID));
        assert(inventory.valid == (index < 2));
        if (index < 2) {
            const StaticModule *module = static_module_find(&inventory, project.entry);
            assert(module->first_pattern != NULL && !module->first_pattern->pending);
        } else { assert(diagnostic_valid(&inventory.error)); }
        destroy_static();
    }
}

/** Cycles arise from actual Template category edges, not repeated shared nodes.
 * Explicit finite foreign trees remain distinct from local recursive names.
 */
static void it_rejects_infinite_known_signatures_but_accepts_shared_acyclic_metadata(void) {
    const char *cases[] = {
        "@<\nvalue\n...family\n ...slot < ...family\n@!\n",
        "@<\nfirst\n...one\n ...slot < ...two\nsecond\n...two\n ...slot < ...one\n@!\n",
        "@<\n...leaf\n ...type\nfirst\n...one\n ...a < ...leaf\n ...b < ...leaf\nsecond\n...two\n ...slot < ...one\n@!\n"
    };
    for (size_t index = 0; index < 3; index++) {
        setup_static(cases[index], STATIC_SCOPE_SOURCE_TEXT);
        assert(static_inventory_collect(&inventory) == (index == 2 ? STATIC_OK : STATIC_INVALID));
        assert(inventory.valid == (index == 2));
        if (index != 2) { assert(strstr(inventory.error.message, "Cyclic") != NULL); }
        destroy_static();
    }
    setup_static("@<\nvalue\n...family\n ...slot < .other\n@!\n", "@<\n...element\n@!\n");
    assert(static_inventory_collect(&inventory) == STATIC_OK && inventory.valid);
    const StaticSignatureEntry *slot = named("...family")->signature->first;
    assert(slot->category == STATIC_CATEGORY_TEMPLATE && slot->signature->count == 1);
    assert(layout_text_is(slot->signature->first->name, "...element"));
    destroy_static();
}

/** Applied residuals require the next normalization stage; projected output
 * categories require known producer metadata in T179. Both retain the exact
 * expression rather than becoming Type by default or an equality wildcard.
 */
static void it_retains_applied_and_projected_metadata_as_explicit_pending_obligations(void) {
    setup_static("@<\n...nested family\n ...element\nfirst\n...outer\n"
        " ...slot < ...nested family\n  ...element < ...type\n"
        "second\n.other\n ...out > ...received\nthird\n...projected family\n ...slot < ...received\n@!\n", STATIC_SCOPE_SOURCE_TEXT);
    assert(static_inventory_collect(&inventory) == STATIC_OK && inventory.valid);
    const StaticModule *module = static_module_find(&inventory, project.entry);
    assert(module->first_pattern->pending && module->last_pattern->pending);
    assert(named("...outer")->signature->first->category == STATIC_CATEGORY_DEFERRED);
    assert(named("...outer")->signature->first->pending_expression != NULL);
    assert(named("...projected family")->signature->first->category == STATIC_CATEGORY_DEFERRED);
    assert(named("...received")->role == STATIC_ALIAS);
    destroy_static();
}

/** An output phase permits omitted producer inputs uniformly; full root-input
 * coverage applies only without that phase. Actual closure remains T179's task.
 */
static void it_allows_partial_known_constructor_inputs_when_outputs_request_closure(void) {
    const char *cases[] = {
        "@<\n...family\n ...a\n ...b\nvalue\n...family\n ...a < ...type\n ... >\n@!\n",
        "@<\n...family\n ...a\n ...b\nvalue\n...family\n ...a < ...type\n@!\n",
        "@<\nfirst\n...family\n ...a < ...type\n ...b < ...type\nsecond\n...family\n ...a < ...type\n ... >\n@!\n"
    };
    for (size_t index = 0; index < 3; index++) {
        setup_static(cases[index], STATIC_SCOPE_SOURCE_TEXT);
        assert(static_inventory_collect(&inventory) == (index == 1 ? STATIC_INVALID : STATIC_OK));
        if (index != 1) {
            const StaticModule *module = static_module_find(&inventory, project.entry);
            assert(module->last_pattern->output_phase && !module->last_pattern->pending);
        }
        destroy_static();
    }
}

/** Concrete constructor names select their exact foreign slot set without
 * importing that set as local formals or searching by compatible shape.
 */
static void it_checks_concrete_constructor_patterns_without_importing_foreign_formals(void) {
    const char *cases[] = {
        "@<\nvalue\n.other\n ...slot < ...local\n@!\n",
        "@<\nvalue\n.other\n ...wrong < ...local\n@!\n"
    };
    for (size_t index = 0; index < 2; index++) {
        setup_static(cases[index], "@<\n...slot\n@!\n");
        assert(static_inventory_collect(&inventory) == (index == 0 ? STATIC_OK : STATIC_INVALID));
        if (index == 0) {
            const StaticModule *module = static_module_find(&inventory, project.entry);
            assert(module->parameter_count == 1 && named("...local")->category == STATIC_CATEGORY_TYPE);
            assert(module->first_pattern->occurrence->origin == project_find(&project, ".other"));
            assert(static_symbol_find(module, module->first_pattern->signature->first->name, SIZE_MAX) == NULL);
        }
        destroy_static();
    }
}

/** An actual deeply nested explicit signature crosses parser, inventory and
 * known-cycle traversal without consuming one C activation per source level.
 */
static void it_collects_deep_real_signature_definitions_without_recursive_c_activation(void) {
    size_t depth = 700;
    const char *head = "@<\n...family\n", *slot = "...slot\n", *tail = "@!\n";
    size_t size = strlen(head) + depth * strlen(slot) + depth * (depth + 1) / 2 + strlen(tail) + 1;
    char *text = malloc(size);
    assert(text != NULL);
    size_t used = strlen(head);
    memcpy(text, head, used);
    for (size_t level = 1; level <= depth; level++) {
        memset(text + used, ' ', level);
        used += level;
        memcpy(text + used, slot, strlen(slot));
        used += strlen(slot);
    }
    memcpy(text + used, tail, strlen(tail) + 1);
    used += strlen(tail);
    assert(used + 1 == size);
    setup_static(text, STATIC_SCOPE_SOURCE_TEXT);
    assert(static_inventory_collect(&inventory) == STATIC_OK && inventory.valid);
    const StaticSignature *signature = named("...family")->signature;
    for (size_t level = 0; level < depth; level++) {
        assert(signature != NULL && signature->count == 1);
        signature = signature->first->signature;
    }
    assert(signature == NULL);
    destroy_static();
    free(text);
}
