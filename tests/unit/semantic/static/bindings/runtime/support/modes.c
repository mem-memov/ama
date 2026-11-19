/** @file
 * @brief Compose resumes, retained/shared reads and canonical selected list methods with normalization.
 */

/** Resume and retained reads have no ordinary inputs; read markers retain their
 * own syntax kind and explicit receivers, rather than becoming entry calls.
 */
static void it_normalizes_resume_and_retained_reads_without_input_positions(void) {
    const char *bodies[] = {
        "worker %\n >\n > resumed list[]\n",
        "worker\n >> previous ref&\n >> previous list[]\n"
    };
    for (size_t retained = 0; retained < 2; retained++) {
        setup_worker(bodies[retained]);
        BindingInterface *view = runtime_interface();
        const AstNode *call = operation("worker");
        BindingApplication *application = NULL;
        assert(binding_normalize_runtime(&binding_plan, view, call, &application));
        assert(application->input_count == 0 && application->output_count == 2);
        assert(call->as.call.mode == (retained != 0 ? AST_RETAINED_READ : AST_RESUME_CALL));
        assert(application->first->omitted == (retained == 0));
        assert(application->first->syntax.runtime_binding->kind == (retained != 0 ? AST_SAVED_OUTPUT : AST_OUTPUT));
        assert(application->last->slot->original_ordinal == 1);
        destroy_binding();
    }
}

/** A concrete shared reference supplies a real constant-read output surface;
 * a module's ordinary result surface cannot silently stand in for it.
 */
static void it_uses_the_shared_constant_surface_for_constant_reads(void) {
    setup_binding("@=\nshared&\n.other\n@!\n.main\n >> received&\n", STATIC_SCOPE_SOURCE_TEXT);
    const StaticModule *module = binding_module(project.entry);
    BindingInterface *view = NULL;
    assert(binding_source_interface(&binding_plan, module, BINDING_CONSTANT_SURFACE, NULL, 0, &view));
    const AstNode *call = project.entry->syntax.body.first;
    assert(call->kind == AST_CALL && call->as.call.mode == AST_CONSTANT_READ);
    BindingApplication *application = NULL;
    assert(binding_normalize_runtime(&binding_plan, view, call, &application));
    assert(application->input_count == 0 && application->output_count == 1 && !application->pending);
    assert(layout_text_is(application->first->slot->name.site, "shared"));
    BindingInterface *wrong = NULL;
    assert(binding_source_interface(&binding_plan, module, BINDING_MODULE_SURFACE, NULL, 0, &wrong));
    BindingApplication *untouched = application;
    assert(!binding_normalize_runtime(&binding_plan, wrong, call, &untouched));
    assert(untouched == application && application->next == NULL && binding_plan.status == BINDING_INVALID);
    destroy_binding();
}

/** Parent list service actions are not ordinary arguments. The selected append
 * independently normalizes named reference fields from canonical descriptors.
 */
static void it_normalizes_selected_intrinsic_methods_without_parent_argument_positions(void) {
    ModuleRoot roots[] = {{STATIC_SCOPE_ROOT_DIRECTORY, NULL, 0, MODULE_ROOT_PROJECT},
        {"../../../../../../stdlib/modules", NULL, 0, MODULE_ROOT_STANDARD}};
    setup_runtime_roots("items[]\n...type\nitems[]\n append !\n  source& < input&\n  element& > appended&\n", roots, 2);
    assert(project_find(&project, "..integer") != NULL);
    const AstNode *parent = operation("items");
    assert(parent->as.call.first == NULL && parent->as.call.input_count == 0 && parent->as.call.output_count == 0);
    const AstOutputAction *action = parent->as.call.first_output;
    assert(action != NULL && action->kind == AST_SELECTED_ACTION && action->next == NULL);
    const AstNode *method = action->operation;
    assert(method->as.call.enclosing_operation == parent && layout_text_is(method->as.call.target->name, "append"));
    BindingInterface *view = NULL;
    assert(binding_list_interface(&binding_plan, SEMANTIC_LIST_APPEND, &view));
    BindingApplication *application = NULL;
    assert(binding_normalize_runtime(&binding_plan, view, method, &application));
    assert(application->input_count == 1 && application->output_count == 1 && application->mode == BINDING_MODE_NAMED);
    assert(application->first->slot->intrinsic_field == semantic_list_position(SEMANTIC_LIST_APPEND, false, 0));
    assert(application->last->slot->intrinsic_field == semantic_list_position(SEMANTIC_LIST_APPEND, true, 0));
    assert(parent->as.call.first == NULL && parent->as.call.output_count == 0);
    destroy_binding();
}
