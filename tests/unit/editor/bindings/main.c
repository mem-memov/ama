/** @file
 * @brief Check exact nominal definitions, qualified constants, signatures and source-ordered lexical visibility.
 */
#include "../common.c"
/** Bounded observations retain checked symbols rather than guessing names from displayed text. */
typedef struct { const SemanticSymbol *items[64]; size_t count; bool stop; } Symbols;
/** Record deterministic finite metadata, optionally ending enumeration after its first result. */
static bool collect(void *context, const SemanticSymbol *field) {
    Symbols *symbols = context;
    assert(symbols->count < sizeof(symbols->items)/sizeof(*symbols->items));
    symbols->items[symbols->count++] = field;
    return !symbols->stop;
}
/** Exact complete-name lookup over returned metadata has no prefix or normalization fallback. */
static const SemanticSymbol *named(const Symbols *symbols, const char *name) {
    const SemanticSymbol *found = NULL;
    for (size_t index = 0; index < symbols->count; index++) {
        if (text_is(symbols->items[index]->name,name)) { assert(found == NULL); found = symbols->items[index]; }
    }
    return found;
}
/** Query a real original source point and retain its exact visible declarations. */
static Symbols complete(size_t offset) {
    Symbols result = {0};
    assert(editor_completions(&analysis,".main",offset,collect,&result));
    return result;
}
/** Qualifiers, direction labels and receivers bind to their respective exact semantic owners. */
static void it_resolves_definitions_and_signatures_without_crossing_caller_callee_namespaces(void) {
    project_fixture("@!\nvalue\n.cell\nworker\n.worker\nworker !\n argument < value\n result& > new reference&\n.palette\n shared >> shared copy\nworker\n result& !\n");
    setup_editor();
    assert(analyze(&analysis) && analysis.state == EDITOR_COMPLETE && analysis.indexed);
    const SourceText *source = analysis.selected->text;
    const SemanticStatement *operation = NULL;
    assert(editor_operation(&analysis, ".main", point(source, "worker !"), &operation));
    assert(operation->syntax->as.call.namespace_expression == NULL);
    assert(editor_operation(&analysis, ".main", point(source, ".palette"), &operation));
    assert(layout_text_is(operation->syntax->as.call.namespace_expression->head, ".palette"));
    assert(editor_operation(&analysis, ".main", point(source, "result& !"), &operation));
    assert(operation->syntax->as.call.namespace_expression == NULL);
    EditorDefinition definition;
    assert(editor_definition(&analysis,".main",point(source,"argument <"),&definition));
    assert(text_is(definition.symbol->name,"argument") && definition.symbol->role == SYMBOL_PARAMETER);
    assert(strcmp(definition.symbol->owner->module->source->logical_path,".worker") == 0);
    assert(editor_definition(&analysis,".main",point(source,"< value")+2,&definition));
    assert(text_is(definition.symbol->name,"value") && definition.symbol->owner->module->source->text == source);
    assert(editor_definition(&analysis,".main",point(source,"new reference&"),&definition));
    assert(definition.symbol->inferred && definition.symbol->reference && !definition.symbol->list);
    assert(text_is(definition.definition,"new reference"));
    assert(editor_definition(&analysis,".main",point(source,".palette"),&definition));
    assert(definition.symbol == NULL && strcmp(definition.type->module->source->logical_path,".palette") == 0);
    assert(editor_definition(&analysis,".main",point(source,"shared >>"),&definition));
    assert(definition.symbol->role == SYMBOL_CONSTANT && text_is(definition.definition,"shared"));
    assert(editor_definition(&analysis,".main",point(source,"result& !"),&definition));
    assert(definition.symbol->role == SYMBOL_RESULT && strcmp(definition.symbol->owner->module->source->logical_path,".worker") == 0);
    Symbols parameters = {0}, results = {0};
    assert(editor_signature(&analysis,".worker",SYMBOL_PARAMETER,collect,&parameters) && parameters.count == 1);
    assert(editor_signature(&analysis,".worker",SYMBOL_RESULT,collect,&results) && results.count == 2);
    assert(named(&results,"result")->reference && named(&results,"items")->list);
    assert(!editor_signature(&analysis,".worker",SYMBOL_STATE,collect,&results));
    Symbols stopped = {.stop=true};
    assert(editor_signature(&analysis,".worker",SYMBOL_RESULT,collect,&stopped) && stopped.count == 1);
    assert(!editor_definition(&analysis,".main",source_length(source),&definition));
    assert(!editor_definition(&analysis,".missing",0,&definition));
    destroy_editor();
}
/** Foreach locals and a later same-named inferred field remain different declarations with disjoint visibility. */
static void it_completes_exact_foreach_scopes_and_source_ordered_inferred_fields(void) {
    project_fixture("@!\nvalues[]\n.cell\nvalue\n.cell\nworker\n.worker\ncopied\n value\n"
        "values[item&]\n{\n element\n  % item&\n}\nworker !\n result& > item&\nlater\n.cell\n");
    setup_editor();
    assert(analyze(&analysis) && analysis.indexed);
    const SourceText *source = analysis.selected->text;
    Symbols first = complete(0);
    assert(named(&first,"later") != NULL && named(&first,"copied") == NULL && named(&first,"item") == NULL);
    Symbols header = complete(point(source,"values[item&]"));
    assert(named(&header,"copied") != NULL && named(&header,"item") == NULL);
    Symbols body = complete(point(source,"% item&"));
    const SemanticSymbol *local = named(&body,"item");
    assert(local != NULL && local->local && local->reference);
    Symbols after = complete(point(source,"worker !\n result"));
    assert(named(&after,"element") != NULL && named(&after,"item") == NULL);
    Symbols introduced = complete(point(source,"result& > item&")+strlen("result& > "));
    const SemanticSymbol *persistent = named(&introduced,"item");
    assert(persistent != NULL && persistent != local && persistent->inferred && !persistent->local);
    EditorDefinition definition;
    assert(editor_definition(&analysis,".main",point(source,"% item&")+2,&definition) && definition.symbol == local);
    assert(editor_definition(&analysis,".main",point(source,"result& > item&")+strlen("result& > "),&definition));
    assert(definition.symbol == persistent);
    assert(!editor_completions(&analysis,".main",source_length(source)+1,collect,&first));
    destroy_editor();
}
/** Constructor completion delegates the same pure ordering/constant policy used by compiler diagnostics. */
static void it_excludes_uninitialized_constructor_fields_without_mutating_semantic_diagnostics(void) {
    project_fixture("@=\nshared\n.cell\n@!\nfirst\n.cell\ncopy\n.cell\n first\nlater\n.cell\n");
    setup_editor();
    assert(analyze(&analysis) && analysis.indexed);
    Symbols available = complete(point(analysis.selected->text," first")+1);
    assert(named(&available,"first") != NULL && named(&available,"shared") != NULL);
    assert(named(&available,"copy") == NULL && named(&available,"later") == NULL);
    const SemanticType *type = semantic_find_type(&analysis.frontend.semantic,".main");
    assert(type != NULL);
    const SemanticSymbol *constant = named(&available,"shared"), *field = named(&available,"first");
    assert(semantic_initializer_visibility(field,constant) == SEMANTIC_CONSTANT_ONLY);
    assert(semantic_initializer_visibility(constant,field) == SEMANTIC_VISIBLE);
    assert(analysis.frontend.valid && analysis.indexed);
    destroy_editor();
}
/** Intrinsic methods expose the same fixed interface descriptors as semantic checking, without inventing a source definition. */
static void it_exposes_checked_list_method_signatures_without_fabricated_module_types(void) {
    project_fixture("@!\nvalues[]\n.cell\npositions[]\n..std.index\nposition\n..std.index\n |0\nvalues[positions[position]] !\nvalues[]\n append !\n  > element&\n length !\n  > count\n");
    setup_editor();
    ModuleRoot roots[] = {{"generated/project",NULL,0,MODULE_ROOT_PROJECT},{TEST_STANDARD_ROOT,NULL,0,MODULE_ROOT_STANDARD}};
    bool valid = editor_analysis_build(&analysis,roots,2,".main");
    if (!valid || !analysis.indexed) { fprintf(stderr,"List editor analysis: %s\n",analysis.diagnostic.message); }
    assert(valid && analysis.indexed);
    const SemanticStatement *operation = NULL;
    const SourceText *source = analysis.selected->text;
    EditorDefinition index_definition;
    assert(editor_definition(&analysis,".main",point(source,"values[positions[position]]")+strlen("values[positions["),&index_definition));
    assert(text_is(index_definition.definition,"position"));
    assert(editor_definition(&analysis,".main",point(source,"values[positions[position]]")+strlen("values["),&index_definition));
    assert(text_is(index_definition.definition,"positions"));
    const SemanticBinding *binding = NULL;
    assert(editor_binding(&analysis,".main",point(source,"> element&"),&binding));
    assert(binding->field == NULL && binding->intrinsic != NULL && binding->intrinsic->element_reference);
    assert(binding->reference && strcmp(binding->type->module->source->logical_path,".cell") == 0);

    assert(editor_operation(&analysis,".main",point(source,"append !"),&operation));
    assert(operation->operation_kind == SEMANTIC_LIST_METHOD_OPERATION && operation->list_method == SEMANTIC_LIST_APPEND);
    assert(strcmp(operation->list_interface.element->module->source->logical_path,".cell") == 0);
    const SemanticListField *input = semantic_list_position(operation->list_method,false,0);
    assert(input != NULL && input->element_reference && strcmp(input->name,"source") == 0);
    assert(semantic_list_position(operation->list_method,false,1) == NULL);
    EditorDefinition definition;
    assert(!editor_definition(&analysis,".main",point(source,"append !"),&definition));
    assert(editor_operation(&analysis,".main",point(source,"length !"),&operation));
    assert(operation->list_method == SEMANTIC_LIST_LENGTH && operation->list_interface.integer != NULL);
    const SemanticListField *result = semantic_list_position(operation->list_method,true,0);
    assert(result != NULL && !result->element_reference && strcmp(result->name,"value") == 0);
    assert(!editor_operation(&analysis,".main",source_length(source),&operation));
    assert(text_is(operation->syntax->as.call.target->name,"length"));
    destroy_editor();
}
/** Run identity, namespace, scope and initialization components against real checked IR. */
void run(void) {
    it_exposes_checked_list_method_signatures_without_fabricated_module_types();
    it_resolves_definitions_and_signatures_without_crossing_caller_callee_namespaces();
    it_completes_exact_foreach_scopes_and_source_ordered_inferred_fields();
    it_excludes_uninitialized_constructor_fields_without_mutating_semantic_diagnostics();
}
/** All semantic/source borrows are consumed before counted fixture teardown. */
int main(void) { run(); return EXIT_SUCCESS; }
