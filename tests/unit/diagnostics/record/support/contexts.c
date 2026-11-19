/** @file
 * @brief Check caller-owned diagnostic attribution, idempotence and malformed finite-chain refusal.
 * @see [T189 contract](../../../../../docs/implementation/specialization-diagnostics.md)
 */

/** Attribution supplements immutable primary/flat evidence and retains each actual caller without reallocating those notes. */
static void it_adds_caller_owned_context_without_replacing_primary_or_existing_related_notes(void) {
    setup_test();
    DiagnosticRelated original = {{.span = {source, 3, 9}}, "original definition", "商品"};
    Diagnostic diagnostic = {.code = DIAGNOSTIC_TYPE_MISMATCH, .message = "Primary failure",
        .primary = {.span = {source, 3, 9}}, .related = &original, .related_count = 1};
    DiagnosticContext first = {0}, second = {0};
    DiagnosticRelated requirement = {{.span = {source, 10, 20}}, "required here", ".shop.item"};
    assert(diagnostic_add_context(&diagnostic, &first, requirement));
    assert(diagnostic_add_context(&diagnostic, &second, requirement));
    assert(diagnostic.contexts == &second && second.next == &first && !first.next);
    assert(diagnostic.related == &original && diagnostic.related_count == 1);
    assert(diagnostic.primary.span.start == 3 && strcmp(diagnostic.message, "Primary failure") == 0);
    assert(diagnostic_add_context(&diagnostic, &first, requirement) && diagnostic.contexts == &second);
    DiagnosticRelated changed = requirement;
    changed.role = "different requirement";
    assert(!diagnostic_add_context(&diagnostic, &first, changed));
    assert(strcmp(first.related.role, "required here") == 0 && diagnostic.contexts == &second);
    changed.role = NULL;
    assert(!diagnostic_add_context(&diagnostic, &first, changed));
    DiagnosticContext untouched = {.related = {.role = "sentinel"}};
    requirement.location.span.end = source_length(source) + 1;
    assert(!diagnostic_add_context(&diagnostic, &untouched, requirement));
    assert(!untouched.next && strcmp(untouched.related.role, "sentinel") == 0 && diagnostic.contexts == &second);
    assert(!diagnostic_add_context(NULL, &untouched, original));
    assert(!diagnostic_add_context(&diagnostic, NULL, original));
    destroy_test();
}

/** A finite deep chain is valid; self and nontrivial cycles or missing roles are rejected without a depth budget. */
static void it_validates_deep_diagnostic_contexts_and_rejects_cycles_without_recursion(void) {
    setup_test();
    size_t count = 4096;
    DiagnosticContext *nodes = calloc(count, sizeof(*nodes));
    assert(nodes);
    for (size_t index = 0; index < count; index++) {
        nodes[index] = (DiagnosticContext){.related = {{.span = {source, 3, 9}}, "required here", NULL},
            .next = index ? &nodes[index - 1] : NULL};
    }
    Diagnostic diagnostic = {.message = "Primary failure", .contexts = &nodes[count - 1]};
    assert(diagnostic_valid(&diagnostic));
    nodes[0].next = &nodes[count / 2];
    assert(!diagnostic_valid(&diagnostic));
    DiagnosticContext destination = {0};
    assert(!diagnostic_add_context(&diagnostic, &destination, nodes[0].related));
    assert(!destination.next && !destination.related.role);
    nodes[0].next = NULL;
    nodes[count / 2].related.role = NULL;
    assert(!diagnostic_valid(&diagnostic));
    nodes[count / 2].related.role = "required here";
    assert(diagnostic_valid(&diagnostic));
    nodes[count - 1].next = &nodes[count - 1];
    assert(!diagnostic_valid(&diagnostic));
    free(nodes);
    destroy_test();
}
