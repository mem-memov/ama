/** @file
 * @brief Check all skip prefixes and the first excess in independent static input/output sequences.
 */

/** A two-slot interface admits each 0..2 prefix independently; a third explicit
 * skip is an arity error, while unknown actual interfaces keep an obligation.
 * This is field mapping evidence, not producer type-closure acceptance.
 */
static void it_checks_every_static_skip_prefix_and_first_excess_position(void) {
    for (size_t inputs = 0; inputs <= 3; inputs++) {
        for (size_t outputs = 0; outputs <= 3; outputs++) {
            for (size_t known = 0; known < 2; known++) {
                char body[128];
                size_t used = (size_t)snprintf(body, sizeof(body), "field\n.other\n");
                for (size_t index = 0; index < inputs; index++) { memcpy(body + used, " ... <\n", 7); used += 7; }
                for (size_t index = 0; index < outputs; index++) { memcpy(body + used, " ... >\n", 7); used += 7; }
                assert(used < sizeof(body));
                body[used] = 0;
                setup_requests(body);
                BindingInterface *view = known != 0 ? request_interface() : NULL;
                BindingApplication *application = NULL;
                bool success = binding_normalize_static(&binding_plan, view, request_head(".other")->expression, &application);
                assert(success == (known == 0 || (inputs <= 2 && outputs <= 2)));
                if (success) {
                    assert(application->pending == (known == 0) && application->input_count == inputs && application->output_count == outputs);
                    assert(application->mode == ((inputs + outputs) == 0 ? BINDING_MODE_EMPTY : BINDING_MODE_POSITIONAL));
                    for (const BindingRequest *request = application->first; request != NULL; request = request->next) {
                        assert(request->omitted && request->syntax.static_binding->omitted);
                        if (known != 0) { assert(request->slot->sequence_ordinal == request->source_ordinal); }
                        else { assert(request->slot == NULL); }
                    }
                } else { assert(application == NULL && binding_plan.status == BINDING_INVALID); }
                destroy_binding();
            }
        }
    }
}
