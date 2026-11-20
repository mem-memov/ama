/** @file
 * @brief Check intrinsic list Type equations without stored method instances or execution.
 */
#include "../operations/common.c"

/** Source order prepares service parents before methods and scalar result calls. */
static void collect_list_calls(const StaticTypedScope *scope, const StaticResultContext *actual) {
    for (const StaticRuntimeEvent *event = scope->runtime->first_event; event; event = event->next) {
        if (event->kind != STATIC_RUNTIME_CALL) { continue; }
        bool changed;
        StaticResultOutcome outcome;
        assert(static_connection_bindings(&connections, actual, event->node, &changed, &outcome));
        assert(outcome == STATIC_RESULT_READY);
    }
}

/** Six canonical methods preserve element-reference ports and exact integer ports. */
static void it_connects_all_six_methods_without_creating_method_storage(void) {
    setup_connections("@!\nanchor\n//integer\nposition\n//integer\nvalues[]\n//integer\nvalues[]\n append !\n  < &anchor\n  > fresh&\n"
        " insert !\n  < position\n  < &anchor\n  > inserted&\n remove !\n  < position\n clear !\n swap !\n  < position\n  < position\n length !\n  > count\n", "@!\n");
    const StaticResultContext *actual;
    const StaticTypedScope *scope = prepare_operations(&actual);
    size_t fields = scope->field_count, contexts = results.context_count;
    collect_list_calls(scope, actual);
    size_t services = 0, methods = 0;
    for (const StaticTypedOperation *operation = connections.first_operation; operation; operation = operation->next) {
        assert(operation->element != NULL && operation->integer == integer_type && operation->target == NULL);
        if (operation->kind == STATIC_OPERATION_LIST_SERVICE) { services++; assert(!operation->bindings); }
        else { assert(operation->kind == STATIC_OPERATION_LIST_METHOD && operation->bindings->interface->kind == BINDING_LIST_INTERFACE); methods++; }
    }
    assert(services == 1 && methods == 6 && scope->field_count == fields && results.context_count == contexts);
    const char *names[] = {"fresh", "inserted", "count"};
    for (size_t i = 0; i < sizeof(names) / sizeof(*names); i++) {
        const StaticTypedField *field;
        assert(static_connection_field(&connections, scope, static_runtime_field(scope->runtime, static_name_site(project.entry, names[i])), &field));
        const StaticTerm *value;
        StaticConstraintOutcome closed;
        assert(static_constraint_value(&solver, field->type, &value, &closed));
        assert(closed == STATIC_CONSTRAINT_COMPLETE && value == integer_type);
    }
    destroy_connections();
}

/** Supplied element references can refine symbolic list Types for both ownership forms. */
static void it_infers_owned_and_reference_list_element_types_from_method_sources(void) {
    for (size_t references = 0; references < 2; references++) {
        setup_connections(references ? "@<\nseed\n.slot\n@!\nanchor\n//integer\nvalues&[]\n.slot\nvalues&[]\n append !\n  source& < &anchor\n  element& > fresh&\n" :
            "@<\nseed\n.slot\n@!\nanchor\n//integer\nvalues[]\n.slot\nvalues[]\n append !\n  source& < &anchor\n  element& > fresh&\n", "@!\n");
        const StaticResultContext *actual;
        const StaticTypedScope *scope = prepare_operations(&actual);
        collect_list_calls(scope, actual);
        const StaticTypedOperation *method = connections.first_operation;
        assert(method->kind == STATIC_OPERATION_LIST_METHOD && method->reference_elements == (references != 0));
        const StaticTerm *value;
        StaticConstraintOutcome closed;
        assert(static_constraint_value(&solver, method->element, &value, &closed));
        assert(closed == STATIC_CONSTRAINT_COMPLETE && value == integer_type);
        destroy_connections();
    }
}

/** Intrinsic scalar outputs support immediate ordinary invocation through public names. */
static void it_resolves_direct_element_and_length_result_operations(void) {
    setup_connections("@!\nanchor\n//integer\nvalues[]\n//integer\nvalues[]\n append !\n  < &anchor\n  element& !\n length !\n  value !\n", "@!\n");
    const StaticResultContext *actual;
    const StaticTypedScope *scope = prepare_operations(&actual);
    collect_list_calls(scope, actual);
    size_t ordinary = 0;
    for (const StaticTypedOperation *operation = connections.first_operation; operation; operation = operation->next) {
        if (operation->kind == STATIC_OPERATION_SOURCE) { assert(operation->target->owner == integer_type); ordinary++; }
    }
    assert(ordinary == 2);
    destroy_connections();
}

/** Method names/forms and source/result ownership never infer alternative operations. */
static void it_rejects_unknown_methods_modes_ports_and_intrinsic_result_suffixes(void) {
    const char *sources[] = {
        "@!\nvalues[]\n//integer\nvalues[]\n unknown !\n",
        "@!\nvalues[]\n//integer\nvalues[]\n append& !\n",
        "@!\nvalues[]\n//integer\nvalues[]\n append %\n",
        "@!\nanchor\n//integer\nvalues[]\n//integer\nvalues[]\n append !\n  < anchor\n",
        "@!\nvalues[]\n//integer\nvalues[]\n append !\n  element !\n",
        "@!\nvalues[]\n//integer\nvalues[]\n length !\n  > result&\n"
    };
    for (size_t kind = 0; kind < sizeof(sources) / sizeof(*sources); kind++) {
        setup_connections(sources[kind], "@!\n");
        const StaticResultContext *actual;
        const StaticTypedScope *scope = prepare_operations(&actual);
        bool failed = false;
        for (const StaticRuntimeEvent *event = scope->runtime->first_event; event; event = event->next) {
            if (event->kind != STATIC_RUNTIME_CALL) { continue; }
            bool changed;
            StaticResultOutcome outcome;
            if (!static_connection_bindings(&connections, actual, event->node, &changed, &outcome)) { failed = true; break; }
        }
        assert(failed && connections.status == STATIC_CONNECTION_CONFLICT);
        destroy_connections();
    }
}

/** Every service/method metadata and equation allocation failure retains owned partial work. */
static void it_unwinds_every_list_service_and_method_allocation_refusal(void) {
    for (size_t phase = 0; phase < 2; phase++) {
        size_t total = 0;
        for (size_t failure = 0; failure <= total; failure++) {
            setup_connections("@!\nanchor\n//integer\nvalues[]\n//integer\nvalues[]\n append !\n  < &anchor\n  > fresh&\n", "@!\n");
            const StaticResultContext *actual;
            const StaticTypedScope *scope = prepare_operations(&actual);
            bool changed;
            StaticResultOutcome outcome;
            if (phase) { assert(static_connection_bindings(&connections, actual, operation_at(scope, 0), &changed, &outcome)); }
            const AstNode *node = operation_at(scope, phase);
            size_t before = allocation_count;
            if (failure) { failure_at = before + failure; }
            changed = false;
            outcome = STATIC_RESULT_PENDING;
            bool success = static_connection_bindings(&connections, actual, node, &changed, &outcome);
            if (!failure) { assert(success); total = allocation_count - before; assert(total > 0); }
            else { assert(!success && !changed && outcome == STATIC_RESULT_PENDING && connections.status == STATIC_CONNECTION_OUT_OF_MEMORY); }
            failure_at = 0;
            destroy_connections();
        }
    }
}

/** Exercise canonical interfaces, element inference, immediate outputs and refusal paths. */
static void run(void) {
    it_connects_all_six_methods_without_creating_method_storage();
    it_infers_owned_and_reference_list_element_types_from_method_sources();
    it_resolves_direct_element_and_length_result_operations();
    it_rejects_unknown_methods_modes_ports_and_intrinsic_result_suffixes();
    it_unwinds_every_list_service_and_method_allocation_refusal();
}
/** Counted teardown completes before success. */
int main(void) { run(); return 0; }
