/** @file
 * @brief Require responsible original producer definitions and their ordinary selection sites in actual frontend diagnostics.
 * @see [T189 contract](../../../../docs/implementation/specialization-diagnostics.md)
 */
#include <stdint.h>
#include "../common.c"
#include "diagnostics/render/render.h"

/** Match original byte-based positions, never explanatory generated display names. */
static bool span_in_lines(SourceSpan span, const SourceText *source, size_t first, size_t last) {
    SourcePosition position;
    return span.source == source && source_position(source, span.start, &position)
        && position.line >= first && position.line <= last;
}

/** Inspect every explicit location without dropping original notes when attribution is attached. */
static bool references_site(const Diagnostic *error, const SourceText *source, size_t first, size_t last) {
    if (span_in_lines(error->primary.span, source, first, last)) { return true; }
    for (size_t index = 0; index < error->related_count; index++) {
        if (span_in_lines(error->related[index].location.span, source, first, last)) { return true; }
    }
    for (const DiagnosticContext *node = error->contexts; node; node = node->next) {
        if (span_in_lines(node->related.location.span, source, first, last)) { return true; }
    }
    return false;
}

/** An unresolved producer must identify both its source definition and the real ordinary selection that requires it. */
static void it_links_incomplete_static_producer_definitions_to_their_original_required_selection(void) {
    directory("generated/project");
    file("generated/project/main", "@!\nvalue\n.factory\n ...out\n");
    file("generated/project/factory", "@...\n...out\n...missing\n");
    setup_frontend();
    ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
        {TEST_STANDARD_ROOT, NULL, 0, MODULE_ROOT_STANDARD}};
    assert(!compiler_frontend_check(&frontend, roots, 2, ".main"));
    const Diagnostic *error = &frontend.error;
    assert(diagnostic_valid(error) && !frontend.valid && !frontend.plan_initialized);
    const ProjectModule *factory = project_find(&frontend.project, ".factory");
    assert(factory && references_site(error, factory->source->text, 1, SIZE_MAX));
    size_t before = allocation_count;
    assert(diagnostic_render(stderr, error) == DIAGNOSTIC_RENDER_OK && allocation_count == before);
    assert(references_site(error, frontend.project.entry->source->text, 4, 4));
    destroy_frontend();
}

/** Unknown ordinary input types must identify their actual declaration or annotation, rather than only the module marker. */
static void it_identifies_the_original_input_declaration_for_unanchored_required_types(void) {
    directory("generated/project");
    file("generated/project/main", "@<\ninput\n...slot\n@!\n");
    setup_frontend();
    ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
        {TEST_STANDARD_ROOT, NULL, 0, MODULE_ROOT_STANDARD}};
    assert(!compiler_frontend_check(&frontend, roots, 2, ".main"));
    assert(diagnostic_valid(&frontend.error) && !frontend.valid && !frontend.plan_initialized);
    assert(diagnostic_render(stderr, &frontend.error) == DIAGNOSTIC_RENDER_OK);
    assert(references_site(&frontend.error, frontend.project.entry->source->text, 2, 3));
    destroy_frontend();
}

/** Every failed frontend allocation keeps an owned, renderable error; context attachment never needs another allocation after the cause. */
static void it_preserves_renderable_producer_failures_under_every_frontend_allocation_refusal(void) {
    directory("generated/project");
    file("generated/project/main", "@!\nvalue\n.factory\n ...out\n");
    file("generated/project/factory", "@...\n...out\n...missing\n");
    ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
        {TEST_STANDARD_ROOT, NULL, 0, MODULE_ROOT_STANDARD}};
    size_t total = 0;
    for (size_t cut = 0; cut <= total; cut++) {
        setup_frontend();
        size_t before = allocation_count;
        if (cut) { failure_at = before + cut; }
        assert(!compiler_frontend_check(&frontend, roots, 2, ".main"));
        assert(diagnostic_valid(&frontend.error) && !frontend.valid);
        if (!cut) { total = allocation_count - before; assert(total); }
        else { assert(frontend.error.code == DIAGNOSTIC_OUT_OF_MEMORY); }
        FILE *stream = tmpfile();
        assert(stream);
        size_t failed_count = allocation_count;
        assert(diagnostic_render(stream, &frontend.error) == DIAGNOSTIC_RENDER_OK);
        assert(allocation_count == failed_count && fclose(stream) == 0);
        failure_at = 0;
        destroy_frontend();
    }
    printf("producer diagnostic frontend allocation refusal positions: %zu\n", total);
}

#include "support/families.c"
#include "support/graphs.c"

/** Explicitly run each original-provenance refusal partition. */
static void run(void) {
    it_preserves_independent_bridge_refusals_across_declaration_and_root_orders();
    it_rejects_malformed_static_names_at_the_original_source_line();
    it_links_incomplete_static_producer_definitions_to_their_original_required_selection();
    it_preserves_renderable_producer_failures_under_every_frontend_allocation_refusal();
    it_identifies_the_original_input_declaration_for_unanchored_required_types();
    it_reports_static_signature_and_kind_conflicts_with_original_constructor_context();
    it_distinguishes_original_eager_ownership_and_shared_value_cycle_locations();
    it_preserves_extra_template_residuals_and_reports_their_original_formals();
    it_reports_static_definition_cycles_without_fabricating_runtime_invocations();
    it_reports_occurs_refusal_with_original_variable_and_constructor_definitions();
    it_reports_changing_key_creation_at_its_actual_original_required_path();
    it_reports_pair_definition_and_conflicting_equality_receipt_as_distinct_original_sites();
}

/** Return success only after every borrowed diagnostic and counted teardown assertion. */
int main(void) { run(); return 0; }
