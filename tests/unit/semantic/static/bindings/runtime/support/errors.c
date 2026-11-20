/** @file
 * @brief Verify ordinary field errors and independent validation of invalid cloned request metadata.
 */

/** The parser already rejects mixed/order/mode/empty-named forms. Clone only
 * valid parsed request records for those defensive normalizer partitions; the
 * project AST and source bytes remain immutable. Missing actual interfaces defer
 * only checks requiring foreign field facts, never lexical contradictions.
 */
static void it_rejects_runtime_duplicate_storage_arity_and_cloned_mode_order_errors(void) {
    const char *bodies[] = {
        "worker !\n first < input&\n",
        "worker !\n unknown < input&\n",
        "worker !\n first& < input&\n first& < input&\n",
        "worker !\n <\n <\n <\n",
        "worker !\n >\n >\n >\n",
        "worker !\n first& < input&\n second[] < source[]\n x& > received ref&\n y[] > received list[]\n"
    };
    for (size_t partition = 0; partition < 9; partition++) {
        for (size_t known = 0; known < 2; known++) {
            setup_worker(bodies[partition < 5 ? partition : 5]);
            BindingInterface *view = known != 0 ? runtime_interface() : NULL;
            const AstNode *call = operation("worker");
            AstNode clone;
            AstArgument arguments[4];
            if (partition >= 5) {
                clone = *call;
                const AstArgument *raw = call->as.call.first;
                for (size_t index = 0; index < 4; index++) {
                    arguments[index] = *raw;
                    arguments[index].next = index + 1 < 4 ? &arguments[index + 1] : NULL;
                    raw = raw->next;
                }
                clone.as.call.first = arguments;
                clone.as.call.last = &arguments[3];
                if (partition == 5) { arguments[2].label = NULL; }
                if (partition == 6) {
                    clone.as.call.first = &arguments[2];
                    arguments[2].next = arguments;
                    arguments[1].next = &arguments[3];
                }
                if (partition == 7) { arguments[0].omitted = true; }
                if (partition == 8) { clone.as.call.mode = AST_RESUME_CALL; }
                call = &clone;
            }
            BindingApplication *application = NULL;
            bool success = binding_normalize_runtime(&binding_plan, view, call, &application);
            bool expected = known == 0 && (partition < 2 || partition == 3 || partition == 4);
            assert(success == expected);
            if (success) { assert(application->pending && application->first->slot == NULL); }
            else {
                assert(application == NULL && binding_plan.first_application == NULL && binding_plan.status == BINDING_INVALID);
                assert(diagnostic_valid(&binding_plan.error) && binding_plan.error.primary.span.source == project.entry->source->text);
                if (partition == 2) {
                    assert(binding_plan.error.related_count == (known != 0 ? 2 : 1));
                    if (known != 0) {
                        assert(binding_plan.related[1].location.span.source == project_find(&project, "/test/fixture/v1.0.0/src/other")->source->text);
                    }
                }
            }
            destroy_binding();
        }
    }
}
