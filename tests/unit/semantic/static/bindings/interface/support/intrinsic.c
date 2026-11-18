/** @file
 * @brief Verify all canonical intrinsic arities and borrowed field identities without source spans.
 */

/** Independent arity expectations complement identity comparisons against the
 * canonical provider; the adapter must introduce no second field/name table.
 */
static void it_reuses_every_canonical_intrinsic_interface_without_source_fabrication(void) {
    setup_binding("@!\n", STATIC_SCOPE_SOURCE_TEXT);
    const size_t arities[][2] = {{1, 1}, {2, 1}, {1, 0}, {0, 0}, {2, 0}, {0, 1}};
    for (size_t method = 0; method < SEMANTIC_LIST_METHOD_COUNT; method++) {
        BindingInterface *view = NULL;
        assert(binding_list_interface(&binding_plan, (SemanticListMethod)method, &view));
        assert(view->origin == NULL && view->kind == BINDING_LIST_INTERFACE && (size_t)view->list_method == method);
        assert(view->sequences[BINDING_STATIC_INPUT].count == 0 && view->sequences[BINDING_STATIC_OUTPUT].count == 0);
        for (size_t result = 0; result < 2; result++) {
            const BindingSequence *sequence = &view->sequences[result ? BINDING_RUNTIME_OUTPUT : BINDING_RUNTIME_INPUT];
            assert(sequence->count == arities[method][result]);
            const BindingSlot *slot = sequence->first;
            for (size_t ordinal = 0; ordinal < sequence->count; ordinal++) {
                const SemanticListField *field = semantic_list_position((SemanticListMethod)method, result != 0, ordinal);
                assert(slot != NULL && slot->intrinsic_field == field && slot->runtime_declaration == NULL && slot->static_symbol == NULL);
                assert(slot->original_ordinal == ordinal && slot->sequence_ordinal == ordinal);
                assert(slot->reference == field->element_reference && !slot->list && name_is(slot->name, field->name));
                assert(slot->name.site.source == NULL && slot->name.bytes == (const unsigned char *)field->name);
                slot = slot->next;
            }
            assert(slot == NULL);
        }
    }
    destroy_binding();
    for (size_t invalid = 0; invalid < 2; invalid++) {
        setup_binding("@!\n", STATIC_SCOPE_SOURCE_TEXT);
        BindingInterface *view = NULL;
        size_t before = allocation_count;
        assert(!binding_list_interface(&binding_plan, invalid == 0 ? SEMANTIC_LIST_METHOD_COUNT : (SemanticListMethod)-1, &view));
        assert(view == NULL && binding_plan.status == BINDING_INVALID && allocation_count == before);
        assert(diagnostic_valid(&binding_plan.error));
        destroy_binding();
    }
}
