/** @file
 * @brief Check all native own-formal introductions independently of bindings or foreign residual metadata.
 */
#include "../common.c"
#include "semantic/static/native/admission.h"

/** Every native ordinary interface Type introduction is forbidden, including
 * an output-only placeholder; no standalone or Template formal syntax exists.
 */
static void it_rejects_every_native_own_formal_at_its_original_marker_and_introduction(void) {
    const char *cases[] = {
        "@<\nvalue\n.input type\n@C\n",
        "@>\nvalue\n.result type\n@C\n"
    };
    const char *names[] = {".input type", ".result type"};
    for (size_t index = 0; index < sizeof(cases) / sizeof(*cases); index++) {
        setup_static(cases[index], STATIC_SCOPE_SOURCE_TEXT);
        assert(static_inventory_collect(&inventory) == STATIC_OK && inventory.valid);
        StaticNativeAdmission result = {0};
        size_t before = allocation_count;
        assert(!static_native_admit(&inventory, &result));
        assert(allocation_count == before && inventory.valid && inventory.status == STATIC_OK);
        assert(diagnostic_valid(&result.error) && layout_text_is(result.error.primary.span, "@C"));
        assert(result.error.related == &result.parameter && result.error.related_count == 1);
        assert(layout_text_is(result.parameter.location.span, names[index]));
        assert(result.parameter.location.span.start == static_name_site(project.entry, names[index]).start);
        assert(strcmp(result.parameter.name, "/test/fixture/v1.0.0/src/main") == 0);
        destroy_static();
    }
}

/** Ordinary typed calls cannot legalize a generic native definition, even if
 * their argument could constrain its Type in a later inference stage.
 */
static void it_rejects_a_native_definition_even_when_every_source_use_binds_its_parameter(void) {
    setup_static("@<\nvalue&\n.type\n@C\n", "@!\nvalue\n/test/fixture/v1.0.0/src/main\nknown&\n/test/fixture/v1.0.0/src/other\n &\nvalue !\n < known&\n");
    assert(static_inventory_collect(&inventory) == STATIC_OK && inventory.valid);
    const StaticModule *other = static_module_find(&inventory, project_find(&project, "/test/fixture/v1.0.0/src/other"));
    assert(other != NULL && other->parameter_count == 0 && other->first_occurrence->origin == project.entry);
    assert(other->first_occurrence->expression->first == NULL);
    StaticNativeAdmission result = {0};
    size_t before = allocation_count;
    assert(!static_native_admit(&inventory, &result) && allocation_count == before);
    assert(layout_text_is(result.error.primary.span, "@C") && layout_text_is(result.parameter.location.span, ".type"));
    destroy_static();
}

/** Concrete fields and named exports create no native own parameters. Ordinary
 * Ama definitions may stay generic; no native definition is specialized.
 */
static void it_accepts_native_nonformal_metadata_and_does_not_restrict_ama_templates(void) {
    const char *main_text[] = {
        "@C\n",
        "@<\nvalue\n.type\n@!\n",
        "@<\nfield\n/test/fixture/v1.0.0/src/other\n .native type\n@C\n",
        "@<\ninput\n/test/fixture/v1.0.0/src/other\n@C\n"
    };
    const char *other_text[] = {STATIC_SCOPE_SOURCE_TEXT,
        "@<\nvalue\n.other type\n@!\n", "@...\n.native type\n/test/fixture/v1.0.0/src/main\n", "@C\n"};
    for (size_t index = 0; index < sizeof(main_text) / sizeof(*main_text); index++) {
        setup_static(main_text[index], other_text[index]);
        assert(static_inventory_collect(&inventory) == STATIC_OK && inventory.valid);
        const char *sentinel = "unchanged accepted admission output";
        StaticNativeAdmission result = {.error = {.message = sentinel}, .parameter = {.role = sentinel}};
        size_t before = allocation_count;
        assert(static_native_admit(&inventory, &result));
        assert(allocation_count == before && result.error.message == sentinel && result.parameter.role == sentinel);
        assert(inventory.valid && inventory.status == STATIC_OK);
        destroy_static();
    }
}

/** Original Type-line order fixes the selected prohibited parameter; definition
 * order is the loaded inventory order.
 */
static void it_selects_original_formal_order_and_the_first_prohibited_native_definition(void) {
    setup_static("@<\nfirst\n.implicit type\nsecond\n.second type\n@C\n", "@<\nvalue\n.other type\n@C\n");
    assert(static_inventory_collect(&inventory) == STATIC_OK && inventory.valid);
    const StaticModule *module = static_module_find(&inventory, project.entry);
    assert(layout_text_is(module->first->name, ".implicit type") && module->first->ordinal == 0);
    StaticNativeAdmission result = {0};
    assert(!static_native_admit(&inventory, &result));
    assert(result.error.primary.span.source == project.entry->source->text);
    assert(layout_text_is(result.parameter.location.span, ".implicit type"));
    destroy_static();
    setup_static("@<\nvalue\n.ama type\n@!\n", "@<\nvalue\n.native type\n@C\n");
    assert(static_inventory_collect(&inventory) == STATIC_OK && inventory.valid);
    assert(!static_native_admit(&inventory, &result));
    assert(result.error.primary.span.source == project_find(&project, "/test/fixture/v1.0.0/src/other")->source->text);
    assert(strcmp(result.parameter.name, "/test/fixture/v1.0.0/src/other") == 0);
    destroy_static();
}

/** Error records borrow Project/source only. Destroying lexical inventory
 * scratch cannot invalidate related storage or original source locations.
 */
static void it_keeps_native_admission_diagnostics_alive_after_inventory_scratch_destruction(void) {
    setup_static("@<\nunused\n.unused\n@C\n", STATIC_SCOPE_SOURCE_TEXT);
    assert(static_inventory_collect(&inventory) == STATIC_OK);
    StaticNativeAdmission result = {0};
    assert(!static_native_admit(&inventory, &result));
    static_inventory_destroy(&inventory);
    assert(diagnostic_valid(&result.error) && result.error.related == &result.parameter);
    assert(layout_text_is(result.error.primary.span, "@C"));
    assert(layout_text_is(result.parameter.location.span, ".unused"));
    assert(strcmp(result.parameter.name, "/test/fixture/v1.0.0/src/main") == 0);
    destroy_static();
}

/** Execute each definition-policy and diagnostic-lifetime partition. */
static void run(void) {
    it_rejects_every_native_own_formal_at_its_original_marker_and_introduction();
    it_rejects_a_native_definition_even_when_every_source_use_binds_its_parameter();
    it_accepts_native_nonformal_metadata_and_does_not_restrict_ama_templates();
    it_selects_original_formal_order_and_the_first_prohibited_native_definition();
    it_keeps_native_admission_diagnostics_alive_after_inventory_scratch_destruction();
}

/** Return success after explicit assertions and counted teardown. */
int main(void) { run(); return 0; }
