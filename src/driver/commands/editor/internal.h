/** @file
 * @brief Separate source-tool orchestration from structured snapshots, queries and rename replacements.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04)
 * retains exact source-backed observations through response completion.
 */
#ifndef AMA_DRIVER_EDITOR_INTERNAL_H
#define AMA_DRIVER_EDITOR_INTERNAL_H
#include "editor.h"
#include "editor/analysis/analysis.h"
#include "editor/rename/rename.h"
#include "tools/output/source/source.h"
/** A synchronous array callback retains only writer state; source records remain borrowed from the analysis. Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
typedef struct { ToolJsonWriter *writer; bool first; } DriverEditorArray;
/** Prefix one array item without allocating or retaining the supplied observation. Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
bool driver_editor_item(DriverEditorArray *);
/** Emit snapshot diagnostics, highlighting, interfaces and an optional original byte-point query. Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
bool driver_editor_response(const DriverOptions *, const EditorAnalysis *, ToolJsonWriter *, Diagnostic *);
/** Emit exact checked definition/completion/call/binding observations at one explicit byte position. Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
bool driver_editor_point(const DriverOptions *, const EditorAnalysis *, ToolJsonWriter *);
/** Emit one symbol for interface/completion enumeration; callback false preserves writer failure. Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
bool driver_editor_symbol(void *, const SemanticSymbol *);
/** Emit the exact complete original/replacement file versions and source edits after fresh rename validation. Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
bool driver_editor_rename(const DriverOptions *, const EditorRename *, ToolJsonWriter *);
#endif
