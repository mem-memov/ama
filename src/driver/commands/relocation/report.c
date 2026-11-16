/** @file
 * @brief Render complete supplied relocation scope without rereading the changed filesystem or guessing logical identities.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * reports final roots, identity maps, rewritten files, preserved drafts and rebuild requirements.
 */
#include "internal.h"

/** Root paths/kinds/exclusions remain the exact returned configuration, including carried or newly introduced roots. */
static bool roots(ToolJsonWriter *writer, const ModuleRoot *items, size_t count) {
    if (!tool_json_raw(writer,"[")) { return false; }
    for (size_t index = 0; index < count; index++) {
        if ((index != 0 && !tool_json_raw(writer,",")) || !tool_json_raw(writer,"{\"path\":")
            || !tool_source_bytes(writer,items[index].path) || !tool_json_raw(writer,",\"kind\":")
            || !tool_json_string(writer,items[index].kind == MODULE_ROOT_STANDARD ? "standard" : "project")
            || !tool_json_raw(writer,",\"excluded\":[")) { return false; }
        for (size_t item = 0; item < items[index].excluded_count; item++) {
            if ((item != 0 && !tool_json_raw(writer,",")) || !tool_source_bytes(writer,items[index].excluded[item])) { return false; }
        }
        if (!tool_json_raw(writer,"]}")) { return false; }
    }
    return tool_json_raw(writer,"]");
}
/** Nullable original identity denotes introduced analysis scope; it never becomes an invented old namespace. */
static bool modules(ToolJsonWriter *writer, const RelocationReport *report) {
    if (!tool_json_raw(writer,"[")) { return false; }
    bool first = true;
    for (const RelocationReportedModule *item = report->modules; item != NULL; item = item->next) {
        if ((!first && !tool_json_raw(writer,",")) || !tool_json_raw(writer,"{\"before_path\":")
            || !tool_source_bytes(writer,item->before) || !tool_json_raw(writer,",\"after_path\":")
            || !tool_source_bytes(writer,item->after) || !tool_json_raw(writer,",\"before_identity\":")
            || !tool_json_string(writer,item->before_identity) || !tool_json_raw(writer,",\"after_identity\":")
            || !tool_json_string(writer,item->after_identity) || !tool_json_raw(writer,",\"active\":")
            || !tool_json_boolean(writer,item->active) || !tool_json_raw(writer,"}")) { return false; }
        first = false;
    }
    return tool_json_raw(writer,"]");
}
/** File roles come from actual planned records; arbitrary C/literal/path-looking bytes are not searched. */
static bool files(ToolJsonWriter *writer, const RelocationReport *report) {
    if (!tool_json_raw(writer,"[")) { return false; }
    bool first = true;
    for (const RelocationReportedFile *item = report->files; item != NULL; item = item->next) {
        if ((!first && !tool_json_raw(writer,",")) || !tool_json_raw(writer,"{\"before\":")
            || !tool_source_bytes(writer,item->before) || !tool_json_raw(writer,",\"after\":")
            || !tool_source_bytes(writer,item->after) || !tool_json_raw(writer,",\"mode\":")
            || !tool_json_unsigned(writer,(uintmax_t)item->mode) || !tool_json_raw(writer,",\"existed\":")
            || !tool_json_boolean(writer,item->existed) || !tool_json_raw(writer,",\"rewritten\":")
            || !tool_json_boolean(writer,item->rewritten) || !tool_json_raw(writer,",\"draft\":")
            || !tool_json_boolean(writer,item->draft) || !tool_json_raw(writer,",\"opaque\":")
            || !tool_json_boolean(writer,item->opaque) || !tool_json_raw(writer,",\"invalidated\":")
            || !tool_json_boolean(writer,item->invalidated) || !tool_json_raw(writer,"}")) { return false; }
        first = false;
    }
    return tool_json_raw(writer,"]");
}
/** Only a fully owned report can be consumed; failed prefixes never masquerade as complete scope. */
bool driver_relocation_report(ToolJsonWriter *writer, const RelocationReport *report) {
    if (report == NULL || !report->ready) { return tool_json_raw(writer,"null"); }
    return tool_json_raw(writer,"{\"source\":") && tool_source_bytes(writer,report->source)
        && tool_json_raw(writer,",\"destination\":") && tool_source_bytes(writer,report->destination)
        && tool_json_raw(writer,",\"entry\":") && tool_json_string(writer,report->entry)
        && tool_json_raw(writer,",\"before_roots\":") && roots(writer,report->before_roots,report->before_root_count)
        && tool_json_raw(writer,",\"after_roots\":") && roots(writer,report->after_roots,report->after_root_count)
        && tool_json_raw(writer,",\"modules\":") && modules(writer,report)
        && tool_json_raw(writer,",\"files\":") && files(writer,report)
        && tool_json_raw(writer,",\"rebuild_required\":") && tool_json_boolean(writer,report->rebuild_required) && tool_json_raw(writer,"}");
}
