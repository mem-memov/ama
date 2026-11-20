/** @file
 * @brief Publish a validated editor change plan as exact versioned bytes without performing a filesystem rename transaction.
 * [NAME-06](../../../../spec/language/declarations.md#name-06)
 * selects checked declaration identities; [SRC-03](../../../../spec/language/source-and-names.md#src-03)
 * preserves every non-name byte for the client's atomic document transaction.
 */
#include "internal.h"
/** Expected device/inode plus complete original bytes let the client check its exact document version without hash assumptions. Component contract: [NAME-06](../../../../spec/language/declarations.md#name-06), [SRC-03](../../../../spec/language/source-and-names.md#src-03). */
static bool file(ToolJsonWriter *writer, const EditorRenameFile *value) {
    const SourceText *old = value->original->text, *replacement = value->replacement;
    if (!tool_json_raw(writer,"{\"path\":") || !tool_source_bytes(writer,source_path(old))
        || !tool_json_raw(writer,",\"device\":") || !tool_json_unsigned(writer,(uintmax_t)value->original->device)
        || !tool_json_raw(writer,",\"inode\":") || !tool_json_unsigned(writer,(uintmax_t)value->original->inode)
        || !tool_json_raw(writer,",\"original\":") || !tool_json_bytes(writer,source_bytes(old),source_length(old))
        || !tool_json_raw(writer,",\"replacement\":") || !tool_json_bytes(writer,source_bytes(replacement),source_length(replacement))
        || !tool_json_raw(writer,",\"edits\":[")) { return false; }
    bool first = true;
    for (const EditorRenameEdit *edit = value->first; edit != NULL; edit = edit->next) {
        if ((!first && !tool_json_raw(writer,",")) || !tool_json_raw(writer,"{\"original\":")
            || !tool_source_span(writer,edit->original) || !tool_json_raw(writer,",\"new_start\":")
            || !tool_json_unsigned(writer,edit->new_start) || !tool_json_raw(writer,",\"new_end\":")
            || !tool_json_unsigned(writer,edit->new_end) || !tool_json_raw(writer,"}")) { return false; }
        first = false;
    }
    return tool_json_raw(writer,"]}");
}
/** All files are one client transaction; a same-name no-op is explicitly an empty plan. Component contract: [NAME-06](../../../../spec/language/declarations.md#name-06), [SRC-03](../../../../spec/language/source-and-names.md#src-03). */
bool driver_editor_rename(const DriverOptions *options, const EditorRename *rename, ToolJsonWriter *writer) {
    if (!tool_json_raw(writer,"{\"schema\":\"ama.rename.v1\",\"entry\":") || !tool_json_string(writer,options->entry)
        || !tool_json_raw(writer,",\"module\":") || !tool_json_string(writer,options->query_module)
        || !tool_json_raw(writer,",\"name\":") || !tool_source_bytes(writer,options->rename_name)
        || !tool_json_raw(writer,",\"declaration\":") || !tool_source_span(writer,rename->target.definition)
        || !tool_json_raw(writer,",\"files\":[")) { return false; }
    bool first = true;
    for (const EditorRenameFile *value = rename->first; value != NULL; value = value->next) {
        if ((!first && !tool_json_raw(writer,",")) || !file(writer,value)) { return false; }
        first = false;
    }
    return tool_json_raw(writer,"]}\n");
}
