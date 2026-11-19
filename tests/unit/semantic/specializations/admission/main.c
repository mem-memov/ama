/** @file
 * @brief Verify ordinary checking precedes candidate certificates across generic operations and ownership failures.
 */
#include "../common.c"
#include "semantic/specializations/check.h"

/** Registry certificate is independent of the later constant execution plan. */
static void checked(void) {
    assert(semantics.resolved && semantics.relations_valid && !semantics.constants_valid);
    for (const StaticRegistryCandidate *candidate = analysis.registry.first_candidate; candidate; candidate = candidate->next) {
        assert(candidate->local_complete && candidate->state == STATIC_CANDIDATE_CHECKED);
    }
}

/** Real generic calls share checked metadata while keeping the source-bound result field. */
static void it_admits_fully_checked_generic_calls_and_finite_nominal_groups(void) {
    const char *mains[] = {"@!\nknown\n..integer\nworker\n.other\nworker !\n < known\n > fresh\n",
        "@!\nnext&\n.main\nchildren[]\n.main\n"};
    for (size_t index = 0; index < 2; index++) {
        setup(mains[index], index ? "@!\n" : "@<\ninput\n...slot\n@>\noutput\n...slot\n@!\noutput\n input\n", true);
        assert(semantic_specializations_check(&semantics, &analysis.closure, NULL) == SEMANTIC_OK);
        checked();
        if (!index) {
            SemanticType *main_type = semantics.first;
            while (main_type->module != project.entry) { main_type = main_type->next; }
            assert(field(main_type, "fresh")->type == field(main_type, "known")->type);
        }
        destroy();
    }
}

/** Unsupported literal/condition/index/copy operations all fail before local completion. */
static void it_rejects_unsupported_substituted_operations_before_admission(void) {
    const char *bodies[] = {
        "@<\ninput\n...slot\n |0\n@!\n",
        "@<\ninput\n...slot\n@!\ninput\n?\n",
        "@<\ninput\n...slot\n@!\nitems[]\n..integer\nitems[input] !\n",
        "@<\ninput\n...slot\n@!\n",
        "@<\ninput\n...slot\n@!\nguard\n..integer\n |0\nguard\n?\n input\n ?\n"};
    for (size_t resource = 0; resource < 2; resource++) {
        for (size_t index = 0; index < 5; index++) {
            directory(STATIC_SCOPE_ROOT_DIRECTORY);
            file("generated/static scope/resource", "@C\n");
            file("generated/static scope/resource.h", "/* Fixture native payload. */\n");
            file("generated/static scope/resource.c", "/* Metadata-only native fixture. */\n");
            file("generated/static scope/resource.abi", index == 3 ? "+ initialize\n- destroy\n" : "+ initialize\n- destroy\n= clone\n");
            setup(resource ? "@!\nknown\n.resource\nworker\n.other\nworker !\n < known\n"
                : "@!\nknown\n..integer\nworker\n.other\nworker !\n < known\n", bodies[index], true);
            SemanticStatus status = semantic_specializations_check(&semantics, &analysis.closure, NULL);
            assert(status == (resource ? SEMANTIC_INVALID : SEMANTIC_OK));
            if (!resource) { checked(); }
            else {
                assert(diagnostic_valid(&semantics.error) && !semantics.constants_valid);
                for (const StaticRegistryCandidate *candidate = analysis.registry.first_candidate; candidate; candidate = candidate->next) {
                    assert(!candidate->local_complete && candidate->state != STATIC_CANDIDATE_CHECKED);
                }
            }
            destroy();
        }
    }
}

/** All ordinary and static-producer dependencies close after explicit named namespace export selection. */
static void it_admits_named_static_exports_and_all_candidate_dependencies(void) {
    directory(STATIC_SCOPE_ROOT_DIRECTORY);
    file("generated/static scope/container", "@=\nempty[]\n...slot\n@<\ninput\n...slot\n@!\n");
    setup("@!\n.other\n ...numbers\n empty[] >> xs[]\n.other\n ...texts\n empty[] >> ys[]\n",
        "@...\n...numbers\n.container\n ...slot < ..integer\n...texts\n.container\n ...slot < ..text\n", true);
    assert(semantic_specializations_check(&semantics, &analysis.closure, NULL) == SEMANTIC_OK);
    assert(analysis.registry.dependency_count > 0);
    checked();
    destroy();
}

/** Unused open definitions need no runtime instance or invented concrete Type. */
static void it_checks_definition_only_closure_without_runtime_types(void) {
    setup("@!\n", "@<\ninput\n...slot\n@!\n", false);
    assert(semantic_specializations_check(&semantics, &analysis.closure, NULL) == SEMANTIC_OK);
    assert(!semantics.first && !analysis.registry.candidate_count);
    checked();
    destroy();
}

/** Missing clone capability blocks copies, while empty owned lists and handles may still exist. */
static void it_admits_noncopyable_storage_without_inventing_copy_operations(void) {
    const char *mains[] = {"@!\nowned[]\n.other\n", "@!\nborrowed&\n.other\n"};
    for (size_t index = 0; index < 2; index++) {
        directory(STATIC_SCOPE_ROOT_DIRECTORY);
        file("generated/static scope/other.h", "/* Fixture native payload. */\n");
        file("generated/static scope/other.c", "/* Metadata-only native fixture. */\n");
        file("generated/static scope/other.abi", "+ initialize\n- destroy\n");
        setup(mains[index], "@C\n", true);
        assert(semantic_specializations_check(&semantics, &analysis.closure, NULL) == SEMANTIC_OK);
        checked();
        SemanticType *main_type = semantics.first;
        while (main_type->module != project.entry) { main_type = main_type->next; }
        assert(main_type->cloneable == (index == 1));
        destroy();
    }
}

/** A constants-only namespace is metadata, never the instance Type of a runtime field. */
static void it_refuses_constant_namespaces_as_runtime_field_types(void) {
    setup("@!\ninvalid\n.other\n", "@=\nvalue\n..integer\n |0\n", true);
    assert(analysis.closure.closed);
    assert(semantic_specializations_check(&semantics, &analysis.closure, NULL) == SEMANTIC_INVALID);
    assert(diagnostic_valid(&semantics.error));
    assert(strstr(semantics.error.message, "executable module"));
    assert(!semantics.names_valid && !semantics.constants_valid);
    destroy();
}

/** A finite nominal identity can still require infinite eager runtime construction. */
static void it_refuses_eager_cycles_without_rejecting_type_identity_closure(void) {
    setup("@!\nchild\n.main\n", "@!\n", true);
    assert(analysis.closure.closed);
    assert(semantic_specializations_check(&semantics, &analysis.closure, NULL) == SEMANTIC_INVALID);
    assert(semantics.eager.status == GRAPH_CYCLE && !semantics.relations_valid);
    assert(!analysis.registry.first_candidate->local_complete);
    destroy();
}

/** Every ordinary-check/certificate allocation can refuse with exact classification and safe teardown. */
static void it_unwinds_every_ordinary_admission_allocation_refusal(void) {
    size_t total = 0;
    for (size_t cut = 0; cut <= total; cut++) {
        setup("@!\nknown\n..integer\nworker\n.other\nworker !\n < known\n > fresh\n",
            "@<\ninput\n...slot\n@>\noutput\n...slot\n@!\noutput\n input\n", true);
        size_t before = allocation_count;
        if (cut) { failure_at = before + cut; }
        SemanticStatus status = semantic_specializations_check(&semantics, &analysis.closure, NULL);
        if (!cut) { assert(status == SEMANTIC_OK); checked(); total = allocation_count - before; assert(total); }
        else { assert(status == SEMANTIC_OUT_OF_MEMORY && diagnostic_valid(&semantics.error)); }
        failure_at = 0;
        destroy();
    }
    printf("ordinary specialization admission allocation refusal positions: %zu\n", total);
}

/** Explicit dispatch covers complete generic, finite-cycle and managed-failure boundaries. */
static void run(void) {
    it_admits_fully_checked_generic_calls_and_finite_nominal_groups();
    it_rejects_unsupported_substituted_operations_before_admission();
    it_admits_named_static_exports_and_all_candidate_dependencies();
    it_checks_definition_only_closure_without_runtime_types();
    it_admits_noncopyable_storage_without_inventing_copy_operations();
    it_refuses_constant_namespaces_as_runtime_field_types();
    it_refuses_eager_cycles_without_rejecting_type_identity_closure();
    it_unwinds_every_ordinary_admission_allocation_refusal();
}
/** Run every explicit assertion case and report success after counted teardown. */
int main(void) { run(); return 0; }
