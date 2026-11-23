/** @file
 * @brief Own authorized destination configuration recipes before joining the complete operation plan.
 * [TOOL-02](../../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * permits preserved selectors, explicit concrete additions and owned copied alias
 * targets, never arbitrary repository/comment changes or a foreign environment.
 */
#ifndef AMA_SOURCE_OPERATION_CONFIGURATION_WRITE_H
#define AMA_SOURCE_OPERATION_CONFIGURATION_WRITE_H
#include "../../../copy/configuration/prepare.h"
#include "../../../copy/configuration/version.h"
#include "../../../copy/aliases/rebase.h"
#include "../../../mapping/files/files.h"
/** Carried configuration is explicit raw material. Initialization has no original
 * package scope. Existing destinations retain their original alias/repository bytes. */
typedef enum { SOURCE_OPERATION_CONFIGURATION_EXISTING, SOURCE_OPERATION_CONFIGURATION_CARRIED,
    SOURCE_OPERATION_CONFIGURATION_INITIALIZED } SourceOperationConfigurationKind;
/** Borrow genuine original/preview scopes, immutable source maps and initializer.
 * preview is an actual independently constructed candidate environment, not a
 * substitute original. INITIALIZED requires its actual empty scope and matching
 * typed initialization; other recipes require their actual original scope.
 * All owners outlive the write and complete-plan consumers. */
typedef struct {
    SourceOperationConfigurationKind kind;
    const SourceCopyClosure *copy;
    const SourceCloneMap *mains;
    const SourceFileMap *files;
    const SourceObservedImage *observed;
    const SourcePackageEnvironment *original;
    const SourcePackageScope *before;
    const SourcePackageEnvironment *preview;
    const SourcePackageScope *after;
    const SourceSelectionVersionPair *versions;
    size_t version_count;
    const SourceVersionInitialization *initialization;
} SourceOperationConfigurationWriteInputs;
/** Preserve recipe admission, permission, alias and application errors independently. */
typedef enum { SOURCE_OPERATION_CONFIGURATION_OK, SOURCE_OPERATION_CONFIGURATION_INVALID,
    SOURCE_OPERATION_CONFIGURATION_SCOPE, SOURCE_OPERATION_CONFIGURATION_PERMISSION,
    SOURCE_OPERATION_CONFIGURATION_ALIASES, SOURCE_OPERATION_CONFIGURATION_TEXT } SourceOperationConfigurationWriteStatus;
/** Own the expected bytes, exact requested roots and failed prefixes. Inputs
 * remain borrowed; do not copy a live owner or mutate it after success. valid
 * certifies this recipe only, not original cuts/root/client or publication rights. */
typedef struct {
    SourceArena arena;
    SourceManager texts;
    SourceTextEdits edits;
    SourceOperationConfigurationWriteInputs inputs;
    SourceSelectionRoots roots;
    const SourceText *result;
    SourceConfigurationStatus configuration_status;
    SourceConfigurationIssue configuration_issue;
    SourceCopyAliasesStatus aliases_status;
    SourceCopyAliasesIssue aliases_issue;
    SourceEditsStatus text_status;
    SourceOperationConfigurationWriteStatus status;
    bool attempted, edits_initialized, valid;
} SourceOperationConfigurationWrite;
/** Initialize scratch/result ownership without I/O or parsing. */
void source_operation_configuration_write_init(SourceOperationConfigurationWrite *, SourceAllocator);
/** Destroy applied/failed edit scratch before source-text/result ownership. */
void source_operation_configuration_write_destroy(SourceOperationConfigurationWrite *);
/** Prepare once with exact scope and raw/configuration provenance. Reuse the
 * shared permission/selector algorithms. Carried configurations additionally
 * root all owned aliases to their original normalized bindings; actual source-use
 * paths are prepared separately. Preserve all other bytes by construction.
 * Apply to owned text under the actual destination package.ini key. No discovery,
 * compiler/native/CMake/test execution, source mutation or new original scope.
 * Final full-plan byte/root equality and actual compiler proof remain mandatory.
 * Failed prefixes stay owned/invalid; repeat refuses without erasing prior state.
 * Work/storage: the shared recipe and edit algorithms plus finite original/raw/P
 * scope scans. Inputs and all returned source/entry handles retain their owners.
 */
SourceOperationConfigurationWriteStatus source_operation_configuration_write_prepare(SourceOperationConfigurationWrite *,
    const SourceOperationConfigurationWriteInputs *);
#endif
