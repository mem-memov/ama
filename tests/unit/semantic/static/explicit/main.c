/** @file
 * @brief Check explicit static names, categories, source provenance and independent interface order.
 */
#include "../common.c"
#include <stdint.h>

/** Runtime and static entries interleave without merging their positions;
 * nested signature names remain foreign to the module's local scope.
 */
static const char *explicit_source = "@<\n...type\nruntime&\n...type\n"
    "...factory\n ...foreign key\n ...nested family\n  ...foreign element\n"
    "@>\nruntime result\n...type\n...first type\n...type\n"
    "...second family\n...factory\n@!\n";

/** Only explicit binders are collected now; actual signature construction and
 * result expression categories remain separate, unclaimed later phases.
 */
static void it_collects_whole_module_explicit_binders_without_importing_runtime_or_signature_names(void) {
    setup_static(explicit_source, STATIC_SCOPE_SOURCE_TEXT);
    StaticModule *module = static_module_add(&inventory, project.entry);
    assert(module != NULL && static_collect_explicit(&inventory, module));
    assert(module->parameter_count == 2 && module->result_count == 2);
    const char *names[] = {"...type", "...factory", "...first type", "...second family"};
    StaticCategory categories[] = {STATIC_CATEGORY_TYPE, STATIC_CATEGORY_TEMPLATE,
        STATIC_CATEGORY_DEFERRED, STATIC_CATEGORY_DEFERRED};
    StaticSymbol *symbol = module->first;
    for (size_t index = 0; index < 4; index++) {
        assert(symbol != NULL && layout_text_is(symbol->name, names[index]));
        assert(symbol->category == categories[index] && symbol->declaration != NULL);
        assert(symbol->role == (index < 2 ? STATIC_PARAMETER : STATIC_RESULT));
        assert(symbol->ordinal == (index < 2 ? SIZE_MAX : index - 2));
        assert(static_symbol_find(module, symbol->name, 0) == symbol);
        assert(symbol->site.source == project.entry->source->text && symbol->site.start == symbol->name.start);
        symbol = symbol->next;
    }
    assert(symbol == NULL && module->last->ordinal == 1 && !inventory.valid);
    assert(static_symbol_find(module, static_name_site(project.entry, "...foreign key"), 0) == NULL);
    assert(static_symbol_find(module, static_name_site(project.entry, "...foreign element"), 0) == NULL);
    assert(module->first->next->declaration->as.static_formal->first != NULL);
    assert(module->last->declaration->as.static_result.expression->kind == AST_TYPE_PLACEHOLDER);
    destroy_static();
}

/** Duplicate input/result declarations fail at the later exact header with the
 * prior declaration retained, independently of their categories or definition.
 */
static void it_rejects_explicit_formal_and_result_collisions_with_original_headers(void) {
    const char *cases[] = {
        "@<\n...same\n...same\n ...foreign\n@!\n",
        "@<\n...same\n@>\n...same\n...same\n@!\n",
        "@>\n...same\n...value type\n...same\n...value type\n@!\n"
    };
    for (size_t index = 0; index < sizeof(cases) / sizeof(*cases); index++) {
        setup_static(cases[index], STATIC_SCOPE_SOURCE_TEXT);
        StaticModule *module = static_module_add(&inventory, project.entry);
        assert(module != NULL && !static_collect_explicit(&inventory, module));
        assert(inventory.status == STATIC_INVALID && diagnostic_valid(&inventory.error));
        assert(inventory.error.related_count == 1 && module->first == module->last);
        assert(layout_text_is(inventory.error.primary.span, "...same"));
        assert(layout_text_is(inventory.error.related[0].location.span, "...same"));
        assert(inventory.error.primary.span.start > inventory.error.related[0].location.span.start);
        destroy_static();
    }
}

/** Native admission is a later consumer: its own unused explicit Type is still
 * inventoried, so fully bound actual uses cannot conceal a free definition.
 */
static void it_retains_unused_native_formals_for_the_later_native_admission_boundary(void) {
    setup_static("@<\n...unused type\n@C\n", STATIC_SCOPE_SOURCE_TEXT);
    StaticModule *module = static_module_add(&inventory, project.entry);
    assert(module != NULL && static_collect_explicit(&inventory, module));
    assert(module->definition->syntax.sections.kind == MODULE_NATIVE);
    assert(module->parameter_count == 1 && module->result_count == 0);
    assert(module->first == module->last && module->first->category == STATIC_CATEGORY_TYPE);
    assert(layout_text_is(module->first->name, "...unused type") && !inventory.valid);
    destroy_static();
}

/** Every real allocation in this stage is refused once; completed prefixes
 * remain owned and never masquerade as a valid whole inventory.
 */
static void it_unwinds_every_explicit_static_inventory_allocation_failure(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        setup_static(explicit_source, STATIC_SCOPE_SOURCE_TEXT);
        StaticModule *module = static_module_add(&inventory, project.entry);
        assert(module != NULL);
        size_t before = allocation_count;
        if (failure != 0) { failure_at = before + failure; }
        bool collected = static_collect_explicit(&inventory, module);
        if (failure == 0) { assert(collected); total = allocation_count - before; assert(total != 0); }
        else {
            assert(!collected && inventory.status == STATIC_OUT_OF_MEMORY && !inventory.valid);
            assert(diagnostic_valid(&inventory.error));
        }
        destroy_static();
    }
}

/** Run every explicit collector contract independently of implicit classification. */
void run(void) {
    it_collects_whole_module_explicit_binders_without_importing_runtime_or_signature_names();
    it_rejects_explicit_formal_and_result_collisions_with_original_headers();
    it_retains_unused_native_formals_for_the_later_native_admission_boundary();
    it_unwinds_every_explicit_static_inventory_allocation_failure();
}

/** Return success only after source-backed assertions and complete owner teardown. */
int main(void) { run(); return EXIT_SUCCESS; }
