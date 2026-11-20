/** @file
 * @brief Emit one immutable editor snapshot through synchronous original-source callbacks.
 * [COMP-02](../../../../spec/implementation/compiler.md#comp-02)
 * reuses compiler metadata and production lexing, including intentional invalid/draft states.
 */
#include "internal.h"
/** A failed element prefix stops its caller without converting stream failure into successful enumeration. Component contract: [COMP-02](../../../../spec/implementation/compiler.md#comp-02). */
bool driver_editor_item(DriverEditorArray *array) {
    if (!array->first && !tool_json_raw(array->writer,",")) { return false; }
    array->first = false;
    return array->writer->status == TOOL_JSON_OK;
}
/** Callback borrows the declaration only for this synchronous emission. Component contract: [COMP-02](../../../../spec/implementation/compiler.md#comp-02). */
bool driver_editor_symbol(void *context, const EditorDefinition *symbol) {
    DriverEditorArray *array = context;
    return driver_editor_item(array) && driver_editor_declaration(array->writer,symbol);
}
/** Error/warning ordering remains the analysis API's original deterministic order. Component contract: [COMP-02](../../../../spec/implementation/compiler.md#comp-02). */
static bool diagnostic(void *context, const Diagnostic *value, bool warning) {
    DriverEditorArray *array = context;
    return driver_editor_item(array) && tool_source_diagnostic(array->writer,value,warning);
}
/** Highlight records preserve complete spelling/payload bytes and exact original spans without retaining temporary lexer records. Component contract: [COMP-02](../../../../spec/implementation/compiler.md#comp-02). */
static bool highlight(void *context, const EditorHighlight *value) {
    DriverEditorArray *array = context;
    ToolJsonWriter *writer = array->writer;
    return driver_editor_item(array) && tool_json_raw(writer,"{\"span\":") && tool_source_span(writer,value->spelling)
        && tool_json_raw(writer,",\"bytes\":") && tool_source_spelling(writer,value->spelling)
        && tool_json_raw(writer,",\"payload\":") && tool_source_spelling(writer,value->payload)
        && tool_json_raw(writer,",\"comment\":") && tool_json_boolean(writer,value->comment)
        && tool_json_raw(writer,",\"category\":") && (value->comment ? tool_json_raw(writer,"null") : tool_json_unsigned(writer,value->kind)) && tool_json_raw(writer,"}");
}
/** Expose each declared interface role independently; unavailable metadata stays an explicit empty/unavailable interface. Component contract: [COMP-02](../../../../spec/implementation/compiler.md#comp-02). */
static bool signature(const DriverOptions *options, const EditorAnalysis *analysis, ToolJsonWriter *writer) {
    const char *names[] = {"constants","parameters","results"};
    const SymbolRole roles[] = {SYMBOL_CONSTANT,SYMBOL_PARAMETER,SYMBOL_RESULT};
    bool available = analysis->indexed && project_find(&analysis->frontend.project,options->query_module) != NULL;
    if (!tool_json_raw(writer,"{\"available\":") || !tool_json_boolean(writer,available)) { return false; }
    for (size_t index = 0; index < sizeof(roles)/sizeof(*roles); index++) {
        DriverEditorArray array = {writer,true};
        if (!tool_json_raw(writer,",") || !tool_json_string(writer,names[index]) || !tool_json_raw(writer,":[")) { return false; }
        (void)editor_signature(analysis,options->query_module,roles[index],driver_editor_symbol,&array);
        if (!tool_json_raw(writer,"]")) { return false; }
    }
    return tool_json_raw(writer,"}");
}
/** A lexical error is structured data; resource failure additionally prevents a successful command acknowledgement. Component contract: [COMP-02](../../../../spec/implementation/compiler.md#comp-02). */
bool driver_editor_response(const DriverOptions *options, const EditorAnalysis *analysis, ToolJsonWriter *writer, Diagnostic *error) {
    const char *states[] = {"complete","draft","invalid","unavailable"};
    if (!tool_json_raw(writer,"{\"schema\":\"ama.editor.v1\",\"state\":") || !tool_json_string(writer,states[analysis->state])
        || !tool_json_raw(writer,",\"entry\":") || !tool_json_string(writer,options->entry)
        || !tool_json_raw(writer,",\"module\":") || !tool_json_string(writer,options->query_module)
        || !tool_json_raw(writer,",\"diagnostics\":[")) { return false; }
    DriverEditorArray array = {writer,true};
    (void)editor_diagnostics(analysis,diagnostic,&array);
    if (!tool_json_raw(writer,"],\"highlights\":{\"items\":[")) { return false; }
    array = (DriverEditorArray){writer,true};
    Diagnostic lexical = {0};
    bool complete = editor_highlights(analysis,options->query_module,highlight,&array,&lexical);
    if (!tool_json_raw(writer,"],\"complete\":") || !tool_json_boolean(writer,complete)
        || !tool_json_raw(writer,",\"error\":")
        || !(lexical.message == NULL ? tool_json_raw(writer,"null") : tool_source_diagnostic(writer,&lexical,false))
        || !tool_json_raw(writer,"},\"signature\":") || !signature(options,analysis,writer)
        || !tool_json_raw(writer,",\"point\":")
        || !(options->has_offset ? driver_editor_point(options,analysis,writer) : tool_json_raw(writer,"null"))
        || !tool_json_raw(writer,"}\n")) { return false; }
    if (lexical.code == DIAGNOSTIC_OUT_OF_MEMORY && lexical.message != NULL) { *error = lexical; return false; }
    return true;
}
