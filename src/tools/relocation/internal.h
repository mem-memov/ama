/** @file
 * @brief Separate exact path, root, inventory and source preflight operations.
 * [TOOL-02](../../../spec/implementation/tools-and-conformance.md#tool-02)
 * derives identity from projected physical placement, never dotted-prefix guesses.
 */
#ifndef AMA_TOOLS_RELOCATION_INTERNAL_H
#define AMA_TOOLS_RELOCATION_INTERNAL_H
#include "relocation.h"
/** Retain the first failure and borrowed stable diagnostic paths. Component contract: [TOOL-02](../../../spec/implementation/tools-and-conformance.md#tool-02). */
bool relocation_fail(RelocationPreview *, RelocationStatus, const char *, const char *);
/** Resolve an exact existing physical directory or a missing final child under an existing parent.
 * Dot components obey filesystem traversal; ordinary components are verified without following links.
 * Component contract: [TOOL-02](../../../spec/implementation/tools-and-conformance.md#tool-02). */
const char *relocation_path(RelocationPreview *, const char *, bool missing_final);
/** Resolve an existing directory in the projected filesystem without creating its final path. Component contract: [TOOL-02](../../../spec/implementation/tools-and-conformance.md#tool-02). */
const char *relocation_final_path(RelocationPreview *, const char *);
/** Component-aware containment includes equality and never confuses shared textual prefixes. Component contract: [TOOL-02](../../../spec/implementation/tools-and-conformance.md#tool-02). */
bool relocation_contains(const char *parent, const char *path);
/** Apply P(p)=D/relative(S,p) inside S, preserving every outside path. Returned bytes borrow the preview. Component contract: [TOOL-02](../../../spec/implementation/tools-and-conformance.md#tool-02). */
const char *relocation_project(RelocationPreview *, const char *);
/** Apply the same projection into an independent arena; unchanged outside paths borrow input. Component contract: [TOOL-02](../../../spec/implementation/tools-and-conformance.md#tool-02). */
const char *relocation_project_to(SourceArena *, const RelocationPreview *, const char *);
/** Validate the two endpoints and the required parent mutation permissions without modifying either. Component contract: [TOOL-02](../../../spec/implementation/tools-and-conformance.md#tool-02). */
bool relocation_endpoints(RelocationPreview *, const RelocationRequest *);
/** Copy normalized original settings and project carried roots, retaining exclusions and declared kinds. Component contract: [TOOL-02](../../../spec/implementation/tools-and-conformance.md#tool-02). */
bool relocation_roots(RelocationPreview *, const RelocationRequest *);
/** Capture the complete physical selected tree iteratively, retaining opaque accompanying entries. Component contract: [TOOL-02](../../../spec/implementation/tools-and-conformance.md#tool-02). */
bool relocation_inventory(RelocationPreview *);
/** Read sorted physical entry names through the ordinary catalog's retained directory reader. Component contract: [TOOL-02](../../../spec/implementation/tools-and-conformance.md#tool-02). */
bool relocation_entries(RelocationPreview *, const char *, const char ***, size_t *);
/** Translate catalog failures without losing original source/physical diagnostic locations. Component contract: [TOOL-02](../../../spec/implementation/tools-and-conformance.md#tool-02). */
bool relocation_catalog_failure(RelocationPreview *, ModuleStatus);
/** Classify original and projected source candidates and validate exact registry/topology preflight. Component contract: [TOOL-02](../../../spec/implementation/tools-and-conformance.md#tool-02). */
bool relocation_discover(RelocationPreview *);
#endif
