/** @file
 * @brief Verify real frontend specialization ownership, ordinary admission, exact root policies and planning.
 */
#include <stdint.h>
#include "../common.c"
#include "compiler/internal.h"

/** Native companions deliberately fail if any check-only stage compiles C. */
static void prepare_sources(const char *main_text) {
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/main", main_text);
    file("generated/project/packages/test/fixture/v1.0.0/src/extra", "@!\nanchor&\n/test/fixture/v1.0.0/src/value\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/value", "@C\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/value.h", "#error Check-only analysis must not compile native C\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/value.c", "#error Check-only analysis must not execute native code\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/value.abi", "+ initialize\n- destroy\n| literal\n= clone\n");
}

/** Explicit selected roots retain the loaded frontend AST/source identities. */
static void it_exposes_closed_generic_type_candidates_before_ordinary_admission(void) {
    prepare_sources("@<\ninput\n.slot\n@>\noutput\n.slot\n@!\nknown\n/test/fixture/v1.0.0/src/value\ninput\n known\noutput\n input\n");
    setup_frontend();
    ModuleRoot root = {"generated/project", NULL, 0, MODULE_ROOT_PROJECT};
    assert(compiler_frontend_load(&frontend, &root, 1, "/test/fixture/v1.0.0/src/main"));
    const ProjectModule *selected[] = {frontend.project.entry};
    assert(compiler_frontend_specialize(&frontend, selected, 1));
    assert(frontend.specialization_initialized && frontend.specializations.valid && frontend.specializations.closure.closed);
    assert(!frontend.semantic_initialized && !frontend.plan_initialized && !frontend.valid);
    const StaticModule *origin = static_module_find(&frontend.static_inventory, frontend.project.entry);
    bool found = false;
    for (const StaticRegistryCandidate *candidate = frontend.specializations.registry.first_candidate; candidate; candidate = candidate->next) {
        if (candidate->key->definition == origin) {
            found = true;
            assert(candidate->key->owner_type->as.nominal.capture_count == 1);
            assert(candidate->key->owner_type->as.nominal.first_capture->value->as.nominal.origin->definition ==
                project_find(&frontend.project, "/test/fixture/v1.0.0/src/value"));
        }
        assert(!candidate->local_complete);
    }
    assert(found);
    destroy_frontend();
}

/** Reversed explicit roots preserve entry identity and full concrete planning. */
static void it_composes_explicit_multi_root_closure_with_the_real_frontend(void) {
    for (size_t reverse = 0; reverse < 2; reverse++) {
        prepare_sources("@!\nvalue\n/test/fixture/v1.0.0/src/value\n |literal\n");
        setup_frontend();
        ModuleRoot root = {"generated/project", NULL, 0, MODULE_ROOT_PROJECT};
        const char *required[] = {"/test/fixture/v1.0.0/src/extra"};
        assert(compiler_frontend_load_required(&frontend, &root, 1, "/test/fixture/v1.0.0/src/main", required, 1));
        const ProjectModule *extra = project_find(&frontend.project, "/test/fixture/v1.0.0/src/extra");
        const ProjectModule *selected[] = {reverse ? extra : frontend.project.entry,
            reverse ? frontend.project.entry : extra};
        assert(compiler_frontend_specialize(&frontend, selected, 2));
        assert(compiler_frontend_finish(&frontend, NULL));
        assert(frontend.valid && frontend.plan.valid && frontend.specializations.valid);
        assert(frontend.plan.entry->type->module == frontend.project.entry);
        bool seen_main = false, seen_extra = false;
        for (const StaticClosureEntry *entry = frontend.specializations.closure.first; entry; entry = entry->next) {
            if (entry->scope->environment->definition->definition == frontend.project.entry) { seen_main = true; }
            if (entry->scope->environment->definition->definition == extra) { seen_extra = true; }
        }
        assert(seen_main && seen_extra);
        destroy_frontend();
    }
}

/** The ordinary public check entry performs the same Type stage before planning. */
static void it_checks_concrete_projects_through_the_public_frontend_entry(void) {
    prepare_sources("@!\nvalue\n/test/fixture/v1.0.0/src/value\n |literal\n");
    setup_frontend();
    assert(check_project());
    assert(frontend.specialization_initialized && frontend.specializations.valid && frontend.specializations.closure.closed);
    assert(frontend.semantic_initialized && frontend.plan_initialized && frontend.valid);
    destroy_frontend();
}

/** Actual frontend completion consumes a closed generic body and its inferred runtime result. */
static void it_checks_generic_operations_through_the_actual_frontend_adapter(void) {
    prepare_sources("@!\nknown\n/test/fixture/v1.0.0/src/value\nworker\n/test/fixture/v1.0.0/src/worker\nworker !\n < known\n > fresh\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/worker", "@<\ninput\n.slot\n@>\noutput\n.slot\n@!\noutput\n input\n");
    setup_frontend();
    assert(check_project() && frontend.valid && frontend.plan.valid);
    assert(frontend.semantic.type_closure == &frontend.specializations.closure);
    const SemanticType *entry = frontend.plan.entry->type;
    assert(entry->symbol_count == 3);
    SemanticSymbol *known = entry->first, *worker = known->next, *fresh = worker->next;
    assert(fresh && !fresh->next && fresh->inferred && fresh->type == known->type);
    assert(worker->type->specialization->context->registry_use->candidate->local_complete);
    assert(worker->type->statements);
    destroy_frontend();
}

/** Static producer output is compile-time metadata; its selected Type reaches ordinary planning. */
static void it_plans_closed_static_exports_as_compile_time_metadata(void) {
    prepare_sources("@!\nvalue\n/test/fixture/v1.0.0/src/factory\n .result\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/factory", "@...\n.result\n/test/fixture/v1.0.0/src/value\n");
    setup_frontend();
    assert(check_project() && frontend.valid && frontend.plan.valid);
    for (const SemanticType *type = frontend.semantic.first; type; type = type->next) {
        assert(type->module->syntax.sections.kind != MODULE_STATIC);
    }
    assert(frontend.plan.entry->type->first->type->module == project_find(&frontend.project, "/test/fixture/v1.0.0/src/value"));
    destroy_frontend();
}

/** Arithmetic refusal cannot touch names, allocate a partial root array or start Type analysis. */
static void it_refuses_type_root_array_overflow_before_allocation(void) {
    prepare_sources("@!\nvalue\n/test/fixture/v1.0.0/src/value\n");
    setup_frontend();
    ModuleRoot root = {"generated/project", NULL, 0, MODULE_ROOT_PROJECT};
    assert(compiler_frontend_load(&frontend, &root, 1, "/test/fixture/v1.0.0/src/main"));
    size_t before = allocation_count;
    assert(!compiler_frontend_select_roots(&frontend, NULL, SIZE_MAX, COMPILER_ROOTS_EXPLICIT));
    assert(allocation_count == before && !frontend.specialization_initialized);
    assert(frontend.error.code == DIAGNOSTIC_OUT_OF_MEMORY && !frontend.semantic_initialized);
    destroy_frontend();
}

/** Catalog definitions remain open; explicitly requested executable Types must close. */
static void it_distinguishes_catalog_definition_and_explicit_type_root_requests(void) {
    for (size_t explicit = 0; explicit < 2; explicit++) {
        prepare_sources("@!\nvalue\n/test/fixture/v1.0.0/src/value\n");
        file("generated/project/packages/test/fixture/v1.0.0/src/extra", "@<\ninput\n.slot\n@!\n");
        setup_frontend();
        ModuleRoot root = {"generated/project", NULL, 0, MODULE_ROOT_PROJECT};
        const char *required[] = {"/test/fixture/v1.0.0/src/extra"};
        assert(compiler_frontend_load_required(&frontend, &root, 1, "/test/fixture/v1.0.0/src/main", required, 1));
        bool valid = compiler_frontend_select_roots(&frontend, required, 1,
            explicit ? COMPILER_ROOTS_EXPLICIT : COMPILER_ROOTS_CONCRETE_DEFINITIONS);
        assert(valid == !explicit);
        if (valid) {
            assert(compiler_frontend_finish(&frontend, NULL));
            assert(!semantic_find_type(&frontend.semantic, "/test/fixture/v1.0.0/src/extra"));
        } else { assert(!frontend.semantic_initialized && !frontend.plan_initialized); }
        destroy_frontend();
    }
}

/** Unused required source definitions still undergo lexical checks before root expansion. */
static void it_rejects_invalid_uninstantiated_loaded_definitions_before_semantic_planning(void) {
    prepare_sources("@!\nanchor&\n/test/fixture/v1.0.0/src/value\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/extra", "@<\ninput\n.slot\n@!\nmissing !\n");
    setup_frontend();
    ModuleRoot root = {"generated/project", NULL, 0, MODULE_ROOT_PROJECT};
    const char *required[] = {"/test/fixture/v1.0.0/src/extra"};
    assert(compiler_frontend_load_required(&frontend, &root, 1, "/test/fixture/v1.0.0/src/main", required, 1));
    assert(!compiler_frontend_finish(&frontend, NULL));
    assert(frontend.specialization_initialized && !frontend.specializations.valid);
    assert(!frontend.semantic_initialized && !frontend.plan_initialized && !frontend.valid);
    assert(frontend.error.primary.span.source == project_find(&frontend.project, "/test/fixture/v1.0.0/src/extra")->source->text);
    destroy_frontend();
}

/** An explicit list cannot omit the entry that finish would otherwise plan,
 * even when another independently selected module is a valid ordinary root. */
static void it_rejects_root_lists_that_omit_the_loaded_frontend_entry(void) {
    for (size_t empty = 0; empty < 2; empty++) {
        prepare_sources("@!\nanchor&\n/test/fixture/v1.0.0/src/value\n");
        setup_frontend();
        ModuleRoot root = {"generated/project", NULL, 0, MODULE_ROOT_PROJECT};
        const char *required[] = {"/test/fixture/v1.0.0/src/extra"};
        assert(compiler_frontend_load_required(&frontend, &root, 1, "/test/fixture/v1.0.0/src/main", required, 1));
        const ProjectModule *selected[] = {project_find(&frontend.project, "/test/fixture/v1.0.0/src/extra")};
        assert(!compiler_frontend_specialize(&frontend, empty ? NULL : selected, empty ? 0 : 1));
        assert(frontend.specialization_initialized && !frontend.specializations.valid);
        assert(diagnostic_valid(&frontend.error) && frontend.error.primary.span.source == frontend.project.entry->source->text);
        assert(!frontend.semantic_initialized && !frontend.plan_initialized && !frontend.valid);
        assert(!compiler_frontend_finish(&frontend, NULL));
        assert(!frontend.semantic_initialized && !frontend.plan_initialized);
        destroy_frontend();
    }
}

/** Dispatch current-stage evidence; no generated execution is claimed here. */
static void run(void) {
    it_exposes_closed_generic_type_candidates_before_ordinary_admission();
    it_composes_explicit_multi_root_closure_with_the_real_frontend();
    it_checks_concrete_projects_through_the_public_frontend_entry();
    it_checks_generic_operations_through_the_actual_frontend_adapter();
    it_plans_closed_static_exports_as_compile_time_metadata();
    it_refuses_type_root_array_overflow_before_allocation();
    it_distinguishes_catalog_definition_and_explicit_type_root_requests();
    it_rejects_invalid_uninstantiated_loaded_definitions_before_semantic_planning();
    it_rejects_root_lists_that_omit_the_loaded_frontend_entry();
}
/** Counted outer teardown releases plans, semantic borrowers, Type owners and sources. */
int main(void) { run(); return 0; }
