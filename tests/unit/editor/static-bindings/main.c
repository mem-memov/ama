/** @file
 * @brief Require original static path/export/parameter definitions without conflating concrete template captures.
 * @see [T192 contract](../../../../docs/implementation/static-editor-bindings.md)
 */
#include "../common.c"

/** One original ordinary definition may be used under one or two closed keys;
 * the static producer and returned ordinary Type remain separate definitions. */
static void static_fixture(bool multiple) {
    project_fixture(multiple
        ? "@!\nfirst\n/test/fixture/v1.0.0/src/factory\n .left\nsecond\n/test/fixture/v1.0.0/src/factory\n .right\n"
        : "@!\nfirst\n/test/fixture/v1.0.0/src/factory\n .left\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/left", "@!\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/right", "@!\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/box", "@<\nvalue\n.slot\n@>\nresult\n.slot\n value\n@!\nresult\n value\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/factory", multiple
        ? "@...\n.left\n/test/fixture/v1.0.0/src/box\n .slot < /test/fixture/v1.0.0/src/left\n.right\n/test/fixture/v1.0.0/src/box\n .slot < /test/fixture/v1.0.0/src/right\n"
        : "@...\n.left\n/test/fixture/v1.0.0/src/box\n .slot < /test/fixture/v1.0.0/src/left\n");
}

#include "support/completions.c"
#include "support/operations.c"
#include "support/foreign.c"
#include "support/names.c"

/** A producer path navigates to that static module, and its selector/foreign
 * label navigate to original graph/formal declarations, never virtual files. */
static void it_navigates_static_paths_exports_and_parameter_labels_to_original_definitions(void) {
    static_fixture(false);
    setup_editor();
    assert(analyze(&analysis) && analysis.indexed && analysis.state == EDITOR_COMPLETE);
    const SourceText *main = analysis.selected->text;
    const SourceText *factory = project_find(&analysis.frontend.project, "/test/fixture/v1.0.0/src/factory")->source->text;
    const SourceText *box = project_find(&analysis.frontend.project, "/test/fixture/v1.0.0/src/box")->source->text;
    EditorDefinition definition;
    assert(editor_definition(&analysis, "/test/fixture/v1.0.0/src/main", point(main, "/test/fixture/v1.0.0/src/factory"), &definition));
    assert(definition.definition.source == factory);
    assert(editor_definition(&analysis, "/test/fixture/v1.0.0/src/main", point(main, ".left"), &definition));
    assert(definition.definition.source == factory && text_is(definition.definition, ".left"));
    assert(editor_definition(&analysis, "/test/fixture/v1.0.0/src/factory", point(factory, ".slot <"), &definition));
    assert(definition.definition.source == box && text_is(definition.definition, ".slot"));
    destroy_editor();
}

/** Distinct captured runtime Types share a source declaration, not one runtime
 * symbol pointer; that must not make a valid source snapshot unindexable. */
static void it_retains_original_declarations_across_multiple_concrete_captures(void) {
    static_fixture(true);
    setup_editor();
    assert(analyze(&analysis) && analysis.indexed && analysis.state == EDITOR_COMPLETE);
    const SourceText *box = project_find(&analysis.frontend.project, "/test/fixture/v1.0.0/src/box")->source->text;
    EditorDefinition definition;
    assert(editor_definition(&analysis, "/test/fixture/v1.0.0/src/box", point(box, "value"), &definition));
    assert(definition.definition.source == box && text_is(definition.definition, "value"));
    assert(definition.original && !definition.symbol && !definition.type);
    const EditorOccurrence *targets;
    assert(editor_binding_targets(&analysis, "/test/fixture/v1.0.0/src/box", point(box, "value"), &targets));
    size_t concrete = 0;
    const SemanticType *first_owner = NULL;
    for (const EditorOccurrence *target = targets; target; target = target->alternatives) {
        assert(target->original == definition.original);
        if (!target->symbol) { continue; }
        concrete++;
        if (!first_owner) { first_owner = target->symbol->owner; }
        else { assert(first_owner != target->symbol->owner); }
    }
    assert(concrete == 2);
    const EditorOccurrence sentinel = {0};
    targets = &sentinel;
    assert(!editor_binding_targets(NULL, "/test/fixture/v1.0.0/src/box", 0, &targets) && targets == &sentinel);
    assert(!editor_binding_targets(&analysis, NULL, 0, &targets) && targets == &sentinel);
    assert(!editor_binding_targets(&analysis, "/test/fixture/v1.0.0/src/box", source_length(box), &targets) && targets == &sentinel);
    destroy_editor();
}

/** Run original-source navigation independently from later rename transactions. */
static void run(void) {
    it_navigates_static_paths_exports_and_parameter_labels_to_original_definitions();
    it_retains_original_declarations_across_multiple_concrete_captures();
    it_enumerates_source_interfaces_without_selecting_the_first_capture();
    it_enumerates_whole_graph_static_results_without_runtime_slots();
    it_refuses_ambiguous_concrete_operation_views_without_losing_source_definitions();
    it_retains_multiple_actual_foreign_formal_origins_without_choosing_one();
    it_preserves_atomic_unicode_graph_names_and_original_receipt_definitions();
}

/** Return success only after checked editor views expire and counted teardown. */
int main(void) { run(); return 0; }
