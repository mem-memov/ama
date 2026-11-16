/** @file
 * @brief Select a checked declaration identity and prepare its complete workspace edit set.
 * [NAME-06](../../../../spec/language/declarations.md#name-06) prevents spelling-based scope conflation;
 * [ARCH-06](../../../../spec/implementation/compiler.md#arch-06) rejects incomplete domains.
 */
#include <assert.h>
#include "../internal.h"
/** A module-path target is not a variable rename; incomplete/entry-only analysis cannot certify external importers. Component contract: [NAME-06](../../../../spec/language/declarations.md#name-06), [ARCH-06](../../../../spec/implementation/compiler.md#arch-06). */
bool editor_rename_prepare(EditorRename *plan, const EditorAnalysis *workspace, const char *module,
    size_t offset, const unsigned char *name, size_t length) {
    assert(!plan->attempted);
    plan->attempted = true;
    if (workspace == NULL || !workspace->workspace || !workspace->indexed) {
        return editor_rename_fail(plan,DIAGNOSTIC_TYPE_MISMATCH,"Rename requires a complete checked workspace",(SourceSpan){0});
    }
    EditorDefinition target;
    if (!editor_definition(workspace,module,offset,&target) || target.symbol == NULL) {
        return editor_rename_fail(plan,DIAGNOSTIC_TYPE_MISMATCH,"The selected spelling has no variable declaration",(SourceSpan){0});
    }
    plan->workspace = workspace;
    plan->target = target.symbol;
    if (!editor_rename_name(plan,name,length) || !editor_rename_files(plan)) { return false; }
    plan->prepared = true;
    return true;
}
