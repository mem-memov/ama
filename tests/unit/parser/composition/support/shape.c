/** @file
 * @brief Validate cross-component AST spans and action aliases against the one original source.
 */
/** Every present span borrows exactly the admitted source and stays inside its byte extent. */
static void check_span(SourceSpan span, const SourceText *source) {
    if (span.source != NULL) { assert(span.source == source && source_span_valid(span)); }
    else { assert(span.start == 0 && span.end == 0); }
}
/** Finite expression worklists check original spans, ordered ownership and the
 * absence of fabricated operands on skips; capacity follows admitted bytes.
 */
static void check_expression(const AstTypeExpression *root, const SourceText *source) {
    if (root == NULL) { return; }
    size_t capacity = source_length(source) + 1, count = 1;
    const AstTypeExpression **queue = calloc(capacity, sizeof(*queue));
    assert(queue != NULL);
    queue[0] = root;
    for (size_t cursor = 0; cursor < count; cursor++) {
        const AstTypeExpression *expression = queue[cursor];
        check_span(expression->span, source);
        check_span(expression->head, source);
        assert(expression->head.start == expression->span.start);
        const AstStaticBinding *last = NULL;
        bool outputs = false;
        for (const AstStaticBinding *binding = expression->first; binding != NULL; binding = binding->next) {
            check_span(binding->span, source);
            check_span(binding->label, source);
            check_span(binding->marker, source);
            check_span(binding->receiver, source);
            if (binding->kind == AST_STATIC_INPUT) {
                assert(!outputs && binding->receiver.source == NULL);
                assert((binding->input == NULL) == binding->omitted);
                if (binding->input != NULL) {
                    assert(count < capacity);
                    queue[count++] = binding->input;
                }
            } else {
                assert(binding->kind == AST_STATIC_OUTPUT && binding->input == NULL);
                assert((binding->receiver.source == NULL) == binding->omitted);
                outputs = true;
            }
            assert(!binding->omitted || binding->positional);
            last = binding;
        }
        assert(last == expression->last);
    }
    free(queue);
}
/** Each signature owns its foreign names and finite nested signatures; no
 * resolution table is consulted to validate their source representation.
 */
static void check_formal(const AstStaticFormal *root, const SourceText *source) {
    size_t capacity = source_length(source) + 1, count = 1;
    const AstStaticFormal **queue = calloc(capacity, sizeof(*queue));
    assert(root != NULL && queue != NULL);
    queue[0] = root;
    for (size_t cursor = 0; cursor < count; cursor++) {
        const AstStaticFormal *formal = queue[cursor], *last = NULL;
        check_span(formal->span, source);
        check_span(formal->name, source);
        for (const AstStaticFormal *child = formal->first; child != NULL; child = child->next) {
            assert(count < capacity);
            queue[count++] = child;
            last = child;
        }
        assert(last == formal->last);
    }
    free(queue);
}
/** Nested named indices share source lifetime and never become reference-valued index expressions. */
static void check_access(const AstAccess *access, const SourceText *source) {
    bool index = false;
    for (; access != NULL; access = access->index) {
        check_span(access->span, source);
        check_span(access->name, source);
        assert(access->name.end > access->name.start);
        assert(!index || (!access->reference && !access->whole_list));
        index = true;
    }
}
/** Literal payload and constant/name views remain borrowed rather than regenerated text. */
static void check_source(const AstSource *value, const SourceText *source) {
    if (value == NULL) { return; }
    check_span(value->span, source);
    check_span(value->module_path, source);
    check_span(value->member, source);
    check_span(value->payload, source);
    check_access(value->access, source);
    if (value->kind == AST_LITERAL_SOURCE) {
        assert(value->payload.end == value->span.end && value->payload.start == value->span.start + 1);
    }
    if (value->kind == AST_SELF_REFERENCE_SOURCE) { assert(value->access == NULL); }
}
/** Queue finite source-owned nodes in source order, checking linked block endpoints. */
static void queue_block(AstBlock block, const AstNode **queue, size_t *count, size_t capacity) {
    const AstNode *last = NULL;
    for (const AstNode *node = block.first; node != NULL; node = node->next) {
        assert(*count < capacity);
        queue[(*count)++] = node;
        last = node;
    }
    assert(last == block.last);
}
/** Check ordinary direction ordinals independently from selected action order and aliases. */
static void check_call(const AstNode *node, const SourceText *source,
    const AstNode **queue, size_t *count, size_t capacity) {
    check_access(node->as.call.target, source);
    check_expression(node->as.call.namespace_expression, source);
    check_span(node->as.call.refusal_marker, source);
    size_t inputs = 0, outputs = 0, actions = 0;
    const AstArgument *last = NULL;
    for (const AstArgument *binding = node->as.call.first; binding != NULL; binding = binding->next) {
        check_span(binding->span, source);
        check_span(binding->marker, source);
        check_access(binding->label, source);
        check_access(binding->receiver, source);
        check_source(binding->source, source);
        size_t ordinal = binding->kind == AST_INPUT ? inputs++ : outputs++;
        assert(binding->position == ordinal);
        last = binding;
    }
    assert(last == node->as.call.last && inputs == node->as.call.input_count && outputs == node->as.call.output_count);
    const AstOutputAction *last_action = NULL;
    for (const AstOutputAction *action = node->as.call.first_output; action != NULL; action = action->next) {
        if (action->kind == AST_BINDING_ACTION) {
            bool found = false;
            for (const AstArgument *binding = node->as.call.first; binding != NULL; binding = binding->next) {
                if (binding == action->binding) { found = true; }
            }
            assert(found && action->binding->kind != AST_INPUT && action->operation == NULL);
            actions++;
        } else {
            assert(action->kind == AST_SELECTED_ACTION && action->binding == NULL);
            assert(action->operation->as.call.enclosing_operation == node);
            assert(*count < capacity);
            queue[(*count)++] = action->operation;
        }
        last_action = action;
    }
    assert(actions == outputs && last_action == node->as.call.last_output);
    queue_block(node->as.call.alternative, queue, count, capacity);
}
/** Traverse every structural subtree without C recursion, checking source and ownership handoffs. */
static void check_tree(const char *original) {
    const SourceText *source = module.layout.source;
    size_t length = strlen(original), capacity = length + 1, count = 0;
    assert(source_length(source) == length && memcmp(source_bytes(source), original, length) == 0);
    const AstNode **queue = calloc(capacity, sizeof(*queue));
    assert(queue != NULL);
    queue_block(module.constants, queue, &count, capacity);
    queue_block(module.parameters, queue, &count, capacity);
    queue_block(module.results, queue, &count, capacity);
    queue_block(module.body, queue, &count, capacity);
    for (size_t cursor = 0; cursor < count; cursor++) {
        const AstNode *node = queue[cursor];
        check_span(node->span, source);
        if (node->kind == AST_DECLARATION) {
            check_access(node->as.declaration.name, source);
            check_expression(node->as.declaration.type, source);
            size_t initializers = 0;
            const AstInitializer *last = NULL;
            for (const AstInitializer *item = node->as.declaration.first; item != NULL; item = item->next) {
                check_source(item->source, source);
                initializers++;
                last = item;
            }
            assert(initializers == node->as.declaration.initializer_count && last == node->as.declaration.last);
        } else if (node->kind == AST_STATIC_FORMAL) {
            check_formal(node->as.static_formal, source);
        } else if (node->kind == AST_STATIC_RESULT) {
            check_span(node->as.static_result.name, source);
            check_expression(node->as.static_result.expression, source);
        } else if (node->kind == AST_STATIC_PROJECTION) {
            check_expression(node->as.static_projection, source);
        } else if (node->kind == AST_ASSIGNMENT) {
            check_access(node->as.assignment.receiver, source);
            check_source(node->as.assignment.source, source);
        } else if (node->kind == AST_CALL) { check_call(node, source, queue, &count, capacity); }
        else if (node->kind == AST_LOOP || node->kind == AST_BRANCH) {
            check_access(node->as.control.condition, source);
            check_span(node->as.control.body_marker, source);
            check_span(node->as.control.alternative_marker, source);
            check_span(node->as.control.closing_marker, source);
            queue_block(node->as.control.body, queue, &count, capacity);
            queue_block(node->as.control.alternative, queue, &count, capacity);
            if (node->as.control.command != NULL) {
                assert(node->kind == AST_LOOP && count < capacity);
                queue[count++] = node->as.control.command;
                const AstNode *terminal = node->as.control.command;
                while (terminal->as.call.first_output != NULL) {
                    assert(terminal->as.call.first_output == terminal->as.call.last_output);
                    terminal = terminal->as.call.first_output->operation;
                }
                assert(terminal == node->as.control.condition_owner);
            }
        } else if (node->kind == AST_FOREACH) {
            check_span(node->as.traversal.collection, source);
            check_span(node->as.traversal.binder, source);
            check_span(node->as.traversal.body_marker, source);
            check_span(node->as.traversal.closing_marker, source);
            queue_block(node->as.traversal.body, queue, &count, capacity);
        } else { assert(node->kind == AST_RETURN || node->kind == AST_BREAK || node->kind == AST_SUSPEND); }
    }
    size_t previous = 0;
    for (const AstWarning *warning = module.warnings; warning != NULL; warning = warning->next) {
        assert(diagnostic_valid(&warning->diagnostic) && warning->diagnostic.primary.span.start > previous);
        previous = warning->diagnostic.primary.span.start;
    }
    free(queue);
}
