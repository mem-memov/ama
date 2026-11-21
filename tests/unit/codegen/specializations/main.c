/** @file
 * @brief Execute distinct and equal captured namespaces and actual concrete instance calls through one C assembly.
 * @see [T188 contract](../../../../docs/implementation/specialized-execution.md)
 */
#include "../execution/common.c"
#include "support/fixture.c"
#include "support/model.c"
#include "support/continuations.c"
#include "support/composition.c"
#include "support/partial.c"
#include "support/busy.c"

/** Two specializations share source syntax, but require distinct C layouts and concrete result routes. */
static void it_executes_distinct_captured_constants_and_concrete_module_calls(void) {
    prepare_captured_definitions();
    native_fixture_completion = inspect_captured_model;
    execute_project(captured_program, "17:1:0\n17:1:0\n17:2:0\n", true);
    native_fixture_completion = NULL;
}

/** Native startup failure must retain the original diagnostic and prevent entering the root body. */
static void it_reports_concrete_shared_initialization_failure_without_entering_the_root(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/factory", "@...\n.numbers\n/test/fixture/v1.0.0/src/container\n .slot < /test/fixture/v1.0.0/src/value\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/container", "@=\nfirst\n.slot\n |invalid\n@<\ninput\n.slot\n@!\n");
    execute_project("@!\n/test/fixture/v1.0.0/src/factory\n .numbers\n first >> value\nprobe\n/test/fixture/v1.0.0/src/worker\nprobe !\n", "", false);
    expect_diagnostic("shared construction");
}

/** Select real compiled compositions explicitly, preserving failed generated evidence. */
static void run(void) {
    it_executes_distinct_captured_constants_and_concrete_module_calls();
    it_reports_concrete_shared_initialization_failure_without_entering_the_root();
    it_preserves_independent_captured_instances_references_and_explicit_continuation_copies();
    it_executes_nested_static_composition_through_an_explicitly_closed_static_assembly();
    it_infers_static_producer_inputs_from_exact_structural_call_constraints();
    it_executes_a_partial_family_completed_by_another_static_producer();
    it_refuses_busy_captured_self_references_without_merging_equal_key_instance_state();
    it_unwinds_every_captured_project_generation_allocation_before_publication();
}

/** Return success only after actual compiled execution and balanced owner teardown. */
int main(void) { run(); return 0; }
