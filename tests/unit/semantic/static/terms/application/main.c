/** @file
 * @brief Verify prepared operand handoff from actual known and pending binding requests.
 */
#include <stdint.h>
#include "../../bindings/request-common.c"
#include "semantic/static/terms/application.h"

/** Known named slots retain original formal identity; pending requests
 * retain their complete labels. Equal inputs never merge distinct application uses.
 */
static void it_preserves_named_slots_and_distinct_use_provenance(void) {
    for (size_t known = 0; known < 2; known++) {
        setup_requests("...request result\n...expected\n ...a < ...type\n ...b < ...type\n");
        const StaticModule *caller = binding_module(project.entry);
        const StaticOccurrence *head = request_head("...expected");
        assert(static_name_site(project.entry, "...expected").source == head->site.source);
        BindingInterface *view = known ? request_interface() : NULL;
        BindingApplication *request;
        assert(binding_normalize_static(&binding_plan, view, head->expression, &request));
        StaticTermStore store;
        static_term_store_init(&store, &inventory, sources.allocator);
        const StaticTermEnvironment *environment;
        const StaticTerm *value, *constructor;
        assert(static_term_environment(&store, caller, NULL, &environment));
        assert(static_term_symbol(&store, environment, static_symbol_find(caller, static_name_site(project.entry, "...type")), &value));
        assert(static_term_symbol(&store, environment, head->symbol, &constructor));
        StaticTermOperand operands[] = {{.request = request->first, .value = value},
            {.request = request->first->next, .value = value}};
        const StaticTermApplication *first, *second;
        assert(static_term_application(&store, &binding_plan, request, environment, constructor, operands, 2, &first));
        assert(static_term_application(&store, &binding_plan, request, environment, constructor, operands, 2, &second));
        assert(first != second && first->head == second->head && first->source == request);
        assert(first->input_count == 2 && first->first->value == value && first->last->value == value);
        assert(first->last->request->named && request->output_count == 0);
        if (known) {
            assert(first->first->request->slot->static_symbol == binding_module(project_find(&project, ".other"))->first);
        } else { assert(first->first->request->slot == NULL && request->pending); }
        const StaticTermApplication *sentinel = first;
        assert(!static_term_application(&store, &binding_plan, request, environment, constructor, NULL, 0, &sentinel));
        assert(sentinel == first && store.application_count == 2 && inventory.valid);
        assert(binding_plan.status == BINDING_OK);
        static_term_store_destroy(&store);
        destroy_binding();
    }
}

/** Refuse every handoff allocation while preserving previously published uses,
 * borrowed request metadata and exact source sites.
 */
static void it_unwinds_every_application_record_allocation_refusal(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        setup_requests("...request result\n...expected\n ...a < ...type\n ...b < ...type\n");
        const StaticModule *caller = binding_module(project.entry);
        const StaticOccurrence *head = request_head("...expected");
        assert(static_name_site(project.entry, "...expected").source == head->site.source);
        BindingApplication *request;
        assert(binding_normalize_static(&binding_plan, NULL, head->expression, &request));
        StaticTermStore store;
        static_term_store_init(&store, &inventory, sources.allocator);
        const StaticTermEnvironment *environment;
        const StaticTerm *value, *constructor;
        assert(static_term_environment(&store, caller, NULL, &environment));
        assert(static_term_symbol(&store, environment, static_symbol_find(caller, static_name_site(project.entry, "...type")), &value));
        assert(static_term_symbol(&store, environment, head->symbol, &constructor));
        StaticTermOperand operands[] = {{.request = request->first, .value = value},
            {.request = request->first->next, .value = value}};
        const StaticTermApplication *prior, *output;
        assert(static_term_application(&store, &binding_plan, request, environment, constructor, operands, 2, &prior));
        output = prior;
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        bool success = static_term_application(&store, &binding_plan, request, environment, constructor, operands, 2, &output);
        if (!failure) { assert(success); total = allocation_count - before; assert(total); }
        else {
            assert(!success && output == prior && prior->next == NULL);
            assert(store.application_count == 1 && store.status == STATIC_TERM_OUT_OF_MEMORY);
            assert(request->pending && binding_plan.status == BINDING_OK);
        }
        failure_at = 0;
        static_term_store_destroy(&store);
        destroy_binding();
    }
}

/** Prepared operand order cannot renumber actual formals; foreign source uses
 * and saturated publication counts refuse without mutating prior records.
 */
static void it_preserves_reordered_operands_and_refuses_foreign_or_saturated_uses(void) {
    for (size_t invalid = 0; invalid < 3; invalid++) {
        setup_requests("...request result\n...expected\n ...a < ...type\n ...b < ...other type\n");
        const StaticModule *caller = binding_module(project.entry);
        const StaticOccurrence *head = request_head("...expected");
        BindingApplication *request;
        assert(binding_normalize_static(&binding_plan, request_interface(), head->expression, &request));
        StaticTermStore store;
        static_term_store_init(&store, &inventory, sources.allocator);
        const StaticTermEnvironment *environment, *foreign;
        const StaticTerm *value, *second_value, *constructor;
        assert(static_term_environment(&store, caller, NULL, &environment));
        assert(static_term_environment(&store, binding_module(project_find(&project, ".other")), NULL, &foreign));
        assert(static_term_symbol(&store, environment, static_symbol_find(caller, static_name_site(project.entry, "...type")), &value));
        assert(static_term_symbol(&store, environment, head->symbol, &constructor));
        assert(static_term_symbol(&store, environment, static_symbol_find(caller,
            static_name_site(project.entry, "...other type")), &second_value));
        StaticTermOperand operands[] = {{.request = request->first->next, .value = second_value},
            {.request = request->first, .value = value}};
        const StaticTermApplication *application;
        assert(static_term_application(&store, &binding_plan, request, environment, constructor, operands, 2, &application));
        assert(application->first->value == value && application->last->value == second_value);
        const StaticTermApplication *output = application;
        BindingApplication copied = *request;
        if (invalid == 2) { store.application_count = SIZE_MAX; }
        size_t before = allocation_count;
        assert(!static_term_application(&store, &binding_plan, invalid == 0 ? &copied : request,
            invalid == 1 ? foreign : environment, constructor, operands, 2, &output));
        assert(allocation_count == before && output == application && application->next == NULL);
        assert(store.status == (invalid == 2 ? STATIC_TERM_OUT_OF_MEMORY : STATIC_TERM_INVALID));
        if (invalid == 2) { store.application_count = 1; }
        static_term_store_destroy(&store);
        destroy_binding();
    }
}

/** Exercise real scope, request and term components together. */
static void run(void) {
    it_preserves_named_slots_and_distinct_use_provenance();
    it_unwinds_every_application_record_allocation_refusal();
    it_preserves_reordered_operands_and_refuses_foreign_or_saturated_uses();
}
/** Return success after all assertions and counted owner teardown. */
int main(void) { run(); return 0; }
