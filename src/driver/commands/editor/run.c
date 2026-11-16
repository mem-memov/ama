/** @file
 * @brief Keep checked snapshots alive through one explicit JSON response without applying editor document changes.
 * [COMP-02](../../../../spec/implementation/compiler.md#comp-02)
 * reuses production analysis; [COMP-04](../../../../spec/implementation/compiler.md#comp-04)
 * acknowledges success only after complete output and flush.
 */
#include <stdlib.h>
#include <string.h>
#include "internal.h"
#include "diagnostics/render/render.h"

/** Complete workspace checking and revalidation precede exposing any rename replacement. */
static bool rename_plan(const DriverOptions *options, EditorAnalysis *analysis, EditorRename *rename, ToolJsonWriter *writer, Diagnostic *error) {
    if (!editor_workspace_build(analysis,options->roots,options->root_count,options->entry)) {
        *error = analysis->diagnostic;
        return false;
    }
    bool complete = editor_rename_prepare(rename,analysis,options->query_module,options->offset,
        (const unsigned char *)options->rename_name,strlen(options->rename_name)) && editor_rename_validate(rename);
    if (complete) { complete = driver_editor_rename(options,rename,writer); }
    else {
        /* Render the candidate-backed diagnostic before destroying its source owner. */
        *error = rename->diagnostic;
    }
    return complete;
}
/** Single-shot source requests never create an executable or perform a document transaction. */
int driver_editor(const DriverOptions *options, FILE *output, FILE *errors) {
    EditorAnalysis analysis;
    editor_analysis_init(&analysis,options->arena.allocator);
    EditorRename rename;
    editor_rename_init(&rename,options->arena.allocator);
    ToolJsonWriter writer;
    tool_json_init(&writer,output);
    Diagnostic error = {0};
    bool complete;
    if (options->request == DRIVER_REQUEST_RENAME_PLAN) { complete = rename_plan(options,&analysis,&rename,&writer,&error); }
    else {
        complete = editor_analysis_build(&analysis,options->roots,options->root_count,options->entry);
        if (complete) {
            const ModuleSource *query = NULL;
            complete = module_catalog_find(&analysis.frontend.project.catalog,options->query_module,&query) == MODULE_OK;
            if (!complete || (options->has_offset && options->offset > source_length(query->text))) {
                complete = false;
                error = (Diagnostic){.code=DIAGNOSTIC_INVALID_PATH,.message="The query module or byte offset is outside the captured source domain"};
            } else { complete = driver_editor_response(options,&analysis,&writer,&error); }
        } else { error = analysis.diagnostic; }
    }
    bool flushed = tool_json_finish(&writer);
    if (!complete || !flushed) {
        if (error.message == NULL) { error = (Diagnostic){.code=DIAGNOSTIC_IO_FAILURE,.message="The structured source-tool response could not be completed"}; }
        (void)diagnostic_render(errors,&error);
    }
    editor_rename_destroy(&rename);
    editor_analysis_destroy(&analysis);
    return complete && flushed ? EXIT_SUCCESS : EXIT_FAILURE;
}
