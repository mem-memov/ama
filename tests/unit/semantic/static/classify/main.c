/** @file
 * @brief Exercise lexical parameter introductions, receiver precedence and original formal ordering.
 */
#include "../common.c"
#include <stdint.h>
#include "semantic/static/classify/sites.h"

/** Prepare immutable sorted occurrences and whole-module explicit binders. */
static StaticModule *prepare(void) {
    StaticModule *module = static_module_add(&inventory, project.entry);
    assert(module != NULL && static_collect_explicit(&inventory, module));
    assert(static_collect_occurrences(&inventory, module));
    static_sort_occurrences(module);
    return module;
}

/** Obtain a fixture's exact local identity after classification. */
static const StaticSymbol *named(const StaticModule *module, const char *name) {
    const StaticSymbol *symbol = static_symbol_find(module,
        static_name_site(project.entry, name), SIZE_MAX);
    assert(symbol != NULL);
    return symbol;
}

/** Output-only heads introduce Type, including skip actions; explicit controls
 * retain their own declaration position and receivers occupy no formal slots.
 */
static void it_classifies_output_only_heads_result_types_and_explicit_controls(void) {
    const char *cases[] = {
        "@<\nvalue&\n...type\n ...out > ...alias\n@!\n",
        "@<\nvalue&\n...type\n ... >\n@!\n",
        "@<\n...type\nvalue&\n...type\n ...out > ...alias\n@!\n",
        "@>\nvalue\n...type\n ... >\n@!\n",
        "@>\n...result\n...type\n ... >\n@!\n"
    };
    for (size_t index = 0; index < 5; index++) {
        setup_static(cases[index], static_scope_text);
        StaticModule *module = prepare();
        assert(static_classify_names(&inventory, module));
        const StaticSymbol *type = named(module, "...type");
        assert(type->category == STATIC_CATEGORY_TYPE && type->role == STATIC_PARAMETER);
        assert(type->ordinal == 0 && module->parameter_count == 1);
        if (index == 0 || index == 2) {
            const StaticSymbol *alias = named(module, "...alias");
            assert(alias->role == STATIC_ALIAS && alias->category == STATIC_CATEGORY_DEFERRED);
            assert(alias->receiver != NULL && alias->ordinal == SIZE_MAX);
        }
        if (index == 4) { assert(named(module, "...result")->role == STATIC_RESULT); }
        destroy_static();
    }
}

/** A received name followed by an interface use stays an alias. Reversing the
 * genuine introduction/receiver order retains a whole-module owner parameter.
 */
static void it_uses_source_order_for_aliases_without_turning_later_uses_into_formals(void) {
    const char *cases[] = {
        "@<\nsource\n.other\n ...out > ...received\nlater\n...received\n@!\n",
        "@<\nsource\n...received\nlater\n.other\n ...out > ...received\n@!\n"
    };
    for (size_t index = 0; index < 2; index++) {
        setup_static(cases[index], static_scope_text);
        StaticModule *module = prepare();
        assert(static_classify_names(&inventory, module));
        const StaticSymbol *symbol = named(module, "...received");
        assert(symbol->role == (index == 0 ? STATIC_ALIAS : STATIC_PARAMETER));
        assert(module->parameter_count == index);
        if (index == 0) {
            assert(static_symbol_find(module, symbol->name, symbol->site.start - 1) == NULL);
            assert(static_symbol_find(module, symbol->name, symbol->site.start) == symbol);
        } else { assert(static_symbol_find(module, symbol->name, 0) == symbol); }
        destroy_static();
    }
}

/** Collected Template patterns determine kind before checking bare forward
 * uses; foreign labels occupy no owner position. Explicit declarations retain
 * their own ordinal despite preceding references.
 */
static void it_collects_whole_interface_template_categories_and_original_introduction_ordinals(void) {
    setup_static("@<\nforward\n...family\n...type\npattern\n...family\n ...slot < ...type\n"
        "later\n...last\n@!\n", static_scope_text);
    StaticModule *module = prepare();
    assert(static_classify_names(&inventory, module));
    const StaticSymbol *family = named(module, "...family");
    assert(family->category == STATIC_CATEGORY_TEMPLATE && family->ordinal == 1);
    assert(named(module, "...type")->ordinal == 0 && named(module, "...last")->ordinal == 2);
    assert(module->parameter_count == 3 && family->site.start > family->name.start);
    assert(static_symbol_find(module, static_name_site(project.entry, "...slot"), SIZE_MAX) == NULL);
    destroy_static();
}

/** Neither runtime-body sources nor unknown result applications introduce a
 * free formal. Their eventual use diagnostics belong to the binding phase.
 * An unused receiver may still establish a static alias in a nested body.
 */
static void it_excludes_body_sources_and_result_only_applied_heads_from_introductions(void) {
    setup_static("@>\nvalue\n...unknown\n ...slot < ...type\n@!\n"
        "body\n...body only\n.other\n ...out > ...alias\n", static_scope_text);
    StaticModule *module = prepare();
    assert(static_classify_names(&inventory, module));
    assert(module->parameter_count == 1 && named(module, "...type")->category == STATIC_CATEGORY_TYPE);
    assert(static_symbol_find(module, static_name_site(project.entry, "...unknown"), SIZE_MAX) == NULL);
    assert(static_symbol_find(module, static_name_site(project.entry, "...body only"), SIZE_MAX) == NULL);
    assert(named(module, "...alias")->role == STATIC_ALIAS);
    destroy_static();
}

/** Refuse every actual implicit symbol allocation while retaining destructible
 * partial records. Explicit collection and occurrence sorting precede injection.
 */
static void it_unwinds_every_implicit_classification_allocation_failure(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        setup_static("@<\nfirst\n...family\n ...slot < ...type\nsecond\n.other\n ...out > ...alias\n@!\n", static_scope_text);
        StaticModule *module = prepare();
        size_t before = allocation_count;
        if (failure != 0) { failure_at = before + failure; }
        bool success = static_classify_names(&inventory, module);
        if (failure == 0) { assert(success); total = allocation_count - before; assert(total == 3); }
        else { assert(!success && inventory.status == STATIC_OUT_OF_MEMORY && !inventory.valid); }
        destroy_static();
    }
}

/** Execute every lexical-classification partition explicitly. */
static void run(void) {
    it_classifies_output_only_heads_result_types_and_explicit_controls();
    it_uses_source_order_for_aliases_without_turning_later_uses_into_formals();
    it_collects_whole_interface_template_categories_and_original_introduction_ordinals();
    it_excludes_body_sources_and_result_only_applied_heads_from_introductions();
    it_unwinds_every_implicit_classification_allocation_failure();
}

/** Return success after every assertion and counted teardown. */
int main(void) { run(); return 0; }
