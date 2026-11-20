/** @file
 * @brief Verify immutable aliases, multiple public outputs and residual constructor values.
 */
#include "../common.c"

/** Equal receiver names share metadata while actual source uses remain distinct. */
static void it_reuses_equal_aliases_without_merging_producer_occurrences(void) {
    setup_publications("@...\n/test/fixture/v1.0.0/src/other\n .slot < //integer\n .out > .alias\n/test/fixture/v1.0.0/src/other\n .slot < //integer\n .out > .alias\n", result_target_source);
    const StaticResultContext *context = publication_context(caller_environment);
    const StaticPublication *first, *second;
    assert(static_publication_resolve(&publications, context, result_projection(0), &first));
    assert(static_publication_resolve(&publications, context, result_projection(1), &second));
    assert(first != second && first->value == NULL && second->value == NULL);
    assert(publications.count == 2 && publications.alias_count == 1);
    assert(publications.first_alias->symbol == result_named(caller_module, ".alias"));
    assert(publications.first_alias->value == integer_type && first->first_output->value == integer_type);
    destroy_publications();
}

/** Returning an open ordinary constructor does not leave producer inputs open. */
static void it_preserves_returned_constructor_residuals_without_producer_types(void) {
    fixture_package_prepare(STATIC_SCOPE_ROOT_DIRECTORY, "fixture");
    file("generated/static scope/packages/test/fixture/v1.0.0/src/family", "@<\nvalue\n.slot\n@!\n");
    setup_publications("@...\n/test/fixture/v1.0.0/src/other\n .family > .returned\n",
        "@...\n.family\n/test/fixture/v1.0.0/src/family\n.anchor\n//integer\n");
    const StaticResultContext *context = publication_context(caller_environment);
    const StaticPublication *publication;
    assert(static_publication_resolve(&publications, context, result_projection(0), &publication));
    assert(publication->value == NULL && publication->output_count == 2);
    const StaticTerm *family = publication->first_output->value;
    assert(family->as.nominal.residual_count == 1 && family->as.nominal.capture_count == 0);
    assert(family->as.nominal.origin->definition == project_find(&project, "/test/fixture/v1.0.0/src/family"));
    assert(publications.first_alias->value == family);
    assert(publication->context->registry_use->candidate->key->definition->parameter_count == 0);
    destroy_publications();
}

/** Only selection is sparse; complete producer publication includes unselected names. */
static void it_publishes_every_declared_output_even_when_only_one_is_selected(void) {
    setup_publications("@...\n/test/fixture/v1.0.0/src/other\n .second > .selected\n",
        "@...\n.first\n//integer\n.second\n//integer\n");
    const StaticResultContext *context = publication_context(caller_environment);
    const StaticPublication *publication;
    assert(static_publication_resolve(&publications, context, result_projection(0), &publication));
    assert(publication->output_count == 2 && publication->first_output->next);
    assert(publication->first_output->symbol == result_named(target_module, ".first"));
    assert(publication->first_output->next->symbol == result_named(target_module, ".second"));
    assert(publications.alias_count == 1);
    destroy_publications();
}

/** Existing receiver equations are equality obligations, never mutable assignments. */
static void it_rejects_conflicting_existing_receivers_without_rebinding(void) {
    fixture_package_prepare(STATIC_SCOPE_ROOT_DIRECTORY, "fixture");
    file("generated/static scope/packages/test/fixture/v1.0.0/src/other type", "@!\n");
    setup_publications("@...\n.expected\n//integer\n/test/fixture/v1.0.0/src/other\n .out > .expected\n",
        "@...\n.out\n/test/fixture/v1.0.0/src/other type\n");
    const StaticResultContext *context = publication_context(caller_environment);
    const StaticPublication *publication = NULL;
    assert(!static_publication_resolve(&publications, context, result_projection(0), &publication));
    assert(publication == NULL && publications.status == STATIC_PUBLICATION_CONFLICT);
    assert(publications.count == 0 && publications.alias_count == 0);
    destroy_publications();
}

/** Execute immutable identity, residual and multiple-output boundaries. */
static void run(void) {
    it_reuses_equal_aliases_without_merging_producer_occurrences();
    it_preserves_returned_constructor_residuals_without_producer_types();
    it_publishes_every_declared_output_even_when_only_one_is_selected();
    it_rejects_conflicting_existing_receivers_without_rebinding();
}
/** Counted teardown precedes successful exit. */
int main(void) { run(); return 0; }
