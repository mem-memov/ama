/** @file
 * @brief Verify compiler-owned static-stage handoffs, definition-time checks and complete failure-prefix teardown.
 */
#include "../../semantic/static/common.c"
#include "compiler/specializations/analysis.h"
#include "semantic/static/native/admission.h"

/** Owned analysis dies before borrowed source inventory and original project. */
static CompilerSpecialization analysis;

/** Compose the actual accepted source/native-policy boundary before initialization. */
static void setup_analysis(const char *caller, const char *target) {
    ModuleRoot roots[] = {{STATIC_SCOPE_ROOT_DIRECTORY, NULL, 0, MODULE_ROOT_PROJECT},
        {TEST_STANDARD_ROOT, NULL, 0, MODULE_ROOT_STANDARD}};
    setup_static_with_roots(caller, target, roots, 2);
    assert(static_inventory_collect(&inventory) == STATIC_OK);
    StaticNativeAdmission admission = {0};
    assert(static_native_admit(&inventory, &admission));
    size_t before = allocation_count;
    compiler_specialization_init(&analysis, &inventory, sources.allocator);
    assert(allocation_count == before && !analysis.attempted && !analysis.valid);
}

/** Preserve borrowed inventory after child destruction, then prove counted balance. */
static void destroy_analysis(void) {
    compiler_specialization_destroy(&analysis);
    assert(!analysis.inventory && !analysis.attempted && inventory.valid);
    destroy_static();
}

/** Compiler-owned children perform real generic-call inference without native execution. */
static void it_closes_selected_root_types_through_owned_semantic_stages(void) {
    setup_analysis("@!\nknown\n..integer\nworker\n.other\nworker !\n < known\n > fresh\n",
        "@<\ninput\n...slot\n@>\noutput\n...slot\n@!\noutput\n input\n");
    const ProjectModule *roots[] = {project.entry};
    assert(compiler_specialization_check(&analysis, roots, 1) && analysis.valid && analysis.closure.closed);
    const StaticClosureEntry *entry = analysis.closure.first;
    while (entry && entry->scope->environment->definition->definition != project.entry) { entry = entry->next; }
    assert(entry);
    SourceSpan name = static_name_site(project.entry, "fresh");
    const StaticTypedField *field = entry->scope->first_field;
    while (field && (field->definition->name.source != name.source || field->definition->name.start != name.start)) { field = field->next; }
    assert(field && field->definition->name.end == name.end);
    const StaticTerm *value;
    StaticConstraintOutcome outcome;
    assert(static_constraint_value(&analysis.constraints, field->type, &value, &outcome));
    assert(outcome == STATIC_CONSTRAINT_COMPLETE && value->as.nominal.origin->definition == project_find(&project, "..integer"));
    for (const StaticRegistryCandidate *candidate = analysis.registry.first_candidate; candidate; candidate = candidate->next) {
        assert(!candidate->local_started && !candidate->local_complete);
    }
    destroy_analysis();
}

/** Loading an open definition checks names but never silently instantiates it. */
static void it_checks_definitions_without_inventing_roots_or_type_defaults(void) {
    setup_analysis("@!\nanchor&\n..integer\n", "@<\nvalue\n...slot\n@!\n");
    assert(compiler_specialization_check(&analysis, NULL, 0) && analysis.valid);
    assert(analysis.connections.runtime_count > 0 && analysis.results.context_count == 0);
    assert(analysis.registry.candidate_count == 0 && analysis.constraints.requirement_count == 0);
    destroy_analysis();
}

/** Invalid runtime names in an unused open template fail at definition time. */
static void it_rejects_bad_lexical_bindings_in_uninstantiated_definitions(void) {
    setup_analysis("@!\nanchor&\n..integer\n", "@<\nvalue\n...slot\n@!\nmissing !\n");
    const ProjectModule *roots[] = {project.entry};
    assert(!compiler_specialization_check(&analysis, roots, 1) && !analysis.valid);
    assert(diagnostic_valid(&analysis.error));
    assert(analysis.error.primary.span.source == project_find(&project, ".other")->source->text);
    assert(analysis.results.context_count == 0);
    destroy_analysis();
}

/** Distinct explicit roots close in either order without source-name or layout merging. */
static void it_preserves_explicit_multiple_roots_in_either_order(void) {
    for (size_t reverse = 0; reverse < 2; reverse++) {
        setup_analysis("@!\nanchor&\n..integer\n", "@!\nanchor&\n..integer\n");
        const ProjectModule *other = project_find(&project, ".other");
        const ProjectModule *roots[] = {reverse ? other : project.entry, reverse ? project.entry : other};
        assert(compiler_specialization_check(&analysis, roots, 2) && analysis.valid);
        bool seen_main = false, seen_other = false;
        for (const StaticClosureEntry *entry = analysis.closure.first; entry; entry = entry->next) {
            if (entry->scope->environment->definition->definition == project.entry) { seen_main = true; }
            if (entry->scope->environment->definition->definition == other) { seen_other = true; }
        }
        assert(seen_main && seen_other);
        destroy_analysis();
    }
}

/** Copied modules, missing root arrays and static producers cannot become ordinary roots. */
static void it_refuses_invalid_or_static_root_selections(void) {
    for (size_t kind = 0; kind < 3; kind++) {
        setup_analysis("@!\nanchor&\n..integer\n", "@...\n...out\n..integer\n");
        ProjectModule copied = *project.entry;
        const ProjectModule *roots[] = {kind == 0 ? &copied : project_find(&project, ".other")};
        assert(!compiler_specialization_check(&analysis, kind == 1 ? NULL : roots, 1) && !analysis.valid);
        assert(diagnostic_valid(&analysis.error) && analysis.results.context_count == 0);
        if (kind == 2) {
            SourceSpan marker = project_find(&project, ".other")->syntax.sections.graph.marker->code;
            assert(analysis.error.primary.span.source == marker.source && analysis.error.primary.span.start == marker.start &&
                analysis.error.primary.span.end == marker.end);
        }
        destroy_analysis();
    }
}

/** Every owned-stage allocation can refuse without publishing successful closure
 * or losing a borrowed diagnostic before reverse destruction. */
static void it_unwinds_every_owned_compiler_analysis_allocation_refusal(void) {
    size_t total = 0;
    for (size_t cut = 0; cut <= total; cut++) {
        setup_analysis("@!\nreceived\n .other.value\n", "@=\nvalue\n..integer\n |7\n@!\n");
        const ProjectModule *roots[] = {project.entry};
        size_t before = allocation_count;
        if (cut) { failure_at = before + cut; }
        bool success = compiler_specialization_check(&analysis, roots, 1);
        if (!cut) { assert(success && analysis.valid); total = allocation_count - before; assert(total > 0); }
        else { assert(!success && !analysis.valid && diagnostic_valid(&analysis.error) && analysis.error.code == DIAGNOSTIC_OUT_OF_MEMORY); }
        failure_at = 0;
        destroy_analysis();
    }
    printf("owned compiler specialization allocation refusal positions: %zu\n", total);
}

/** Explicit dispatch covers real handoffs, source timing, selected roots and failures. */
static void run(void) {
    it_closes_selected_root_types_through_owned_semantic_stages();
    it_checks_definitions_without_inventing_roots_or_type_defaults();
    it_rejects_bad_lexical_bindings_in_uninstantiated_definitions();
    it_preserves_explicit_multiple_roots_in_either_order();
    it_refuses_invalid_or_static_root_selections();
    it_unwinds_every_owned_compiler_analysis_allocation_refusal();
}
/** Borrowed inventory remains valid until complete counted teardown. */
int main(void) { run(); return 0; }
