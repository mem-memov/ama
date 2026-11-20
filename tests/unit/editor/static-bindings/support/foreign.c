/** @file
 * @brief Distinguish shared label spellings from actual formal origins in higher-order static compositions.
 * @see [T192 contract](../../../../../docs/implementation/static-editor-bindings.md)
 */

/** One source application accepts two actual constructors with equal named
 * signatures, but each owns its own original slot declaration. */
static void it_retains_multiple_actual_foreign_formal_origins_without_choosing_one(void) {
    project_fixture("@!\nfirst\n/test/fixture/v1.0.0/src/factory\n .first\nsecond\n/test/fixture/v1.0.0/src/factory\n .second\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/left", "@!\n");
    const char *box = "@<\nvalue\n.slot\n@>\nresult\n.slot\n value\n@!\nresult\n value\n";
    file("generated/project/packages/test/fixture/v1.0.0/src/box", box);
    file("generated/project/packages/test/fixture/v1.0.0/src/other box", box);
    file("generated/project/packages/test/fixture/v1.0.0/src/apply", "@...\n.out\n.family\n .slot < .type\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/factory", "@...\n/test/fixture/v1.0.0/src/apply\n .family < /test/fixture/v1.0.0/src/box\n .type < /test/fixture/v1.0.0/src/left\n .out > .first\n/test/fixture/v1.0.0/src/apply\n .family < /test/fixture/v1.0.0/src/other box\n .type < /test/fixture/v1.0.0/src/left\n .out > .second\n");
    setup_editor();
    assert(analyze(&analysis) && analysis.indexed);
    const ProjectModule *apply = project_find(&analysis.frontend.project,"/test/fixture/v1.0.0/src/apply");
    const SourceText *source = apply->source->text;
    const EditorOccurrence *targets;
    assert(editor_binding_targets(&analysis,"/test/fixture/v1.0.0/src/apply",point(source,".slot <"),&targets));
    size_t count = 0;
    bool first = false, second = false;
    for (const EditorOccurrence *target = targets; target; target = target->alternatives) {
        assert(target->static_symbol && text_is(target->static_symbol->name,".slot"));
        const ProjectModule *owner = target->static_symbol->owner->definition;
        first |= owner == project_find(&analysis.frontend.project,"/test/fixture/v1.0.0/src/box");
        second |= owner == project_find(&analysis.frontend.project,"/test/fixture/v1.0.0/src/other box");
        count++;
    }
    assert(first && second && count == 2);
    EditorDefinition sentinel = {.module=apply,.definition={source,1,2}};
    assert(!editor_definition(&analysis,"/test/fixture/v1.0.0/src/apply",point(source,".slot <"),&sentinel));
    assert(sentinel.module == apply && sentinel.definition.source == source && sentinel.definition.start == 1 && sentinel.definition.end == 2);
    EditorDefinition family;
    assert(editor_definition(&analysis,"/test/fixture/v1.0.0/src/apply",point(source,".family"),&family));
    assert(family.static_symbol && family.static_symbol->owner->definition == apply);
    destroy_editor();
}
