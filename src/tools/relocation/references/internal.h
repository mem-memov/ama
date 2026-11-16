/** @file
 * @brief Separate captured domain loading, resolved graph mapping and original-span edit selection.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * applies each original binding exactly once rather than repeating textual substitution.
 */
#ifndef AMA_RELOCATION_REFERENCES_INTERNAL_H
#define AMA_RELOCATION_REFERENCES_INTERNAL_H
#include "references.h"
/** Retain the first error without allocating while reporting failure. Component contract: [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02). */
bool relocation_references_fail(RelocationReferences *, RelocationStatus, const char *, SourceSpan);
/** Original configured identities remain authoritative; newly introduced scope uses only its explicit final-root identity. Component contract: [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02). */
const char *relocation_analysis_name(const RelocationModule *);
/** Copy the complete domain and load all active modules through ordinary dependency/semantic rules without an entry. Component contract: [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02). */
bool relocation_references_domain(RelocationReferences *);
/** Match a loaded nominal module to its exact captured physical/name row. Component contract: [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02). */
const RelocationModule *relocation_reference_module(const RelocationReferences *, const ProjectModule *);
/** Enumerate actual dependency bindings and add only explicit changed module-prefix edits. Component contract: [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02). */
bool relocation_references_graph(RelocationReferences *);
/** Deduplicate identical replacements; reject conflicting or overlapping original spans. Component contract: [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02). */
bool relocation_reference_edit(RelocationReferences *, SourceSpan, const RelocationModule *);
#endif
