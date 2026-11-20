/** @file
 * @brief Observe original parameter and static graph interfaces independently of runtime capture variants.
 * @see [T192 contract](../../../../../docs/implementation/static-editor-bindings.md)
 */

/** Copies retain borrowed source/metadata pointers, never the temporary callback record itself. */
typedef struct { EditorDefinition items[16]; size_t count; } StaticViews;

/** Collect finite source observations without selecting a concrete specialization. */
static bool collect_static_view(void *context, const EditorDefinition *value) {
    StaticViews *views = context;
    assert(views->count < sizeof(views->items)/sizeof(*views->items));
    views->items[views->count++] = *value;
    return true;
}

/** Original runtime inputs remain one interface field, and static constructor
 * parameters do not become extra runtime arguments in an ordinary template. */
static void it_enumerates_source_interfaces_without_selecting_the_first_capture(void) {
    static_fixture(true);
    setup_editor();
    assert(analyze(&analysis) && analysis.indexed);
    StaticViews inputs = {0}, outputs = {0}, visible = {0};
    assert(editor_signature(&analysis,".box",SYMBOL_PARAMETER,collect_static_view,&inputs));
    assert(inputs.count == 1 && inputs.items[0].original && !inputs.items[0].symbol && !inputs.items[0].type);
    assert(text_is(inputs.items[0].definition,"value"));
    assert(editor_signature(&analysis,".box",SYMBOL_RESULT,collect_static_view,&outputs));
    assert(outputs.count == 1 && text_is(outputs.items[0].definition,"result"));
    assert(editor_completions(&analysis,".box",0,collect_static_view,&visible));
    assert(visible.count == 3);
    size_t runtime = 0, parameters = 0;
    for (size_t index = 0; index < visible.count; index++) {
        runtime += visible.items[index].original != NULL;
        parameters += visible.items[index].static_symbol != NULL;
    }
    assert(runtime == 2 && parameters == 1);
    destroy_editor();
}

/** Static graph symbols are visible before their definitions and expose named
 * metadata results; no ordinary runtime field or Type is invented for them. */
static void it_enumerates_whole_graph_static_results_without_runtime_slots(void) {
    static_fixture(true);
    setup_editor();
    assert(analyze(&analysis) && analysis.indexed);
    StaticViews visible = {0}, inputs = {0}, results = {0};
    assert(editor_completions(&analysis,".factory",0,collect_static_view,&visible));
    assert(editor_signature(&analysis,".factory",SYMBOL_PARAMETER,collect_static_view,&inputs));
    assert(editor_signature(&analysis,".factory",SYMBOL_RESULT,collect_static_view,&results));
    assert(visible.count == 2 && inputs.count == 0 && results.count == 2);
    for (size_t index = 0; index < results.count; index++) {
        assert(results.items[index].static_symbol && !results.items[index].original);
        assert(!results.items[index].symbol && !results.items[index].type);
    }
    destroy_editor();
}
