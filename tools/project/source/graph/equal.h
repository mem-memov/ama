/** @file
 * @brief Compare exact checked source-graph projections independently of snapshot byte admission.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves original normalized bindings and sites;
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * refuses changed prepared graph facts before use.
 */
#ifndef AMA_PROJECT_SOURCE_GRAPH_EQUAL_H
#define AMA_PROJECT_SOURCE_GRAPH_EQUAL_H
#include "delivery/source/records.h"
/** Compare finite closed-protocol definition/edge lists in their canonical
 * definition and original edge order. Include categories, target/member/implicit
 * facts and source path/offset sites. No allocation or mutation. This proves
 * metadata projection equality, not source-content equality; callers separately
 * admit complete captured bytes. Valid source-backed spans are required under
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
bool source_graph_equal(const DeliverySourceDefinition *, const DeliverySourceDefinition *);
#endif
