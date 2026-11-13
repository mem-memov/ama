/** @file
 * @brief Compare exact semantic projections across independent owners without bytewise struct comparisons, pointer hashes or reconstructed syntax.
 */
/** Symbols are compared by real owner/name/scope/type metadata, never allocation address or name alone. */
static void compare_symbol(const SemanticSymbol *left, const SemanticSymbol *right) {
    if (left == NULL || right == NULL) { assert(left == right); return; }
    assert(left->owner->module == right->owner->module && same_span(left->name, right->name));
    assert(same_type(left->type, right->type) && left->scope == right->scope && left->role == right->role);
    assert(left->reference == right->reference && left->list == right->list && left->local == right->local);
    assert(left->inferred == right->inferred && left->ordinal == right->ordinal && left->role_ordinal == right->role_ordinal);
}
/** Access chains retain exact borrowed syntax and their independent caller/result symbols. */
static void compare_access(const SemanticAccess *left, const SemanticAccess *right) {
    for (; left != NULL && right != NULL; left = left->index, right = right->index) {
        assert(left->syntax == right->syntax && left->reference == right->reference && left->list == right->list);
        compare_symbol(left->symbol, right->symbol);
    }
    assert(left == NULL && right == NULL);
}
/** Both authored and intrinsic sources preserve their complete kind/type/form/site rather than only payload text. */
static void compare_source(const SemanticSource *left, const SemanticSource *right) {
    if (left == NULL || right == NULL) { assert(left == right); return; }
    assert(left->syntax == right->syntax && left->kind == right->kind && same_span(left->site, right->site));
    assert(same_type(left->type, right->type) && left->reference == right->reference && left->list == right->list);
    compare_symbol(left->declaration, right->declaration);
    compare_symbol(left->constant, right->constant);
    compare_access(left->access, right->access);
}
/** A null parent stays null; actual parent/root/exit identity is its original unique AST node. */
static const AstNode *syntax_of(const SemanticStatement *statement) { return statement == NULL ? NULL : statement->syntax; }
/** Check every ordered statement, binding/action and resolved control role in the independent staged owner. */
static void compare_statements(const SemanticStatement *left, const SemanticStatement *right) {
    for (; left != NULL && right != NULL; left = left->next, right = right->next) {
        assert(left->syntax == right->syntax && left->operation_kind == right->operation_kind);
        assert(same_type(left->namespace_type, right->namespace_type) && left->transfer == right->transfer);
        assert(syntax_of(left->enclosing_operation) == syntax_of(right->enclosing_operation));
        assert(syntax_of(left->enclosing_loop) == syntax_of(right->enclosing_loop));
        assert(syntax_of(left->loop_root) == syntax_of(right->loop_root));
        assert(syntax_of(left->condition_owner) == syntax_of(right->condition_owner));
        assert(left->empty_body == right->empty_body && left->negative_branch == right->negative_branch);
        assert(left->intrinsic_target == right->intrinsic_target && left->intrinsic_result == right->intrinsic_result);
        assert(left->target_reference == right->target_reference && left->list_method == right->list_method);
        assert(same_type(left->list_interface.element, right->list_interface.element));
        assert(same_type(left->list_interface.integer, right->list_interface.integer));
        assert(left->list_interface.reference == right->list_interface.reference);
        assert(syntax_of(left->list_service) == syntax_of(right->list_service));
        compare_access(left->access, right->access);
        compare_source(left->source, right->source);
        assert(same_type(left->condition.type, right->condition.type) && left->condition.reference == right->condition.reference);
        assert(left->condition.list == right->condition.list && left->condition.intrinsic == right->condition.intrinsic);
        assert(left->condition.position == right->condition.position);
        compare_symbol(left->condition.field, right->condition.field);
        compare_access(left->condition.access, right->condition.access);
        if (left->traversal == NULL || right->traversal == NULL) { assert(left->traversal == right->traversal); }
        else {
            assert(left->traversal->syntax == right->traversal->syntax && left->traversal->introduced == right->traversal->introduced);
            compare_symbol(left->traversal->collection, right->traversal->collection);
            compare_symbol(left->traversal->binding, right->traversal->binding);
        }
        const SemanticBinding *a = left->bindings, *b = right->bindings;
        for (; a != NULL && b != NULL; a = a->next, b = b->next) {
            assert(a->syntax == b->syntax && a->intrinsic == b->intrinsic && same_type(a->type, b->type));
            assert(a->position == b->position && a->reference == b->reference && a->list == b->list && a->transfer == b->transfer);
            compare_symbol(a->field, b->field);
            compare_source(a->source, b->source);
            compare_access(a->receiver, b->receiver);
        }
        assert(a == NULL && b == NULL);
        const SemanticOutputAction *x = left->actions, *y = right->actions;
        for (; x != NULL && y != NULL; x = x->next, y = y->next) {
            assert(x->syntax == y->syntax && syntax_of(x->operation) == syntax_of(y->operation));
            assert((x->binding == NULL ? NULL : x->binding->syntax) == (y->binding == NULL ? NULL : y->binding->syntax));
            assert((x->binding == NULL) != (x->operation == NULL));
        }
        assert(x == NULL && y == NULL);
    }
    assert(left == NULL && right == NULL);
}
/** Compare complete type inventories, ABI binding spans, initializers and startup/reverse orders exactly. */
static void compare_programs(const SemanticProgram *left, const SemanticProgram *right) {
    assert(left->constants_valid && right->constants_valid && left->constant_count == right->constant_count);
    const SemanticType *a = left->first, *b = right->first;
    for (; a != NULL && b != NULL; a = a->next, b = b->next) {
        assert(a->module == b->module && a->executable == b->executable && a->cloneable == b->cloneable);
        assert(same_type(a->clone_seed, b->clone_seed));
        compare_symbol(a->clone_cause, b->clone_cause);
        assert(a->requires_size_bridge == b->requires_size_bridge && a->symbol_count == b->symbol_count);
        for (size_t role = 0; role < 4; role++) { assert(a->counts[role] == b->counts[role]); }
        for (size_t slot = 0; slot < ABI_BINDING_COUNT; slot++) {
            assert(a->abi.bindings[slot].present == b->abi.bindings[slot].present);
            assert(same_span(a->abi.bindings[slot].function, b->abi.bindings[slot].function));
        }
        const SemanticSymbol *x = a->first, *y = b->first;
        for (; x != NULL && y != NULL; x = x->next, y = y->next) {
            compare_symbol(x, y);
            assert(x->declaration == y->declaration && same_span(x->site, y->site) && same_span(x->type_site, y->type_site) && same_span(x->spelling, y->spelling));
            const SemanticInitializer *i = x->initializers, *j = y->initializers;
            for (; i != NULL && j != NULL; i = i->next, j = j->next) { assert(i->transfer == j->transfer); compare_source(i->source, j->source); }
            assert(i == NULL && j == NULL);
        }
        assert(x == NULL && y == NULL);
        const SemanticSymbol *local_left = a->locals, *local_right = b->locals;
        for (; local_left != NULL && local_right != NULL; local_left = local_left->next, local_right = local_right->next) {
            compare_symbol(local_left, local_right);
        }
        assert(local_left == NULL && local_right == NULL);
        const SemanticForeachBinding *role_left = a->foreach_bindings, *role_right = b->foreach_bindings;
        for (; role_left != NULL && role_right != NULL; role_left = role_left->next, role_right = role_right->next) {
            assert(role_left->syntax == role_right->syntax && role_left->introduced == role_right->introduced);
            compare_symbol(role_left->collection, role_right->collection);
            compare_symbol(role_left->binding, role_right->binding);
        }
        assert(role_left == NULL && role_right == NULL);
        compare_statements(a->statements, b->statements);
    }
    assert(a == NULL && b == NULL);
    const SemanticConstantPlan *x = left->constant_first, *y = right->constant_first;
    for (; x != NULL && y != NULL; x = x->next, y = y->next) {
        assert(x->ordinal == y->ordinal);
        compare_symbol(x->symbol, y->symbol);
        assert((x->previous == NULL ? 0 : x->previous->ordinal + 1) == x->ordinal);
        assert((y->previous == NULL ? 0 : y->previous->ordinal + 1) == y->ordinal);
    }
    assert(x == NULL && y == NULL);
}

/** Optional diagnostic prose is compared exactly; absence stays distinct from an empty invented message. */
static bool same_text(const char *left, const char *right) {
    return left == NULL || right == NULL ? left == right : strcmp(left, right) == 0;
}
/** Independent analysis retains the identical first failure category, all original spans and every related role/name. */
static void compare_diagnostics(const Diagnostic *left, const Diagnostic *right) {
    assert(left->code == right->code && same_text(left->message, right->message));
    assert(same_text(left->subject, right->subject) && same_text(left->expected, right->expected));
    assert(same_text(left->cause, right->cause) && same_text(left->help, right->help));
    assert(same_span(left->primary.span, right->primary.span) && same_text(left->primary.path, right->primary.path));
    assert(left->related_count == right->related_count);
    for (size_t index = 0; index < left->related_count; index++) {
        const DiagnosticRelated *a = &left->related[index], *b = &right->related[index];
        assert(same_span(a->location.span, b->location.span) && same_text(a->location.path, b->location.path));
        assert(same_text(a->role, b->role) && same_text(a->name, b->name));
    }
}
