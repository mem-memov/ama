/** @file
 * @brief Check ordinary Type introductions and separation from graph public binders.
 */
#include "../common.c"
#include <stdint.h>
#include "semantic/static/classify/sites.h"

/** Prepare immutable sorted occurrences after public pair collection. */
static StaticModule *prepare(void) {
    StaticModule *module = static_module_add(&inventory, project.entry);
    assert(module != NULL && static_collect_explicit(&inventory, module));
    assert(static_collect_occurrences(&inventory, module));
    static_sort_occurrences(module);
    return module;
}

#include "support/graph.c"

/** All ordinary input/output placeholders introduce Type once, in first written
 * type-line order. Constants and body uses can precede or follow that interface.
 */
static void it_classifies_ordinary_interface_types_once_in_original_order(void) {
    setup_static("@=\nshared\n...later\n@<\nfirst&\n...first\nsecond\n...later\n"
        "@>\nresult\n...result\nagain\n...first\n@!\nstate\n...result\n", STATIC_SCOPE_SOURCE_TEXT);
    StaticModule *module = prepare();
    assert(static_classify_names(&inventory, module));
    assert(module->parameter_count == 3 && module->result_count == 0);
    const char *names[] = {"...first", "...later", "...result"};
    for (size_t ordinal = 0; ordinal < 3; ordinal++) {
        const StaticSymbol *symbol = static_symbol_at(module, STATIC_PARAMETER, ordinal);
        assert(symbol != NULL && layout_text_is(symbol->name, names[ordinal]));
        assert(symbol->category == STATIC_CATEGORY_TYPE && symbol->receiver == NULL);
        assert(static_symbol_find(module, static_name_site(project.entry, names[ordinal])) == symbol);
    }
    destroy_static();
}

/** Neither ordinary noninterface uses nor already collected public pairs create
 * free ordinary parameters; public graph binders are excluded from free inputs.
 */
static void it_excludes_body_uses_and_public_graph_binders_from_ordinary_introductions(void) {
    const char *cases[] = {"@!\nstate\n...body only\n",
        "@...\n...known\n.other\n...forwarded\n...known\n"};
    for (size_t index = 0; index < 2; index++) {
        setup_static(cases[index], STATIC_SCOPE_SOURCE_TEXT);
        StaticModule *module = prepare();
        assert(static_classify_names(&inventory, module));
        assert(module->parameter_count == 0 && module->result_count == 2 * index);
        destroy_static();
    }
}

/** Every actual ordinary formal allocation is refused once; public collection
 * and occurrence ownership are prepared before the failure point.
 */
static void it_unwinds_every_ordinary_classification_allocation_failure(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        setup_static("@<\nfirst\n...first\nsecond\n...second\n@>\nresult\n...result\n@!\n",
            STATIC_SCOPE_SOURCE_TEXT);
        StaticModule *module = prepare();
        size_t before = allocation_count;
        if (failure != 0) { failure_at = before + failure; }
        bool success = static_classify_names(&inventory, module);
        if (failure == 0) { assert(success); total = allocation_count - before; assert(total == 3); }
        else { assert(!success && inventory.status == STATIC_OUT_OF_MEMORY && !inventory.valid); }
        destroy_static();
    }
}

/** Execute current ordinary source classification partitions explicitly. */
static void run(void) {
    it_collects_graph_inputs_from_nested_sources_after_all_public_names();
    it_keeps_equal_graph_input_spellings_in_independent_module_scopes();
    it_unwinds_every_graph_input_classification_allocation_failure();
    it_classifies_ordinary_interface_types_once_in_original_order();
    it_excludes_body_uses_and_public_graph_binders_from_ordinary_introductions();
    it_unwinds_every_ordinary_classification_allocation_failure();
}
/** Return success after assertions and counted teardown. */
int main(void) { run(); return 0; }
