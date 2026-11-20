/** @file
 * @brief Check public graph pair provenance and exclusion of foreign labels and ordinary fields.
 */
#include "../common.c"
#include "semantic/static/bindings/owner.h"
#include "support/graph.c"
#include "support/permutations.c"

/** Graph pairs are public binders; nested labels belong to the callee interface.
 * Free-input classification remains separate from the public-binder pass.
 */
static const char *explicit_source = "@...\n.first type\n.type\n"
    ".second family\n.factory\n .foreign key < .type\n"
    " .nested family < .other factory\n  .foreign element < .type\n";

/** Named pair collection has no runtime fields, explicit formals or signatures. */
static void it_collects_graph_pairs_without_importing_free_inputs_or_foreign_labels(void) {
    setup_static(explicit_source, STATIC_SCOPE_SOURCE_TEXT);
    StaticModule *module = static_module_add(&inventory, project.entry);
    assert(module != NULL && static_collect_explicit(&inventory, module));
    assert(module->parameter_count == 0 && module->result_count == 2);
    const char *names[] = {".first type", ".second family"};
    StaticSymbol *symbol = module->first;
    const AstNode *node = project.entry->syntax.graph.first;
    for (size_t index = 0; index < 2; index++) {
        assert(symbol != NULL && layout_text_is(symbol->name, names[index]));
        assert(symbol->category == STATIC_CATEGORY_DEFERRED && symbol->declaration == node);
        assert(symbol->role == STATIC_RESULT && symbol->ordinal == index);
        assert(static_symbol_find(module, symbol->name) == symbol);
        assert(symbol->site.source == project.entry->source->text && symbol->site.start == symbol->name.start);
        symbol = symbol->next;
        node = node->next;
    }
    assert(symbol == NULL && !inventory.valid);
    assert(static_symbol_find(module, static_name_site(project.entry, ".foreign key")) == NULL);
    assert(static_symbol_find(module, static_name_site(project.entry, ".foreign element")) == NULL);
    assert(static_symbol_find(module, static_name_site(project.entry, ".type")) == NULL);
    destroy_static();
}

/** Repeated complete public names fail at the second header with the original
 * header retained, independently of either RHS's spelling or category.
 */
static void it_rejects_duplicate_public_pairs_with_original_headers(void) {
    setup_static("@...\n.same\n.value\n.same\n.other\n", STATIC_SCOPE_SOURCE_TEXT);
    StaticModule *module = static_module_add(&inventory, project.entry);
    assert(module != NULL && !static_collect_explicit(&inventory, module));
    assert(inventory.status == STATIC_INVALID && diagnostic_valid(&inventory.error));
    assert(inventory.error.related_count == 1 && module->first == module->last);
    assert(layout_text_is(inventory.error.primary.span, ".same"));
    assert(layout_text_is(inventory.error.related[0].location.span, ".same"));
    assert(inventory.error.primary.span.start > inventory.error.related[0].location.span.start);
    destroy_static();
}

/** Ordinary and native Type lines introduce no explicit static graph results;
 * their actual free Type parameters are classified by the interface phase.
 */
static void it_excludes_ordinary_and_native_type_lines_from_public_graph_collection(void) {
    const char *cases[] = {"@<\nvalue\n.type\n@!\n", "@<\nvalue\n.type\n@C\n"};
    for (size_t index = 0; index < 2; index++) {
        setup_static(cases[index], STATIC_SCOPE_SOURCE_TEXT);
        StaticModule *module = static_module_add(&inventory, project.entry);
        assert(module != NULL && static_collect_explicit(&inventory, module));
        assert(module->parameter_count == 0 && module->result_count == 0);
        assert(module->first == NULL && module->last == NULL && !inventory.valid);
        destroy_static();
    }
}

/** Each measured pair-collection refusal leaves owned partial records invalid. */
static void it_unwinds_every_public_pair_collection_allocation_failure(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        setup_static(explicit_source, STATIC_SCOPE_SOURCE_TEXT);
        StaticModule *module = static_module_add(&inventory, project.entry);
        assert(module != NULL);
        size_t before = allocation_count;
        if (failure != 0) { failure_at = before + failure; }
        bool collected = static_collect_explicit(&inventory, module);
        if (failure == 0) { assert(collected); total = allocation_count - before; assert(total != 0); }
        else {
            assert(!collected && inventory.status == STATIC_OUT_OF_MEMORY && !inventory.valid);
            assert(diagnostic_valid(&inventory.error));
        }
        destroy_static();
    }
}

/** Run pair collection independently of free-input and alias classification. */
void run(void) {
    it_keeps_pair_roles_for_every_three_statement_permutation();
    it_uses_the_first_lexical_receipt_as_the_alias_definition();
    it_reports_pair_duplicates_against_pair_headers_not_earlier_receipts();
    it_collects_pairs_before_receipts_and_retains_first_alias_definition();
    it_exposes_every_public_alias_in_the_actual_source_interface();
    it_binds_forward_alias_uses_without_creating_external_inputs();
    it_unwinds_every_public_receipt_collection_allocation_failure();
    it_collects_graph_pairs_without_importing_free_inputs_or_foreign_labels();
    it_rejects_duplicate_public_pairs_with_original_headers();
    it_excludes_ordinary_and_native_type_lines_from_public_graph_collection();
    it_unwinds_every_public_pair_collection_allocation_failure();
}
/** Return success only after source-backed assertions and complete owner teardown. */
int main(void) { run(); return EXIT_SUCCESS; }
