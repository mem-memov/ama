/** @file
 * @brief Verify foreach lexical intervals, exact reuse and caller-side selector/index occurrences.
 */
#include "../common.c"

/** New outer/sibling locals stay distinct; inner reuse updates the outer cell.
 * Local occurrences do not prevent a later fresh module field with the same name.
 */
static void it_fixes_nested_disjoint_and_reused_foreach_bindings_without_shadowing(void) {
    setup_connections("@!\nknown\n//integer\nvalues[]\n//integer\nreused&\n//integer\nvalues[reused&]\n{\n}\n"
        "values[item&]\n{\n values[item&]\n {\n  copied&\n   item&\n }\n}\nvalues[item&]\n{\n}\nitem&\n &known\nvalues[item&]\n{\n}\n", "@!\n");
    const StaticRuntimeModule *module;
    assert(static_connection_runtime(&connections, caller_module, &module));
    assert(module->foreach_count == 5 && module->field_count == 7 && module->receiver_count == 2);
    const StaticRuntimeForeach *reused = module->first_foreach, *outer = reused->next, *inner = outer->next,
        *sibling = inner->next, *last = sibling->next;
    assert(!reused->introduced && reused->binding->scope == NULL);
    assert(outer->introduced && outer->binding->scope == outer->syntax);
    assert(!inner->introduced && inner->binding == outer->binding);
    assert(sibling->introduced && sibling->binding != outer->binding && sibling->binding->scope == sibling->syntax);
    assert(!last->introduced && last->binding->scope == NULL && last->binding->inferred);
    assert(module->last_receiver->introduced && module->last_receiver->field == last->binding);
    assert(module->first_receiver->introduced && module->first_receiver->field->scope == NULL);
    assert(static_runtime_field(module, outer->binding->name) == last->binding);
    assert(static_runtime_local(module, outer->binding->name, inner->syntax->as.traversal.binder) == outer->binding);
    assert(static_runtime_local(module, outer->binding->name, last->syntax->as.traversal.binder) == NULL);
    destroy_connections();
}

/** Static lookup applies even to empty bodies; out-of-scope or construction-time
 * local reads and incompatible visible bindings cannot become implicit fields.
 */
static void it_rejects_local_escape_construction_reads_and_incompatible_foreach_shadowing(void) {
    const char *sources[] = {
        "@!\nvalues[]\n//integer\nvalues[item&]\n{\n}\ncopied&\n item&\n",
        "@!\nvalues[]\n//integer\nvalues[item&]\n{\n captured&\n //integer\n  item&\n}\n",
        "@!\nvalues[]\n//integer\nitem\n//integer\nvalues[item&]\n{\n}\n",
        "@=\nvalues[]\n//integer\n@!\nvalues[item&]\n{\n}\n",
        "@!\nvalues&[]\n//integer\nvalues[item&]\n{\n}\n"
    };
    for (size_t index = 0; index < sizeof(sources) / sizeof(*sources); index++) {
        setup_connections(sources[index], "@!\n");
        const StaticRuntimeModule *module = NULL;
        assert(!static_connection_runtime(&connections, caller_module, &module));
        assert(module == NULL && connections.status == STATIC_CONNECTION_CONFLICT && diagnostic_valid(&connections.error));
        destroy_connections();
    }
}

/** Selector/label base names stay foreign, but their index names resolve in the
 * initiating caller and cannot silently become new inferred module variables.
 */
static void it_binds_caller_indices_and_excludes_selected_result_base_names(void) {
    setup_connections("@!\nindex\n//integer\nworker\n/test/fixture/v1.0.0/src/other\nworker !\n selected[index] !\n  label > label\n"
        "label\n index\n", "@!\n");
    const StaticRuntimeModule *module;
    assert(static_connection_runtime(&connections, caller_module, &module));
    assert(module->receiver_count == 2 && module->first_receiver->introduced && !module->last_receiver->introduced);
    assert(module->first_receiver->field == module->last_receiver->field && layout_text_is(module->first_receiver->field->name, "label"));
    bool found = false;
    for (const StaticRuntimeUse *use = module->first_use; use; use = use->next) {
        assert(!layout_text_is(use->field->name, "selected"));
        if (use->index) { assert(layout_text_is(use->field->name, "index")); found = true; }
    }
    assert(found && module->field_count == 3 && solver.equation_count == 0);
    destroy_connections();
}

/** Run exact scope/reuse, invalid lifetime and selector-index classification. */
static void run(void) {
    it_fixes_nested_disjoint_and_reused_foreach_bindings_without_shadowing();
    it_rejects_local_escape_construction_reads_and_incompatible_foreach_shadowing();
    it_binds_caller_indices_and_excludes_selected_result_base_names();
}
/** Return only after every assertion and counted teardown. */
int main(void) { run(); return 0; }
