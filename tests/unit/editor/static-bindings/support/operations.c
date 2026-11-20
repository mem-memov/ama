/** @file
 * @brief Refuse singular operation/binding metadata when a source body has multiple concrete contexts.
 * @see [T192 contract](../../../../../docs/implementation/static-editor-bindings.md)
 */

/** Source navigation remains available, but a raw typed operation pointer must
 * not silently choose one of the two actual captured owner Types. */
static void it_refuses_ambiguous_concrete_operation_views_without_losing_source_definitions(void) {
    static_fixture(true);
    file("generated/project/packages/test/fixture/v1.0.0/src/worker", "@<\nargument\n/test/fixture/v1.0.0/src/left\n@>\nresult\n/test/fixture/v1.0.0/src/left\n@!\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/box", "@<\nvalue\n.slot\n@>\nresult\n.slot\n value\n@!\nlocal\n/test/fixture/v1.0.0/src/left\nworker\n/test/fixture/v1.0.0/src/worker\nworker !\n argument < local\n result > received\n");
    setup_editor();
    assert(analyze(&analysis) && analysis.indexed);
    const SourceText *source = project_find(&analysis.frontend.project, "/test/fixture/v1.0.0/src/box")->source->text;
    SemanticStatement operation_sentinel = {0};
    const SemanticStatement *operation = &operation_sentinel;
    assert(!editor_operation(&analysis,"/test/fixture/v1.0.0/src/box",point(source,"worker !"),&operation));
    assert(operation == &operation_sentinel);
    SemanticBinding binding_sentinel = {0};
    const SemanticBinding *binding = &binding_sentinel;
    assert(!editor_binding(&analysis,"/test/fixture/v1.0.0/src/box",point(source,"argument <"),&binding));
    assert(binding == &binding_sentinel);
    EditorDefinition definition;
    assert(editor_definition(&analysis,"/test/fixture/v1.0.0/src/box",point(source,"argument <"),&definition));
    assert(definition.original && text_is(definition.definition,"argument"));
    assert(definition.module == project_find(&analysis.frontend.project,"/test/fixture/v1.0.0/src/worker"));
    destroy_editor();
}
