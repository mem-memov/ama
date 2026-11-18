/** @file
 * @brief Verify every ordinary skip prefix and first excess against independent actual directions.
 */

/** All 0..2 input/output skip prefixes are legal and keep their own counter;
 * the third skip fails only when actual arity is supplied. No default executes.
 */
static void it_checks_every_runtime_skip_prefix_and_first_excess_position(void) {
    for (size_t inputs = 0; inputs <= 3; inputs++) {
        for (size_t outputs = 0; outputs <= 3; outputs++) {
            for (size_t known = 0; known < 2; known++) {
                char body[96];
                size_t used = (size_t)snprintf(body, sizeof(body), "worker !\n");
                for (size_t index = 0; index < inputs; index++) { memcpy(body + used, " <\n", 3); used += 3; }
                for (size_t index = 0; index < outputs; index++) { memcpy(body + used, " >\n", 3); used += 3; }
                assert(used < sizeof(body));
                body[used] = 0;
                setup_worker(body);
                BindingInterface *view = known != 0 ? runtime_interface() : NULL;
                BindingApplication *application = NULL;
                bool success = binding_normalize_runtime(&binding_plan, view, operation("worker"), &application);
                assert(success == (known == 0 || (inputs <= 2 && outputs <= 2)));
                if (success) {
                    assert(application->input_count == inputs && application->output_count == outputs && application->pending == (known == 0));
                    assert(application->mode == ((inputs + outputs) == 0 ? BINDING_MODE_EMPTY : BINDING_MODE_POSITIONAL));
                    for (const BindingRequest *request = application->first; request != NULL; request = request->next) {
                        assert(request->omitted && request->syntax.runtime_binding->omitted);
                        if (known != 0) { assert(request->slot->sequence_ordinal == request->source_ordinal); }
                        else { assert(request->slot == NULL); }
                    }
                } else { assert(application == NULL && binding_plan.status == BINDING_INVALID); }
                destroy_binding();
            }
        }
    }
}
