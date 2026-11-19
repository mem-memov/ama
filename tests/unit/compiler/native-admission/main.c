/** @file
 * @brief Prove native own-formal errors precede capture, ABI preparation and ordinary semantic completion.
 */
#include "../common.c"
#include "compiler/internal.h"
#include "lowering/control/internal.h"

/** Prepare exact source plus deliberately invalid native C/ABI; early own-formal
 * refusal must not invoke native compilation or reach ABI contract checking.
 */
static void prepare_native(const char *source) {
    directory("generated/project");
    file("generated/project/main", source);
    file("generated/project/main.h", "#error Admission checking must not compile native C\n");
    file("generated/project/main.c", "#error Admission checking must not compile native C\n");
    file("generated/project/main.abi", "not an ABI contract\n");
}

/** Original source marker and parameter context must outlive all inner helper
 * calls and remain owned by the public frontend until diagnostic rendering.
 */
static void rejected(const char *parameter, const char *module_name) {
    const ProjectModule *module = project_find(&frontend.project, module_name);
    assert(module != NULL && !frontend.valid && frontend.static_initialized && frontend.static_inventory.valid);
    assert(!frontend.semantic_initialized && !frontend.plan_initialized && !frontend.inputs.attempted);
    assert(frontend.error.related == &frontend.native_admission.parameter && frontend.error.related_count == 1);
    assert(diagnostic_valid(&frontend.error) && layout_text_is(frontend.error.primary.span, "@C"));
    assert(frontend.error.primary.span.source == module->source->text);
    assert(layout_text_is(frontend.error.related[0].location.span, parameter));
    assert(strcmp(frontend.error.related[0].name, module_name) == 0);
    assert(frontend.proposed_sources.first == NULL);
}

#include "support/paths.c"
#include "support/failures.c"
#include "support/lowering.c"
#include "support/concrete.c"

/** Run actual frontend compositions and ownership prefixes explicitly. */
static void run(void) {
    it_rejects_native_own_formals_before_check_capture_or_authoring_abi_proposals();
    it_checks_native_admission_on_preloaded_catalog_and_explicit_required_paths();
    it_preserves_existing_concrete_native_check_and_authoring_paths();
    it_unwinds_every_new_frontend_static_admission_allocation_failure();
    it_refuses_unresolved_static_nodes_at_the_executable_lowering_boundary();
    it_refuses_static_graphs_and_selected_types_before_concrete_inventory_publication();
}

/** Return success after all assertions and counted teardown. */
int main(void) { run(); return 0; }
