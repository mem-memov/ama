/** @file
 * @brief Declare source-domain byte, catalog and graph report composition.
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01)
 * separates compiler analysis from project mutation;
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves original bindings, capture state and qualified constant members;
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) retains exact
 * bytes and source coordinates and refuses incomplete output as success.
 */
#ifndef AMA_DELIVERY_SOURCE_REPORT_INTERNAL_H
#define AMA_DELIVERY_SOURCE_REPORT_INTERNAL_H
#include "report.h"
#include "tools/output/json/json.h"
/** Encode a nullable physical pathname as lossless bytes, never lossy UTF-8.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves bytes.
 */
bool delivery_source_report_path(ToolJsonWriter *, const char *);
/** Encode a nullable owned source snapshot including original path/content.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves bytes.
 */
bool delivery_source_report_text(ToolJsonWriter *, const SourceText *);
/** Append versions and all catalog sources in authoritative catalog order.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * retains source/configuration authority without choosing new roots.
 */
bool delivery_source_report_catalog(ToolJsonWriter *, const ModuleCatalog *);
/** Append the checked, byte-sorted definition graph and original edge order.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires exact source sites, targets and normalized members.
 */
bool delivery_source_report_graph(ToolJsonWriter *, const DeliverySourceDefinition *);
#endif
