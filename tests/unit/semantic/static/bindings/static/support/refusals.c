/** @file
 * @brief Check duplicate/foreign labels and every allocation refusal in current named requests.
 */

/** Duplicate names are contradictory even with unknown origins. Unknown names
 * remain pending until actual metadata independently establishes their absence.
 */
static void it_rejects_duplicate_and_unknown_named_graph_requests(void) {
    const char *cases[] = {
        "@...\n.other\n ...a < ...value\n ...a < ...value\n ...out > ...one\n",
        "@...\n.other\n ...out > ...one\n ...out > ...two\n",
        "@...\n.other\n ...unknown < ...value\n ...out > ...one\n",
        "@...\n.other\n ...unknown > ...one\n"
    };
    for (size_t index = 0; index < 4; index++) {
        for (size_t known = 0; known < 2; known++) {
            setup_named_requests(cases[index]);
            BindingInterface *view = known != 0 ? named_interface(NULL, 0) : NULL;
            BindingApplication *application = NULL;
            bool success = binding_normalize_static(&binding_plan, view, expression, &application);
            assert(success == (index >= 2 && known == 0));
            if (success) { assert(application->pending); }
            else {
                assert(application == NULL && binding_plan.status == BINDING_INVALID);
                assert(diagnostic_valid(&binding_plan.error));
                assert(binding_plan.first_application == NULL);
            }
            destroy_named_requests();
        }
    }
}

/** Every request preparation allocation is refused once, without publishing an
 * incomplete application or mutating the independent interface snapshot.
 */
static void it_unwinds_every_named_graph_request_allocation_failure(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        setup_named_requests("@...\n.other\n ...a < ...value\n ...out > ...one\n");
        BindingInterface *view = named_interface(NULL, 0);
        BindingApplication *application = NULL;
        size_t before = allocation_count;
        if (failure != 0) { failure_at = before + failure; }
        bool success = binding_normalize_static(&binding_plan, view, expression, &application);
        if (failure == 0) { assert(success); total = allocation_count - before; assert(total != 0); }
        else {
            assert(!success && binding_plan.status == BINDING_OUT_OF_MEMORY && application == NULL);
            assert(binding_plan.first_application == NULL && diagnostic_valid(&binding_plan.error));
        }
        assert(view->sequences[BINDING_STATIC_INPUT].count == 2 && producer->parameter_count == 2);
        destroy_named_requests();
    }
}
