/** @file
 * @brief Own alias-only transformations of actual copied active module INI companions.
 * [TOOL-02](../../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves keys/comments and opaque draft/native bytes outside declared aliases;
 * [PKG-04](../../../../../../spec/project/packages.md#pkg-04) fixes original scope.
 */
#ifndef AMA_SOURCE_OPERATION_MODULE_WRITE_H
#define AMA_SOURCE_OPERATION_MODULE_WRITE_H
#include "../../../copy/aliases/rebase.h"
#include "../../../copy/closure.h"
#include "../../../mapping/files/files.h"
#include "../../../mapping/map.h"
#include "../../../mapping/unit/member.h"
#include "../../../environment/environment.h"
/** Borrow exact successful copy/map/P/environment and an actual C row. Standard
 * modules need no package environment; ordinary modules require their actual
 * original package scope. All owners outlive this write and its plan consumers. */
typedef struct {
    const SourceCopyClosure *copy;
    const SourceCloneMap *mains;
    const SourceFileMap *files;
    const SourcePackageEnvironment *original;
    const SourceCloneMapping *clone;
} SourceOperationModuleWriteInputs;
/** Keep category/provenance refusal separate from parser, normalization and edit failure. */
typedef enum { SOURCE_OPERATION_MODULE_OK, SOURCE_OPERATION_MODULE_INVALID,
    SOURCE_OPERATION_MODULE_SCOPE, SOURCE_OPERATION_MODULE_DRAFT,
    SOURCE_OPERATION_MODULE_DOCUMENT, SOURCE_OPERATION_MODULE_ALIASES,
    SOURCE_OPERATION_MODULE_TEXT } SourceOperationModuleWriteStatus;
/** Own parser/scope/edit/result scratch and all failed prefixes. Borrow actual
 * original source bytes and C/P/environment owners. Never mutate a live success.
 * valid certifies this module-alias recipe, not final graph/write publication. */
typedef struct {
    SourceArena arena;
    SourceManager texts;
    ConfigurationDocument document;
    SourceTextEdits edits;
    SourceOperationModuleWriteInputs inputs;
    const ConfigurationAliasScope *aliases;
    const ConfigurationAliasScope *package;
    const SourceFileMapping *file;
    const SourceText *result;
    SourceUnitPathStatus path_status;
    ConfigurationStatus document_status;
    ConfigurationScopeStatus scope_status;
    SourceIssue scope_issue;
    SourceCopyAliasesStatus aliases_status;
    SourceCopyAliasesIssue aliases_issue;
    SourceEditsStatus text_status;
    SourceOperationModuleWriteStatus status;
    bool attempted, edits_initialized, valid;
} SourceOperationModuleWrite;
/** Initialize empty result/parser/scratch ownership without discovery or effects. */
void source_operation_module_write_init(SourceOperationModuleWrite *, SourceAllocator);
/** Retire edit/scope borrowers before parser/result ownership after every prefix. */
void source_operation_module_write_destroy(SourceOperationModuleWrite *);
/** Prepare one active copied module's actual present local INI only. Check C
 * row identity before dereference; match complete raw P source/destination bytes
 * against the registered original companion and shared unit-member path mapping.
 * Draft companions stay opaque and cannot be parsed/rewritten through this API.
 * Resolve original package/local scopes and root all local alias targets using
 * the common normalizer/writer. Preserve keys/comments/line structure by edits.
 * No I/O, execution, imports, mutation or foreign alias-table borrowing. Apply
 * bytes only to owned result storage; actual final scope/compiler/site and
 * complete operation authorization remain mandatory. Failed prefixes remain
 * owned/invalid. Work/storage is bounded by the shared parser/normalizer/edit
 * algorithms plus finite clone/definition/P/scope scans; inputs stay borrowed.
 */
SourceOperationModuleWriteStatus source_operation_module_write_prepare(SourceOperationModuleWrite *,
    const SourceOperationModuleWriteInputs *);
#endif
