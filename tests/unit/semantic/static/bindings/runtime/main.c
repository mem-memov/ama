/** @file
 * @brief Compose actual ordinary call syntax with runtime and intrinsic interface snapshots.
 */
#include "../runtime-common.c"

/** Supply the runtime surface with its independently fixed Type formal identities.
 * Type-value closure belongs to typed consumers, not request normalization.
 */
static BindingInterface *runtime_interface(void) {
    const StaticModule *module = binding_module(project_find(&project, ".other"));
    const StaticSymbol *captures[] = {
        static_symbol_find(module, static_name_site(module->definition, "...a")),
        static_symbol_find(module, static_name_site(module->definition, "...b"))};
    BindingInterface *view = NULL;
    assert(binding_source_interface(&binding_plan, module, BINDING_MODULE_SURFACE, captures, 2, &view));
    assert(view->sequences[BINDING_STATIC_INPUT].count == 0);
    return view;
}

/** Locate a top-level ordinary operation, leaving selected children with their
 * actual enclosing operation rather than flattening the action tree.
 */
static const AstNode *operation(const char *name) {
    for (const AstNode *node = project.entry->syntax.body.first; node != NULL; node = node->next) {
        if (node->kind == AST_CALL && node->as.call.target != NULL && layout_text_is(node->as.call.target->name, name)) { return node; }
    }
    assert(false);
    return NULL;
}

/** Explicit static field bindings determine worker's nominal producer without
 * becoming ordinary call positions or executing any input initialization.
 */
static const char *worker_declaration = "worker\n.other\n";

/** Concatenate a maintained declaration and operation body for real parsing. */
static void setup_worker(const char *body) {
    size_t size = strlen(worker_declaration) + strlen(body) + 1;
    char *text = malloc(size);
    assert(text != NULL);
    memcpy(text, worker_declaration, strlen(worker_declaration));
    memcpy(text + strlen(worker_declaration), body, strlen(body) + 1);
    setup_runtime_requests(text);
    free(text);
}

#include "support/ordinary.c"
#include "support/modes.c"
#include "support/errors.c"
#include "support/omissions.c"

/** Execute every ordinary/interface-mode partition explicitly. */
static void run(void) {
    it_keeps_runtime_omissions_and_named_storage_labels_in_independent_sequences();
    it_normalizes_resume_and_retained_reads_without_input_positions();
    it_uses_the_shared_constant_surface_for_constant_reads();
    it_normalizes_selected_intrinsic_methods_without_parent_argument_positions();
    it_rejects_runtime_duplicate_storage_arity_and_cloned_mode_order_errors();
    it_checks_every_runtime_skip_prefix_and_first_excess_position();
}

/** Return success only after assertions and counted teardown. */
int main(void) { run(); return 0; }
