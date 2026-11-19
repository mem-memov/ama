/** @file
 * @brief Check independent ordinary positions, source payload identity and exact storage labels.
 */

/** Positional omissions consume both direction positions; named order instead
 * follows exact field identities and preserves reference/list label contracts.
 */
static void it_keeps_runtime_omissions_and_named_storage_labels_in_independent_sequences(void) {
    const char *bodies[] = {
        "worker !\n <\n <\n >\n > received list[]\n",
        "worker !\n second[] < source[]\n first& < input&\n y[] > received list[]\n x& > received ref&\n"
    };
    for (size_t named = 0; named < 2; named++) {
        setup_worker(bodies[named]);
        BindingInterface *view = runtime_interface();
        const BindingSlot *first = view->sequences[BINDING_RUNTIME_INPUT].first;
        const AstNode *call = operation("worker");
        BindingApplication *application = NULL;
        assert(binding_normalize_runtime(&binding_plan, view, call, &application));
        assert(!application->pending && application->kind == BINDING_RUNTIME_APPLICATION);
        assert(application->input_count == 2 && application->output_count == 2);
        assert(application->mode == (named != 0 ? BINDING_MODE_NAMED : BINDING_MODE_POSITIONAL));
        const AstArgument *raw = call->as.call.first;
        const BindingRequest *request = application->first;
        for (size_t index = 0; index < 4; index++) {
            assert(request != NULL && request->syntax.runtime_binding == raw);
            assert(request->source_ordinal == index % 2);
            assert(request->slot->original_ordinal == (named != 0 ? 1 - index % 2 : index % 2));
            assert(request->direction == (index < 2 ? BINDING_RUNTIME_INPUT : BINDING_RUNTIME_OUTPUT));
            assert(request->omitted == (named == 0 && index < 3));
            request = request->next;
            raw = raw->next;
        }
        assert(request == NULL && view->sequences[BINDING_RUNTIME_INPUT].first == first);
        assert(first->next->sequence_ordinal == 1 && view->sequences[BINDING_STATIC_OUTPUT].count == 0);
        destroy_binding();
    }
}
