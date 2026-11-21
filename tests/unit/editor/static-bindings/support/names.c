/** @file
 * @brief Preserve atomic Unicode/dotted/hyphenated names across graph forwards and defining versus equality receipts.
 * @see [T192 contract](../../../../../docs/implementation/static-editor-bindings.md)
 */

/** Graph names have whole-module scope; a later equality receipt reuses the
 * first immutable alias rather than introducing a new variable or parameter. */
static void it_preserves_atomic_unicode_graph_names_and_original_receipt_definitions(void) {
    project_fixture("@!\nданные\n/test/fixture/v1.0.0/src/сборка\n .полученный.тип-данных\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/ячейка", "@!\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/коробка-данных", "@<\nзначение\n.тип.ключа-с пробелом\n@!\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/сборщик", "@...\n.выходной.тип-данных\n/test/fixture/v1.0.0/src/коробка-данных\n .тип.ключа-с пробелом < /test/fixture/v1.0.0/src/ячейка\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/сборка", "@...\n.готовый.тип-данных\n.связанный.тип-данных\n.связанный.тип-данных\n/test/fixture/v1.0.0/src/коробка-данных\n .тип.ключа-с пробелом < /test/fixture/v1.0.0/src/ячейка\n/test/fixture/v1.0.0/src/сборщик\n .выходной.тип-данных > .полученный.тип-данных\n/test/fixture/v1.0.0/src/сборщик\n .выходной.тип-данных > .полученный.тип-данных\n");
    setup_editor();
    assert(analyze(&analysis) && analysis.indexed);
    const SourceText *source = project_find(&analysis.frontend.project,"/test/fixture/v1.0.0/src/сборка")->source->text;
    const char *bytes = (const char *)source_bytes(source);
    const char *alias = "> .полученный.тип-данных";
    const char *first = strstr(bytes,alias), *second = first ? strstr(first+1,alias) : NULL;
    assert(first && second);
    EditorDefinition receipt;
    assert(editor_definition(&analysis,"/test/fixture/v1.0.0/src/сборка",(size_t)(second-bytes)+2,&receipt));
    assert(receipt.static_symbol && receipt.static_symbol->role == STATIC_ALIAS);
    const StaticSymbol *alias_symbol = receipt.static_symbol;
    assert(receipt.definition.start == (size_t)(first-bytes)+2);
    assert(text_is(receipt.definition,".полученный.тип-данных"));
    const char *forward = strstr(bytes,".связанный.тип-данных");
    const char *declaration = forward ? strstr(forward+1,".связанный.тип-данных") : NULL;
    assert(forward && declaration);
    assert(editor_definition(&analysis,"/test/fixture/v1.0.0/src/сборка",(size_t)(forward-bytes),&receipt));
    assert(receipt.definition.start == (size_t)(declaration-bytes));
    EditorDefinition label;
    assert(editor_definition(&analysis,"/test/fixture/v1.0.0/src/сборка",point(source,".тип.ключа-с пробелом <")+strlen(".тип."),&label));
    assert(label.static_symbol && text_is(label.definition,".тип.ключа-с пробелом"));
    assert(label.static_symbol->owner->definition == project_find(&analysis.frontend.project,"/test/fixture/v1.0.0/src/коробка-данных"));
    EditorDefinition selected;
    assert(editor_definition(&analysis,"/test/fixture/v1.0.0/src/main",point(analysis.selected->text,".полученный.тип-данных"),&selected));
    assert(selected.static_symbol == alias_symbol);
    assert(selected.definition.start == (size_t)(first-bytes)+2 && selected.definition.source == source);
    destroy_editor();
}

/** Nested declaration selectors and named inputs bind the same original foreign
 * symbols at both indentation levels; they never become caller placeholders.
 */
static void it_navigates_nested_declaration_inputs_and_selections_to_exact_foreign_symbols(void) {
    project_fixture("@!\nданные&\n./сборщик\n .тип.входа < ./сборщик\n  .тип.входа < ./cell\n  .результат\n .результат\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/сборщик", "@...\n.результат\n.тип.входа\n");
    setup_editor();
    assert(analyze(&analysis) && analysis.indexed);
    const SourceText *source = analysis.selected->text;
    const char *owner = "/test/fixture/v1.0.0/src/main";
    EditorDefinition outer, inner;
    assert(editor_definition(&analysis, owner, point(source, "\n .результат\n") + 2, &outer));
    assert(editor_definition(&analysis, owner, point(source, "\n  .результат\n") + 3, &inner));
    assert(outer.static_symbol && outer.static_symbol == inner.static_symbol);
    assert(text_is(outer.definition, ".результат") && outer.static_symbol->role == STATIC_RESULT);
    const ProjectModule *factory = project_find(&analysis.frontend.project, "/test/fixture/v1.0.0/src/сборщик");
    assert(outer.static_symbol->owner->definition == factory);
    assert(editor_definition(&analysis, owner, point(source, "\n .тип.входа <") + 2, &outer));
    assert(editor_definition(&analysis, owner, point(source, "\n  .тип.входа <") + 3, &inner));
    assert(outer.static_symbol && outer.static_symbol == inner.static_symbol);
    assert(text_is(outer.definition, ".тип.входа") && outer.static_symbol->role == STATIC_PARAMETER);
    assert(outer.static_symbol->owner->definition == factory);
    destroy_editor();
}
