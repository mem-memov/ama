/** @file
 * @brief Verify exact producer input closure and refusal of noninvertible static inference.
 */
#include "../common.c"
#include "semantic/static/publication/closure/internal.h"

/** One output does not determine an independent second input; explicit binding does. */
static void it_requires_all_inputs_even_when_one_selected_output_is_known(void) {
    for (size_t supplied = 0; supplied < 2; supplied++) {
        setup_publications(supplied ?
            "@...\n...expected\n..integer\n.other\n ...key < ..integer\n ...out > ...expected\n" :
            "@...\n...expected\n..integer\n.other\n ...out > ...expected\n",
            "@...\n...out\n...value\n...other\n...key\n");
        const StaticResultContext *context = publication_context(caller_environment);
        const StaticPublication *publication = NULL;
        bool success = static_publication_resolve(&publications, context, result_projection(0), &publication);
        assert(success == (supplied != 0));
        if (success) {
            assert(publication->value == NULL && publication->output_count == 2);
            assert(publication->first_output->value == integer_type);
            assert(publication->first_output->next->value == integer_type);
            assert(publication->context->registry_use->candidate->key->producer_applied &&
                publication->context->registry_use->candidate->key->definition->parameter_count == 2);
        } else {
            assert(publication == NULL && publications.status == STATIC_PUBLICATION_UNRESOLVED);
        }
        destroy_publications();
    }
}

/** A concrete constant output supplies no evidence for an unrelated free input. */
static void it_refuses_unused_input_underdetermination_without_publishing_aliases(void) {
    setup_publications("@...\n...expected\n..integer\n.other\n ...out > ...expected\n",
        "@...\n...out\n..integer\n...other\n...unused\n");
    const StaticResultContext *context = publication_context(caller_environment);
    const StaticPublication *publication = NULL;
    assert(!static_publication_resolve(&publications, context, result_projection(0), &publication));
    assert(publication == NULL && publications.count == 0 && publications.alias_count == 0);
    assert(publications.status == STATIC_PUBLICATION_UNRESOLVED);
    destroy_publications();
}

/** A metadata expression may close without the unrelated lexical owner's full key;
 * requesting the whole root must still reject that incomplete owner.
 */
static void it_separates_intermediate_value_facts_from_whole_root_publication(void) {
    setup_publications("@...\n...known\n..integer\n...other\n...unknown\n.other\n ...out > ...known\n",
        "@...\n...out\n..integer\n");
    const StaticResultContext *context = publication_context(caller_environment);
    const StaticPublication *value, *root = NULL;
    assert(static_publication_resolve(&publications, context, result_pair("...known"), &value));
    assert(value->value == integer_type && value->context == NULL);
    assert(!static_publication_scope(&publications, context, &root));
    assert(root == NULL && publications.status == STATIC_PUBLICATION_UNRESOLVED);
    assert(value->value == integer_type && publications.count == 1);
    destroy_publications();
}

/** Tagged proof payloads must be registered before any dereference. Invalid
 * domains and copied handles preserve the caller's outcome and first error.
 */
static void it_refuses_foreign_closure_handles_and_missing_outcome_receivers(void) {
    for (size_t kind = 0; kind < 5; kind++) {
        setup_publications("@...\n...out\n..integer\n", "@...\n");
        const StaticResultContext *context = publication_context(caller_environment);
        StaticResultGraph *graph;
        assert(static_result_require_graph(&results, context, &graph));
        StaticResultGraph copied = *graph;
        StaticPublicationItem root = {.kind = STATIC_CLOSURE_GRAPH, .as.graph = kind == 3 ? &copied : graph};
        if (kind < 3) { root = (StaticPublicationItem){.kind = kind == 0 ? STATIC_CLOSURE_SOURCE :
            kind == 1 ? STATIC_CLOSURE_DEFINITION : (StaticPublicationWorkKind)99}; }
        bool outcome = true;
        assert(!static_publication_dependencies_closed(&publications, root, kind == 4 ? NULL : &outcome));
        assert(outcome && publications.status == STATIC_PUBLICATION_INVALID);
        const char *message = publications.error.message;
        assert(!static_publication_dependencies_closed(&publications,
            (StaticPublicationItem){.kind = STATIC_CLOSURE_GRAPH, .as.graph = graph}, &outcome));
        assert(outcome && publications.error.message == message);
        destroy_publications();
    }
}

/** Explicitly execute the independent closure partitions. */
static void run(void) {
    it_refuses_foreign_closure_handles_and_missing_outcome_receivers();
    it_requires_all_inputs_even_when_one_selected_output_is_known();
    it_refuses_unused_input_underdetermination_without_publishing_aliases();
    it_separates_intermediate_value_facts_from_whole_root_publication();
}
/** Counted teardown must finish before successful process exit. */
int main(void) { run(); return 0; }
