/** @file
 * @brief Check actual frontend root selection and independently owned explicit delivery identity.
 * @see [T245 contract](../../../../docs/implementation/build-launcher-contracts.md)
 */
#include "../../compiler/common.c"
#include "delivery/selection/selection.h"
static SourceArena delivery;
/** Prepare actual source/native input files; check-only stages must never compile them. */
static void prepare(const char *body) {
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/main", body);
    file("generated/project/packages/test/fixture/v1.0.0/src/main.h", "#error Selection must not invoke native compilation\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/main.c", "#error Selection must not execute native code\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/main.abi", "+ initialize\n- destroy\n");
    setup_frontend(); source_arena_init(&delivery, sources.allocator);
}
/** Expire borrowed delivery views before destroying the frontend/source owners. */
static void finish(void) { source_arena_destroy(&delivery); destroy_frontend(); }
/** Both mode values carry the actual checked ordinary/native root without execution. */
static void it_owns_explicit_component_bytes_for_both_validated_root_kinds_and_modes(void) {
    const char *roots[] = {"@!\n", "@C\n"};
    for (size_t kind = 0; kind < 2; kind++) {
        prepare(roots[kind]); bool checked = check_project(); assert(checked);
        for (unsigned mode = 0; mode < 2; mode++) {
            char component[] = "компонент / name  ";
            DeliveryRequest request = {.mode = (DeliveryMode)mode, .component = component};
            const DeliverySelection *selection = NULL; Diagnostic error = {0};
            bool accepted = delivery_selection_prepare(&delivery, &frontend, &request, &selection, &error);
            assert(accepted && selection && selection->frontend == &frontend && selection->root == frontend.plan.entry);
            assert(selection->mode == (DeliveryMode)mode && selection->component != component && !strcmp(selection->component, component));
            component[0] = 'x';
            assert(!strcmp(selection->component, "компонент / name  ") && selection->root->type->executable);
        }
        finish();
    }
}
/** Invalid component/mode inputs allocate nothing and never publish a partial selection. */
static void it_refuses_invalid_mode_and_component_without_touching_output_or_frontend(void) {
    prepare("@!\n"); bool checked = check_project(); assert(checked);
    const char *invalid[] = {NULL, "", "\xc0\xaf", "\xed\xa0\x80", "\xf4\x90\x80\x80"};
    DeliverySelection sentinel = {0}; const DeliverySelection *output = &sentinel; Diagnostic error = {0};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]); index++) {
        DeliveryRequest request = {.mode = DELIVERY_EXECUTABLE, .component = invalid[index]};
        size_t before = allocation_count;
        bool accepted = delivery_selection_prepare(&delivery, &frontend, &request, &output, &error);
        assert(!accepted && output == &sentinel && allocation_count == before && error.code == DIAGNOSTIC_INVALID_PATH && frontend.valid);
    }
    DeliveryRequest request = {.mode = (DeliveryMode)999, .component = "component"};
    bool accepted = delivery_selection_prepare(&delivery, &frontend, &request, &output, &error);
    assert(!accepted && output == &sentinel && error.code == DIAGNOSTIC_INVALID_PATH);
    finish();
}
/** Unchecked, open and static frontends cannot become executable delivery evidence. */
static void it_refuses_unchecked_open_and_static_roots_before_allocating_metadata(void) {
    const char *roots[] = {"@!\n", "@...\n", "@<\ninput\n.type\n@!\n"};
    for (size_t index = 0; index < 3; index++) {
        prepare(roots[index]);
        if (index) { bool checked = check_project(); assert(!checked); }
        DeliverySelection sentinel = {0}; const DeliverySelection *output = &sentinel; Diagnostic error = {0};
        DeliveryRequest request = {.mode = DELIVERY_EXECUTABLE, .component = "component"};
        size_t before = allocation_count;
        bool accepted = delivery_selection_prepare(&delivery, &frontend, &request, &output, &error);
        assert(!accepted && output == &sentinel && allocation_count == before && error.code == DIAGNOSTIC_TYPE_MISMATCH);
        finish();
    }
}
/** Each owned preparation allocation can refuse and the same live frontend can retry. */
static void it_unwinds_each_selection_allocation_refusal_without_poisoning_the_root(void) {
    size_t measured = 0;
    for (size_t failure = 0; failure == 0 || failure <= measured; failure++) {
        prepare("@!\n"); bool checked = check_project(); assert(checked);
        DeliveryRequest request = {.mode = DELIVERY_SOURCE_INCLUSION, .component = "component"};
        DeliverySelection sentinel = {0}; const DeliverySelection *output = &sentinel; Diagnostic error = {0};
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        bool accepted = delivery_selection_prepare(&delivery, &frontend, &request, &output, &error);
        if (!failure) { assert(accepted); measured = allocation_count - before; assert(measured == 2); }
        else {
            assert(!accepted && output == &sentinel && error.code == DIAGNOSTIC_OUT_OF_MEMORY && frontend.valid);
            failure_at = 0;
            accepted = delivery_selection_prepare(&delivery, &frontend, &request, &output, &error);
            assert(accepted && output->root == frontend.plan.entry);
        }
        finish();
    }
}
/** Run complete owned selection and refusal partitions explicitly. */
void run(void) {
    it_owns_explicit_component_bytes_for_both_validated_root_kinds_and_modes();
    it_refuses_invalid_mode_and_component_without_touching_output_or_frontend();
    it_refuses_unchecked_open_and_static_roots_before_allocating_metadata();
    it_unwinds_each_selection_allocation_refusal_without_poisoning_the_root();
}
/** Exit only after every counted frontend/delivery allocation has retired. */
int main(void) { run(); return 0; }
