/** @file
 * @brief Emit exact byte-point definitions, visible names and checked nominal/intrinsic call/binding metadata.
 * [NAME-06](../../../../spec/language/declarations.md#name-06)
 * preserves scoped identities; [LIST-04](../../../../spec/language/lists.md#list-04)
 * keeps intrinsic signatures distinct from source module declarations.
 */
#include "internal.h"
/** A missing exact spelling is a successful absent query, never a guessed adjacent declaration. Component contract: [NAME-06](../../../../spec/language/declarations.md#name-06), [LIST-04](../../../../spec/language/lists.md#list-04). */
static bool definition(const DriverOptions *options, const EditorAnalysis *analysis, ToolJsonWriter *writer) {
    EditorDefinition value;
    if (!editor_definition(analysis,options->query_module,options->offset,&value)) { return tool_json_raw(writer,"null"); }
    return tool_json_raw(writer,"{\"declaration\":") && tool_source_span(writer,value.definition)
        && tool_json_raw(writer,",\"symbol\":") && tool_source_symbol(writer,value.symbol)
        && tool_json_raw(writer,",\"type\":") && tool_source_type(writer,value.type) && tool_json_raw(writer,"}");
}
/** Intrinsic signatures use their actual static field descriptors and currently resolved nominal element/integer types. Component contract: [NAME-06](../../../../spec/language/declarations.md#name-06), [LIST-04](../../../../spec/language/lists.md#list-04). */
static bool intrinsic(const SemanticStatement *operation, ToolJsonWriter *writer) {
    if (operation->operation_kind != SEMANTIC_LIST_METHOD_OPERATION) { return tool_json_raw(writer,"null"); }
    const char *name = semantic_list_method_name(operation->list_method);
    if (name == NULL) { writer->status = TOOL_JSON_INVALID_VALUE; return false; }
    if (!tool_json_raw(writer,"{\"method\":") || !tool_json_string(writer,name)) { return false; }
    for (unsigned result = 0; result < 2; result++) {
        if (!tool_json_raw(writer,result ? ",\"results\":[" : ",\"parameters\":[")) { return false; }
        for (size_t position = 0;; position++) {
            const SemanticListField *field = semantic_list_position(operation->list_method,result != 0,position);
            if (field == NULL) { break; }
            if ((position != 0 && !tool_json_raw(writer,",")) || !tool_json_raw(writer,"{\"name\":")
                || !tool_json_string(writer,field->name) || !tool_json_raw(writer,",\"reference\":")
                || !tool_json_boolean(writer,field->element_reference) || !tool_json_raw(writer,",\"type\":")
                || !tool_source_type(writer,field->element_reference ? operation->list_interface.element : operation->list_interface.integer)
                || !tool_json_raw(writer,"}")) { return false; }
        }
        if (!tool_json_raw(writer,"]")) { return false; }
    }
    return tool_json_raw(writer,"}");
}
/** Only an actual checked call at this written target exposes operation metadata. Component contract: [NAME-06](../../../../spec/language/declarations.md#name-06), [LIST-04](../../../../spec/language/lists.md#list-04). */
static bool operation(const DriverOptions *options, const EditorAnalysis *analysis, ToolJsonWriter *writer) {
    const SemanticStatement *value;
    if (!editor_operation(analysis,options->query_module,options->offset,&value)) { return tool_json_raw(writer,"null"); }
    const char *kinds[] = {"instance","constant","list-service","list-method"};
    return tool_json_raw(writer,"{\"kind\":") && tool_json_string(writer,kinds[value->operation_kind])
        && tool_json_raw(writer,",\"namespace\":") && tool_source_type(writer,value->namespace_type)
        && tool_json_raw(writer,",\"element_type\":") && tool_source_type(writer,value->list_interface.element)
        && tool_json_raw(writer,",\"integer_type\":") && tool_source_type(writer,value->list_interface.integer)
        && tool_json_raw(writer,",\"element_reference\":") && tool_json_boolean(writer,value->list_interface.reference)
        && tool_json_raw(writer,",\"intrinsic\":") && intrinsic(value,writer) && tool_json_raw(writer,"}");
}
/** Written callee labels keep their real declaration or intrinsic descriptor and independent position. Component contract: [NAME-06](../../../../spec/language/declarations.md#name-06), [LIST-04](../../../../spec/language/lists.md#list-04). */
static bool binding(const DriverOptions *options, const EditorAnalysis *analysis, ToolJsonWriter *writer) {
    const SemanticBinding *value;
    if (!editor_binding(analysis,options->query_module,options->offset,&value)) { return tool_json_raw(writer,"null"); }
    return tool_json_raw(writer,"{\"field\":") && tool_source_symbol(writer,value->field)
        && tool_json_raw(writer,",\"intrinsic_name\":") && tool_json_string(writer,value->intrinsic == NULL ? NULL : value->intrinsic->name)
        && tool_json_raw(writer,",\"type\":") && tool_source_type(writer,value->type)
        && tool_json_raw(writer,",\"position\":") && tool_json_unsigned(writer,value->position)
        && tool_json_raw(writer,",\"reference\":") && tool_json_boolean(writer,value->reference)
        && tool_json_raw(writer,",\"list\":") && tool_json_boolean(writer,value->list) && tool_json_raw(writer,"}");
}
/** Visible-name enumeration delegates declaration order and foreach scope to the semantic visibility predicate. Component contract: [NAME-06](../../../../spec/language/declarations.md#name-06), [LIST-04](../../../../spec/language/lists.md#list-04). */
bool driver_editor_point(const DriverOptions *options, const EditorAnalysis *analysis, ToolJsonWriter *writer) {
    if (!tool_json_raw(writer,"{\"offset\":") || !tool_json_unsigned(writer,options->offset)
        || !tool_json_raw(writer,",\"definition\":") || !definition(options,analysis,writer)
        || !tool_json_raw(writer,",\"operation\":") || !operation(options,analysis,writer)
        || !tool_json_raw(writer,",\"binding\":") || !binding(options,analysis,writer)
        || !tool_json_raw(writer,",\"completions\":{\"items\":[")) { return false; }
    DriverEditorArray array = {writer,true};
    bool available = editor_completions(analysis,options->query_module,options->offset,driver_editor_symbol,&array);
    return tool_json_raw(writer,"],\"available\":") && tool_json_boolean(writer,available) && tool_json_raw(writer,"}}");
}
