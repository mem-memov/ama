/** @file
 * @brief Check lexical local intervals, compatible cell reuse and construction-time exclusions.
 */
/** Count actual distinct introduced cells, independently of the number of lexical traversal roles. */
static size_t local_count(void) {
    size_t count = 0;
    for (SemanticSymbol *local = semantic_find_type(&semantic,".main")->locals; local != NULL; local = local->next) { count++; }
    return count;
}
/** Disjoint bodies own distinct cells; a later persistent field cannot rewrite those earlier lexical bindings. */
static void it_scopes_disjoint_local_references_without_retroactive_module_binding(void) {
    prepare_inference();
    valid_inference("@!\nowned\n.value\nitems[]\n.value\nknown&\n.value\n &owned\n"
        "items[item&]\n{\n escaped&\n  item&\n}\nitems[item&]\n{\n copy\n  % item&\n}\n"
        "item&\n known&\nitems[item&]\n{\n item&\n  known&\n}\n");
    SemanticForeachBinding *first = semantic_find_type(&semantic,".main")->foreach_bindings;
    SemanticForeachBinding *second = first->next, *third = second->next;
    assert(first->introduced && second->introduced && !third->introduced && third->next == NULL);
    assert(first->binding != second->binding && local_count() == 2);
    assert(first->binding->local && second->binding->local && !field("item")->local && field("item")->inferred);
    assert(third->binding == field("item") && field("escaped")->reference && !field("copy")->reference);
    const AstNode *first_assignment = first->syntax->as.traversal.body.first;
    SourceSpan first_use = first_assignment->as.assignment.source->access->name;
    assert(semantic_local_at(semantic_find_type(&semantic,".main"), first->binding->name, first_use) == first->binding);
    assert(semantic_local_at(semantic_find_type(&semantic,".main"), first->binding->name, field("item")->site) == NULL);
    assert(field("escaped")->type == first->binding->type && field("copy")->type == second->binding->type);
    destroy_semantic();
}
/** Nested reuse aliases the same cell, and a later explicit compatible reference is already visible at its earlier header. */
static void it_reuses_enclosing_or_later_explicit_references_without_shadowing(void) {
    prepare_inference();
    valid_inference("@!\nitems[]\n.value\nitems[item&]\n{\n items[item&]\n {\n  escaped&\n   item&\n }\n}\n");
    SemanticForeachBinding *outer = semantic_find_type(&semantic,".main")->foreach_bindings;
    SemanticForeachBinding *inner = outer->next;
    assert(outer->introduced && !inner->introduced && outer->binding == inner->binding && local_count() == 1);
    assert(inner->binding->scope == outer->syntax);
    destroy_semantic();
    valid_inference("@!\nitems[]\n.value\nitems[item&]\n{\n escaped&\n  item&\n}\nitem&\n.value\n");
    outer = semantic_find_type(&semantic,".main")->foreach_bindings;
    assert(!outer->introduced && outer->binding == field("item") && !outer->binding->inferred && local_count() == 0);
    destroy_semantic();
    valid_inference("@!\nitems[]\n.value\ncopies[]\n items[]\ncopies[item&]\n{\n %\n}\n");
    assert(field("copies")->inferred && semantic_find_type(&semantic,".main")->foreach_bindings->collection == field("copies"));
    destroy_semantic();
    valid_inference("@!\nworker\n.worker\nitems[]\n.value\nitems[item&]\n{\n worker !\n  >\n  > item&\n}\n");
    assert(field("item") == NULL && local_count() == 1);
    assert(semantic_find_type(&semantic,".main")->foreach_bindings->binding->local);
    destroy_semantic();
}
/** Empty/skipped traversals still reject invalid storage/types, shadow attempts and instance-construction use of runtime locals. */
static void it_rejects_incompatible_bindings_and_construction_time_local_access(void) {
    prepare_inference();
    const char *invalid[] = {
        "@!\nitems[]\n.value\nitem\n.value\nitems[item&]\n{\n}\n",
        "@!\nitems[]\n.value\nitem&[]\n.value\nitems[item&]\n{\n}\n",
        "@!\nitems[]\n.value\nitem&\n.other\nitems[item&]\n{\n}\n",
        "@!\nitems[]\n.value\nother items[]\n.other\nitems[item&]\n{\n other items[item&]\n {\n }\n}\n",
        "@!\nitems[]\n.value\nitems[item&]\n{\n explicit&\n .value\n  item&\n}\n",
        "@!\nitems[]\n.value\nitems[item&]\n{\n}\noutside&\n item&\n",
        "@=\nitems[]\n.value\n@!\nitems[item&]\n{\n}\n",
        "@!\nitems[]\n.value\nowned\n.value\nitems[item&]\n{\n item\n  owned\n}\n"
    };
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]); index++) {
        SemanticStatus status = infer_text(invalid[index]);
        assert(status == SEMANTIC_INVALID && !semantic.inference_valid && diagnostic_valid(&semantic.error));
        if (index == 4) { assert(layout_text_is(semantic.error.primary.span,"item")); }
        destroy_semantic();
    }
}
