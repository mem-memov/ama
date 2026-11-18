/** @file
 * @brief Verify qualified own constant resolution without synthetic syntax or creation edges.
 */
#include "../common.c"

/** Exactly the original catalog occurrence supplies the compact source. */
static const AstSource *source_at(size_t ordinal) {
    for (const StaticConstantOccurrence *site = caller_module->first_constant_source; site; site = site->next) {
        if (ordinal-- == 0) { return site->source; }
    }
    assert(false); return NULL;
}

/** Both spellings select the same declaration/Type in the explicitly required
 * ordinary target; neither introduces a caller-to-namespace creation edge.
 */
static void it_agrees_between_compact_and_explicit_fixed_namespace_reads(void) {
    setup_constants("@!\nanchor&\n..integer\nx\n..integer\n .other.value\n.other\n value >> y\n",
        "@=\nvalue\n..integer\n |1\n@!\n");
    const StaticResultContext *caller = publication_context(caller_environment);
    const StaticResultContext *actual = constant_fixed_target_context();
    const StaticConstantSource *source, *again;
    assert(static_constant_source(&constants, caller, source_at(0), &source));
    assert(static_constant_source(&constants, caller, source_at(0), &again) && source == again);
    assert(source->namespace_type == target_constructor && source->occurrence->origin == target_module->definition);
    const StaticConstantValue *value, *repeated;
    assert(static_constant_value(&constants, source, actual, &value));
    assert(static_constant_value(&constants, source, actual, &repeated) && value == repeated);
    const StaticConstantRead *read;
    const StaticConstantSelection *selection;
    assert(static_constant_read(&constants, caller, constant_namespace_read(0), &read));
    assert(static_constant_select(&constants, read, actual, &selection));
    assert(read->source->producer == NULL && value->declaration == selection->first->declaration);
    assert(value->declaration->source->source->head == integer_type && guards.edge_count == 0);
    assert(constants.source_count == 1 && constants.value_count == 1 && constants.declaration_count == 1);
    destroy_constants();
}

/** Qualified collection suffixes and own membership are mandatory; returned
 * fields, nested names and expected receiver Types cannot repair a bad source.
 */
static void it_checks_qualified_members_storage_and_open_namespaces(void) {
    const char *members[] = {"items[]", "items", "missing", "result"};
    for (size_t mode = 0; mode < 4; mode++) {
        char caller[256];
        int length = snprintf(caller, sizeof(caller), "@!\nanchor&\n..integer\nx[]\n..integer\n .other.%s\n", members[mode]);
        assert(length > 0 && (size_t)length < sizeof(caller));
        setup_constants(caller, "@=\nitems[]\n..integer\n@>\nresult\n..integer\n@!\n");
        const StaticResultContext *context = publication_context(caller_environment), *actual = constant_fixed_target_context();
        const StaticConstantSource *source;
        const StaticConstantValue *value = NULL;
        assert(static_constant_source(&constants, context, source_at(0), &source));
        bool success = static_constant_value(&constants, source, actual, &value);
        assert(success == (mode == 0));
        if (success) { assert(value->declaration->list && value->declaration->syntax == constant_named(target_module, "items")); }
        else { assert(value == NULL && constants.status == STATIC_CONSTANT_CONFLICT); }
        destroy_constants();
    }
    setup_constants("@!\nanchor&\n..integer\nx\n..integer\n .other.value\n",
        "@=\nvalue\n...slot\n@<\n...slot\n@!\n");
    const StaticResultContext *context = publication_context(caller_environment);
    const StaticConstantSource *source;
    const StaticConstantValue *value = NULL;
    assert(static_constant_source(&constants, context, source_at(0), &source));
    assert(!static_constant_value(&constants, source, context, &value));
    assert(value == NULL && constants.status == STATIC_CONSTANT_UNRESOLVED && constants.declaration_count == 0);
    destroy_constants();
}

/** Foreign/copied syntax, context and registration handles cannot substitute
 * for original ownership; missing or wrong actual namespace scopes fail exactly.
 */
static void it_refuses_foreign_qualified_handles_scopes_and_count_wrap(void) {
    for (size_t mode = 0; mode < 8; mode++) {
        setup_constants("@!\nanchor&\n..integer\nx\n..integer\n .other.value\n", "@=\nvalue\n..integer\n@!\n");
        const StaticResultContext *context = publication_context(caller_environment), *actual = constant_fixed_target_context();
        AstSource copied = *source_at(0);
        const StaticConstantSource *source = NULL;
        if (mode < 3) {
            if (mode == 2) { constants.source_count = SIZE_MAX; }
            assert(!static_constant_source(&constants, mode == 1 ? NULL : context,
                mode == 0 ? &copied : source_at(0), &source));
            assert(source == NULL && constants.status == (mode == 2 ? STATIC_CONSTANT_OUT_OF_MEMORY : STATIC_CONSTANT_INVALID));
        } else {
            assert(static_constant_source(&constants, context, source_at(0), &source));
            StaticConstantSource foreign = *source;
            const StaticConstantValue *value = NULL;
            if (mode == 7) { constants.value_count = SIZE_MAX; }
            assert(!static_constant_value(&constants, mode == 3 ? &foreign : source,
                mode == 4 ? NULL : mode == 5 ? context : actual, mode == 6 ? NULL : &value));
            assert(value == NULL && constants.status == (mode == 7 ? STATIC_CONSTANT_OUT_OF_MEMORY : STATIC_CONSTANT_INVALID));
        }
        assert(diagnostic_valid(&constants.error));
        destroy_constants();
    }
}

/** Distinct caller specializations reuse one fixed namespace without a false
 * creation path between their keys or duplicate shared declaration identity.
 */
static void it_reuses_one_fixed_namespace_from_distinct_template_specializations(void) {
    setup_constants("@<\n...slot\n@!\nanchor&\n..integer\nx\n..integer\n .other.value\n.other\n value >> y\n",
        "@=\nvalue\n..integer\n@!\n");
    const StaticResultContext *actual = constant_fixed_target_context();
    const StaticConstantDeclaration *shared = NULL;
    const StaticTerm *keys[2];
    for (size_t index = 0; index < 2; index++) {
        StaticTermCapture capture = {.formal = application_formal(caller_module, 0), .value = index ? target_constructor : integer_type};
        const StaticTermEnvironment *environment;
        assert(static_term_nominal(&terms, caller_module, &capture, 1, (SourceSpan){0}, &keys[index]));
        assert(static_term_environment(&terms, caller_module, keys[index], &environment));
        const StaticResultContext *context = publication_context(environment);
        const StaticConstantSource *source;
        const StaticConstantValue *value;
        const StaticConstantRead *read;
        const StaticConstantSelection *selection;
        assert(static_constant_source(&constants, context, source_at(0), &source));
        assert(static_constant_value(&constants, source, actual, &value));
        assert(static_constant_read(&constants, context, constant_namespace_read(0), &read));
        assert(static_constant_select(&constants, read, actual, &selection));
        if (index == 0) { shared = value->declaration; }
        assert(value->declaration == shared && selection->first->declaration == shared && read->source->producer == NULL);
    }
    assert(keys[0] != keys[1] && guards.edge_count == 0 && guards.status == STATIC_GUARD_OK);
    assert(constants.source_count == 2 && constants.value_count == 2 && constants.declaration_count == 1);
    destroy_constants();
}

/** Root markers and the final member's complete name retain original spans. */
static void it_preserves_standard_root_identity_and_original_member_spans(void) {
    setup_constants("@!\nanchor&\n..integer\nx\n..integer\n ..integer.example member\n", "@!\n");
    const StaticResultContext *context = publication_context(caller_environment);
    const StaticConstantSource *source;
    assert(static_constant_source(&constants, context, source_at(0), &source));
    assert(source->namespace_type == integer_type);
    assert(layout_text_is(source->occurrence->source->module_path, "..integer"));
    assert(layout_text_is(source->occurrence->source->member, "example member"));
    assert(source->occurrence->origin == integer_type->as.nominal.origin->definition && guards.edge_count == 0);
    destroy_constants();
}

/** Sweep all allocations in source registration and final member selection.
 * Every refused operation leaves public outputs untouched and counted owners empty.
 */
static void it_unwinds_every_qualified_registration_and_selection_allocation_refusal(void) {
    for (size_t phase = 0; phase < 2; phase++) {
        size_t total = 0;
        for (size_t cut = 0; cut <= total; cut++) {
            setup_constants("@!\nanchor&\n..integer\nx\n..integer\n .other.value\n", "@=\nvalue\n..integer\n@!\n");
            const StaticResultContext *context = publication_context(caller_environment), *actual = constant_fixed_target_context();
            const StaticConstantSource *source = NULL;
            const StaticConstantValue *value = NULL;
            if (phase) { assert(static_constant_source(&constants, context, source_at(0), &source)); }
            size_t before = allocation_count;
            if (cut) { failure_at = before + cut; }
            bool success = phase ? static_constant_value(&constants, source, actual, &value) :
                static_constant_source(&constants, context, source_at(0), &source);
            if (!cut) { assert(success); total = allocation_count - before; assert(total > 0); }
            else {
                assert(!success && constants.status == STATIC_CONSTANT_OUT_OF_MEMORY);
                assert(phase ? value == NULL : source == NULL);
            }
            failure_at = 0;
            destroy_constants();
        }
    }
}

/** Execute compact/explicit agreement, strict selection and ownership partitions. */
static void run(void) {
    it_agrees_between_compact_and_explicit_fixed_namespace_reads();
    it_checks_qualified_members_storage_and_open_namespaces();
    it_refuses_foreign_qualified_handles_scopes_and_count_wrap();
    it_reuses_one_fixed_namespace_from_distinct_template_specializations();
    it_preserves_standard_root_identity_and_original_member_spans();
    it_unwinds_every_qualified_registration_and_selection_allocation_refusal();
}
/** Return success only after every assertion and counted teardown. */
int main(void) { run(); return 0; }
