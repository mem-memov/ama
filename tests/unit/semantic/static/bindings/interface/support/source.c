/** @file
 * @brief Verify original source ordinals, exact capture identity and disjoint interface surfaces.
 */

/** Each module kind exposes only its own surfaces. Dense internal ordinals
 * preserve original identities without creating positional static syntax.
 */
static void it_freezes_independent_ordinary_and_graph_interface_sequences(void) {
    for (size_t graph = 0; graph < 2; graph++) {
        setup_binding(graph ? graph_source : interface_source, STATIC_SCOPE_SOURCE_TEXT);
        const StaticModule *module = binding_module(project.entry);
        BindingInterface *view = NULL;
        assert(binding_source_interface(&binding_plan, module, BINDING_MODULE_SURFACE, NULL, 0, &view));
        assert(view == binding_plan.first_interface && view == binding_plan.last_interface);
        assert(view->kind == BINDING_SOURCE_INTERFACE && view->origin == module);
        const size_t arities[][4] = {{4,0,3,2},{2,2,0,0}};
        const char *ordinary[][4] = {{"...explicit type", "...implicit input", "...factory", "...implicit result"},
            {NULL}, {"first", "second", "factory"}, {"runtime", "introduced"}};
        const char *static_names[][4] = {{"...implicit result", "...explicit type"},
            {"...static one", "...static two"}, {NULL}, {NULL}};
        for (size_t direction = 0; direction < BINDING_SEQUENCE_COUNT; direction++) {
            const BindingSequence *sequence = &view->sequences[direction];
            assert(sequence->count == arities[graph][direction]);
            const BindingSlot *slot = sequence->first;
            for (size_t ordinal = 0; ordinal < sequence->count; ordinal++) {
                const char *name = graph ? static_names[direction][ordinal] : ordinary[direction][ordinal];
                assert(slot != NULL && name_is(slot->name, name));
                assert(slot->original_ordinal == ordinal && slot->sequence_ordinal == ordinal);
                assert(source_span_valid(slot->name.site) && slot->intrinsic_field == NULL);
                if (direction < BINDING_RUNTIME_INPUT) {
                    assert(slot->static_symbol == symbol(module, name) && slot->runtime_declaration == NULL);
                } else { assert(slot->runtime_declaration != NULL && slot->static_symbol == NULL); }
                slot = slot->next;
            }
            assert(slot == NULL);
            if (sequence->count == 0) { assert(sequence->first == NULL && sequence->last == NULL); }
            else { assert(sequence->last->next == NULL); }
        }
        if (!graph) {
            assert(view->sequences[BINDING_RUNTIME_INPUT].first->reference);
            assert(view->sequences[BINDING_RUNTIME_INPUT].first->next->list);
            assert(symbol(module, "...factory")->category == STATIC_CATEGORY_TYPE);
            assert(symbol(module, "...factory")->signature == NULL);
        }
        destroy_binding();
    }
}

/** Capture enumeration order never becomes positional order. Previously
 * published snapshots and source symbol ordinals remain immutable.
 */
static void it_filters_exact_fixed_identities_without_reordering_or_mutating_other_snapshots(void) {
    setup_binding(interface_source, STATIC_SCOPE_SOURCE_TEXT);
    const StaticModule *module = binding_module(project.entry);
    const StaticSymbol *captures[] = {symbol(module, "...factory"), symbol(module, "...implicit input")};
    BindingInterface *all = NULL, *filtered = NULL, *reversed = NULL;
    assert(binding_source_interface(&binding_plan, module, BINDING_MODULE_SURFACE, NULL, 0, &all));
    const BindingSlot *original = all->sequences[BINDING_STATIC_INPUT].first;
    assert(binding_source_interface(&binding_plan, module, BINDING_MODULE_SURFACE, captures, 2, &filtered));
    const StaticSymbol *reverse[] = {captures[1], captures[0]};
    assert(binding_source_interface(&binding_plan, module, BINDING_MODULE_SURFACE, reverse, 2, &reversed));
    for (const BindingInterface *view = filtered; view != NULL; view = view->next) {
        const BindingSequence *sequence = &view->sequences[BINDING_STATIC_INPUT];
        assert(sequence->count == 2 && sequence->first->original_ordinal == 0 && sequence->last->original_ordinal == 3);
        assert(sequence->first->sequence_ordinal == 0 && sequence->last->sequence_ordinal == 1);
        assert(name_is(sequence->last->name, "...implicit result"));
    }
    assert(reversed == binding_plan.last_interface && all->sequences[BINDING_STATIC_INPUT].first == original);
    assert(all->sequences[BINDING_STATIC_INPUT].count == 4 && original->next->original_ordinal == 1);
    assert(captures[0]->ordinal == 2 && captures[1]->ordinal == 1 && module->parameter_count == 4);
    destroy_binding();
}

/** A foreign same-spelled formal, result binder, duplicate capture and invalid
 * metadata array/surface never select a convenient fallback identity.
 */
static void it_rejects_invalid_capture_sets_before_allocating_or_changing_sources(void) {
    for (size_t partition = 0; partition < 6; partition++) {
        setup_binding(partition == 1 ? graph_source : interface_source, "@<\nargument\n...explicit type\n@!\n");
        const StaticModule *module = binding_module(project.entry);
        const StaticModule *other = binding_module(project_find(&project, ".other"));
        const StaticSymbol *captures[] = {symbol(module, "...explicit type"), symbol(module, "...explicit type")};
        size_t count = 1;
        BindingRuntimeSurface surface = BINDING_MODULE_SURFACE;
        if (partition == 0) { captures[0] = symbol(other, "...explicit type"); }
        if (partition == 1) { captures[0] = symbol(module, "...static one"); }
        if (partition == 2) { count = 2; }
        if (partition == 3) { captures[0] = NULL; }
        if (partition == 4) { surface = (BindingRuntimeSurface)-1; }
        size_t before = allocation_count;
        BindingInterface *output = NULL;
        assert(!binding_source_interface(&binding_plan, module, surface, partition == 5 ? NULL : captures, count, &output));
        assert(output == NULL && binding_plan.first_interface == NULL && binding_plan.status == BINDING_INVALID);
        assert(diagnostic_valid(&binding_plan.error) && allocation_count == before);
        assert(binding_plan.error.primary.span.source == project.entry->source->text);
        destroy_binding();
    }
}

/** A constant read exposes only its actual shared fields as runtime outputs;
 * a completely empty module contributes no synthetic default interface fields.
 */
static void it_selects_shared_constant_surfaces_and_preserves_empty_sequences(void) {
    setup_binding(interface_source, STATIC_SCOPE_SOURCE_TEXT);
    BindingInterface *view = NULL;
    assert(binding_source_interface(&binding_plan, binding_module(project.entry), BINDING_CONSTANT_SURFACE, NULL, 0, &view));
    assert(view->sequences[BINDING_RUNTIME_INPUT].count == 0);
    assert(view->sequences[BINDING_RUNTIME_OUTPUT].count == 1);
    assert(name_is(view->sequences[BINDING_RUNTIME_OUTPUT].first->name, "shared"));
    assert(view->sequences[BINDING_STATIC_INPUT].count == 4 && view->sequences[BINDING_STATIC_OUTPUT].count == 0);
    destroy_binding();
    setup_binding("@!\n", STATIC_SCOPE_SOURCE_TEXT);
    assert(binding_source_interface(&binding_plan, binding_module(project.entry), BINDING_MODULE_SURFACE, NULL, 0, &view));
    for (size_t direction = 0; direction < BINDING_SEQUENCE_COUNT; direction++) {
        assert(view->sequences[direction].count == 0 && view->sequences[direction].first == NULL && view->sequences[direction].last == NULL);
    }
    destroy_binding();
}

/** Fixing every original input produces an empty residual sequence while
 * preserving output/runtime positions and all original producer metadata.
 */
static void it_preserves_other_sequences_when_all_static_inputs_are_fixed(void) {
    setup_binding(interface_source, STATIC_SCOPE_SOURCE_TEXT);
    const StaticModule *module = binding_module(project.entry);
    const StaticSymbol *captures[] = {symbol(module, "...implicit result"), symbol(module, "...factory"),
        symbol(module, "...implicit input"), symbol(module, "...explicit type")};
    BindingInterface *view = NULL;
    assert(binding_source_interface(&binding_plan, module, BINDING_MODULE_SURFACE, captures, 4, &view));
    assert(view->sequences[BINDING_STATIC_INPUT].count == 0 && view->sequences[BINDING_STATIC_INPUT].first == NULL);
    assert(view->sequences[BINDING_STATIC_OUTPUT].count == 0 && view->sequences[BINDING_RUNTIME_INPUT].count == 3
        && view->sequences[BINDING_RUNTIME_OUTPUT].count == 2);
    assert(module->parameter_count == 4 && captures[0]->ordinal == 3 && captures[3]->ordinal == 0);
    destroy_binding();
}

/** Source-name ambiguity is refused during snapshot construction rather than
 * choosing the first matching ordinary field. T172 certifies static scope only.
 */
static void it_rejects_duplicate_actual_runtime_names_without_publishing_partial_views(void) {
    const char *cases[] = {
        "@<\nsame\n...type\nsame\n...type\n@!\n",
        "@>\nsame\n...type\nsame\n...type\n@!\n"
    };
    for (size_t index = 0; index < 2; index++) {
        setup_binding(cases[index], STATIC_SCOPE_SOURCE_TEXT);
        BindingInterface *view = NULL;
        assert(!binding_source_interface(&binding_plan, binding_module(project.entry), BINDING_MODULE_SURFACE, NULL, 0, &view));
        assert(view == NULL && binding_plan.first_interface == NULL && binding_plan.status == BINDING_INVALID);
        assert(diagnostic_valid(&binding_plan.error) && layout_text_is(binding_plan.error.primary.span, "same"));
        assert(!binding_memory_failure(&binding_plan) && binding_plan.status == BINDING_INVALID);
        destroy_binding();
    }
}
