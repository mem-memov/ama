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
#include "../wire.h"
#include "tools/output/json/json.h"
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
