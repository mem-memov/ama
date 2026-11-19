/** @file
 * @brief Verify source-order runtime events, stable sorting and failure ownership without concrete Types.
 */
#include "../common.c"

/** Parser/project-valid calls retain lexical operands even before interface checking. */
static const char *event_source = "@=\nshared\n.other\n |0\n@<\ninput\n...type\n shared\n"
    "@>\noutput\n.other\n shared\n@!\nanchor&\n..integer\nworker\n.other\nworker !\n < input\n"
    " child !\n  < shared\n > fresh\n:\n fresh\n  shared\nflags[item&]\n{\n fresh\n  shared\n}\n";

/** Original written call operands are interleaved despite separate AST lists;
 * static formals and Type heads introduce no runtime events or occurrences.
 */
static void it_orders_original_runtime_operands_across_interfaces_selected_calls_and_handlers(void) {
    setup_connections(event_source, "@!\n");
    StaticRuntimeModule module = {.source = caller_module};
    assert(static_runtime_events(&connections, &module));
    static_runtime_sort(&module);
    StaticRuntimeEventKind kinds[] = {STATIC_RUNTIME_DECLARATION, STATIC_RUNTIME_INITIALIZER,
        STATIC_RUNTIME_DECLARATION, STATIC_RUNTIME_INITIALIZER, STATIC_RUNTIME_DECLARATION,
        STATIC_RUNTIME_INITIALIZER, STATIC_RUNTIME_DECLARATION, STATIC_RUNTIME_DECLARATION,
        STATIC_RUNTIME_CALL, STATIC_RUNTIME_BINDING, STATIC_RUNTIME_CALL, STATIC_RUNTIME_BINDING,
        STATIC_RUNTIME_BINDING, STATIC_RUNTIME_ASSIGNMENT, STATIC_RUNTIME_FOREACH, STATIC_RUNTIME_ASSIGNMENT};
    assert(module.event_count == sizeof(kinds) / sizeof(*kinds));
    const StaticRuntimeEvent *event = module.first_event;
    size_t previous = 0;
    for (size_t index = 0; index < sizeof(kinds) / sizeof(*kinds); index++) {
        assert(event && event->kind == kinds[index] && event->site.start >= previous);
        previous = event->site.start;
        assert(event->node != NULL && event->site.source == project.entry->source->text);
        if (index < 2) { assert(event->context == STATIC_CONTEXT_CONSTANT); }
        else if (index < 4) { assert(event->context == STATIC_CONTEXT_INPUT); }
        else if (index < 6) { assert(event->context == STATIC_CONTEXT_RESULT); }
        else { assert(event->context == STATIC_CONTEXT_BODY); }
        if (event->kind == STATIC_RUNTIME_INITIALIZER) { assert(event->source && event->argument == NULL); }
        if (event->kind == STATIC_RUNTIME_BINDING) { assert(event->argument && event->source == NULL); }
        if (event->next == NULL) { assert(event == module.last_event); }
        event = event->next;
    }
    assert(event == NULL && module.first_field == NULL && solver.equation_count == 0);
    destroy_connections();
}

/** Independent monotonicity/stability checks cover empty/singleton, tied keys
 * and all merge-width boundaries through 33 events; sorting allocates nothing.
 */
static void it_preserves_runtime_event_identity_and_stable_equal_sites_at_merge_boundaries(void) {
    setup_connections("@!\nanchor&\n..integer\n", "@!\n");
    for (size_t count = 0; count <= 33; count++) {
        for (size_t ties = 0; ties < 2; ties++) {
            StaticRuntimeEvent events[33] = {0};
            StaticRuntimeModule module = {.first_event = count ? events : NULL, .event_count = count};
            for (size_t index = 0; index < count; index++) {
                events[index].next = index + 1 < count ? &events[index + 1] : NULL;
                events[index].site.start = ties ? (count - index - 1) / 3 : count - index - 1;
            }
            size_t before = allocation_count;
            static_runtime_sort(&module);
            assert(before == allocation_count && module.event_count == count);
            bool seen[33] = {false};
            size_t visited = 0;
            const StaticRuntimeEvent *previous = NULL;
            for (const StaticRuntimeEvent *event = module.first_event; event; event = event->next) {
                size_t identity = (size_t)(event - events);
                assert(identity < count && !seen[identity]);
                seen[identity] = true;
                if (previous) {
                    assert(previous->site.start <= event->site.start);
                    if (previous->site.start == event->site.start) { assert(previous < event); }
                }
                previous = event;
                visited++;
            }
            assert(visited == count && module.last_event == previous);
        }
    }
    destroy_connections();
}

/** Refuse every event preparation allocation; a failed owner retains its first
 * source diagnostic and every partial frame remains destructible.
 */
static void it_unwinds_every_runtime_event_preparation_allocation_refusal(void) {
    size_t total = 0;
    for (size_t cut = 0; cut <= total; cut++) {
        setup_connections(event_source, "@!\n");
        StaticRuntimeModule module = {.source = caller_module};
        size_t before = allocation_count;
        if (cut) { failure_at = before + cut; }
        bool success = static_runtime_events(&connections, &module);
        if (!cut) { assert(success); total = allocation_count - before; assert(total > 0); }
        else {
            assert(!success && connections.status == STATIC_CONNECTION_OUT_OF_MEMORY && diagnostic_valid(&connections.error));
            const char *message = connections.error.message;
            assert(!static_connection_fail(&connections, STATIC_CONNECTION_CONFLICT, (SourceSpan){0}, "later"));
            assert(connections.error.message == message && static_connection_allocate(&connections, 1) == NULL);
        }
        failure_at = 0;
        destroy_connections();
    }
}

/** Borrowed errors keep exact status/provenance; healthy unresolved terms alone
 * do not make metadata ownership fail or execute any expression/body work.
 */
static void it_preserves_borrowed_dependency_error_status_and_source_provenance(void) {
    const StaticConstantStatus statuses[] = {STATIC_CONSTANT_INVALID, STATIC_CONSTANT_CONFLICT,
        STATIC_CONSTANT_UNRESOLVED, STATIC_CONSTANT_OUT_OF_MEMORY};
    const StaticConnectionStatus expected[] = {STATIC_CONNECTION_INVALID, STATIC_CONNECTION_CONFLICT,
        STATIC_CONNECTION_UNRESOLVED, STATIC_CONNECTION_OUT_OF_MEMORY};
    for (size_t index = 0; index < 4; index++) {
        setup_connections("@<\ninput\n...type\n@!\nanchor&\n..integer\n", "@!\n");
        assert(static_connection_dependencies(&connections) && connections.status == STATIC_CONNECTION_OK);
        assert(!static_constant_fail(&constants, statuses[index], caller_module->first->site, "original constant failure"));
        assert(!static_connection_dependencies(&connections));
        assert(connections.status == expected[index] && connections.error.message == constants.error.message);
        assert(connections.error.primary.span.start == constants.error.primary.span.start && diagnostic_valid(&connections.error));
        destroy_connections();
    }
}

/** Run source order, sorting, allocation and dependency composition partitions. */
static void run(void) {
    it_orders_original_runtime_operands_across_interfaces_selected_calls_and_handlers();
    it_preserves_runtime_event_identity_and_stable_equal_sites_at_merge_boundaries();
    it_unwinds_every_runtime_event_preparation_allocation_refusal();
    it_preserves_borrowed_dependency_error_status_and_source_provenance();
}
/** Return only after all assertions and counted teardown. */
int main(void) { run(); return 0; }
