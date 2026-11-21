/** @file
 * @brief Check default-only inputs, exact literal capabilities and borrowed-owner refusal.
 * @see [T245 contract](../../../../docs/implementation/build-launcher-contracts.md)
 */
#include "../../compiler/common.c"
#include "delivery/inputs/inputs.h"
static SourceArena delivery;
static const DeliverySelection *selection;
/** Own a real checked root and a native literal Type lacking clone/condition hooks. */
static void prepare(const char *body) {
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/main", body);
    file("generated/project/packages/test/fixture/v1.0.0/src/value", "@C\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/value.h", "#error Metadata capture must not compile C\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/value.c", "#error Metadata capture must not execute native code\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/value.abi", "+ initialize\n- destroy\n| literal\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/state", "@!\n");
    setup_frontend(); source_arena_init(&delivery, sources.allocator);
    bool checked = check_project(); assert(checked);
    DeliveryRequest request = {.component = "component", .mode = DELIVERY_EXECUTABLE}; Diagnostic error = {0};
    bool prepared = delivery_selection_prepare(&delivery, &frontend, &request, &selection, &error); assert(prepared);
}
/** Destroy borrowed descriptor views before their real frontend/source owners. */
static void finish(void) { source_arena_destroy(&delivery); destroy_frontend(); }
/** Match actual source declaration bytes independently of metadata indexes. */
static bool named(const SemanticSymbol *field, const char *name) {
    return field->name.end - field->name.start == strlen(name) &&
        !memcmp(source_bytes(field->name.source) + field->name.start, name, strlen(name));
}
/** Default-only complex/reference/list fields remain admitted without external codecs. */
static void it_preserves_exact_inputs_and_supports_literal_only_native_types(void) {
    prepare("@<\ninput name\n/test/fixture/v1.0.0/src/value\n"
        "link&\n/test/fixture/v1.0.0/src/value\nitems[]\n/test/fixture/v1.0.0/src/value\n"
        "state\n/test/fixture/v1.0.0/src/state\n@>\noutput\n/test/fixture/v1.0.0/src/value\n@!\n");
    const DeliveryInputs *inputs = NULL; Diagnostic error = {0};
    bool prepared = delivery_inputs_prepare(&delivery, selection, &inputs, &error);
    assert(prepared && inputs->count == 4 && inputs->selection == selection);
    const char *names[] = {"input name", "link", "items", "state"};
    for (size_t index = 0; index < 4; index++) {
        assert(inputs->entries[index].ordinal == index && named(inputs->entries[index].field, names[index]));
        assert(inputs->entries[index].text_supported == (index == 0));
    }
    const SemanticType *literal = inputs->entries[0].field->type;
    assert(!literal->cloneable && !literal->abi.bindings[ABI_CONDITION].present);
    assert(inputs->entries[1].field->reference && inputs->entries[2].field->list);
    finish();
}
/** No input is invented for empty or result-only interfaces. */
static void it_retains_empty_tables_without_requiring_result_transports(void) {
    prepare("@>\nresult\n/test/fixture/v1.0.0/src/value\n@!\n");
    const DeliveryInputs *inputs = NULL; Diagnostic error = {0};
    bool prepared = delivery_inputs_prepare(&delivery, selection, &inputs, &error);
    assert(prepared && !inputs->count && !inputs->entries); finish();
}
/** A live invalidated frontend refuses before old descriptor fields are inspected. */
static void it_refuses_expired_frontend_state_without_using_borrowed_ir(void) {
    prepare("@!\n");
    compiler_frontend_destroy(&frontend);
    DeliveryInputs sentinel = {0}; const DeliveryInputs *output = &sentinel; Diagnostic error = {0};
    size_t before = allocation_count;
    bool prepared = delivery_inputs_prepare(&delivery, selection, &output, &error);
    assert(!prepared && output == &sentinel && error.code == DIAGNOSTIC_CHANGED_INPUT && allocation_count == before);
    source_arena_destroy(&delivery); destroy_test();
}
/** Failed table allocation never changes its selection and can be retried. */
static void it_preserves_the_selection_and_output_on_table_allocation_refusal(void) {
    prepare("@<\nvalue\n/test/fixture/v1.0.0/src/value\n@!\n");
    DeliveryInputs sentinel = {0}; const DeliveryInputs *output = &sentinel; Diagnostic error = {0};
    failure_at = allocation_count + 1;
    bool prepared = delivery_inputs_prepare(&delivery, selection, &output, &error);
    assert(!prepared && output == &sentinel && error.code == DIAGNOSTIC_OUT_OF_MEMORY);
    failure_at = 0;
    prepared = delivery_inputs_prepare(&delivery, selection, &output, &error);
    assert(prepared && output->count == 1 && output->entries[0].text_supported && frontend.valid); finish();
}
/** Run exact capability, omission, lifetime and memory-failure cases explicitly. */
void run(void) {
    it_preserves_exact_inputs_and_supports_literal_only_native_types();
    it_retains_empty_tables_without_requiring_result_transports();
    it_refuses_expired_frontend_state_without_using_borrowed_ir();
    it_preserves_the_selection_and_output_on_table_allocation_refusal();
}
/** Exit only after all frontend/delivery owners have retired. */
int main(void) { run(); return 0; }
